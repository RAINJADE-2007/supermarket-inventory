import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { access, mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { createServer } from 'node:http';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { createInterface } from 'node:readline';
import { fileURLToPath } from 'node:url';

const root = fileURLToPath(new URL('../', import.meta.url));
const publicDir = join(root, 'web', 'public');
const sampleFile = join(root, 'data', 'sample_products.txt');
const defaultNative = process.platform === 'win32' ? 'inventory_web.exe' : 'inventory_web';
const binary = resolve(process.env.INVENTORY_WEB_BIN || join(root, 'build', 'bin', defaultNative));
const port = Number(process.env.INVENTORY_PORT || 4174);
const maxBodyBytes = 8 * 1024 * 1024;

const assets = new Map([
  ['/', ['index.html', 'text/html']],
  ['/index.html', ['index.html', 'text/html']],
  ['/styles.css', ['styles.css', 'text/css']],
  ['/api.js', ['api.js', 'text/javascript']],
  ['/app.js', ['app.js', 'text/javascript']],
  ['/sample.js', ['sample.js', 'text/javascript']]
]);

const productFields = ['id', 'name', 'category', 'priceFen', 'warningStock'];
const actionFields = new Map([
  ['snapshot', []], ['add', [...productFields, 'stock']], ['update', productFields],
  ['stockIn', ['id', 'quantity']], ['stockOut', ['id', 'quantity']],
  ['id', ['text']], ['name', ['text']], ['category', ['text']],
  ['sort', ['key', 'order']], ['lowStock', []], ['generate', ['count']],
  ['import', []], ['export', []], ['sample', []]
]);
const numbers = new Set(['priceFen', 'stock', 'warningStock', 'quantity', 'count']);

let worker;
let pending;
let failed = false;
let workingDirectory;
let queue = Promise.resolve();

function enqueue(task) {
  const result = queue.then(task);
  queue = result.catch(() => {});
  return result;
}

async function startWorker() {
  if (worker) return;
  await access(binary);
  workingDirectory ??= await mkdtemp(join(tmpdir(), 'inventory-web-'));
  const child = spawn(binary, [], { cwd: workingDirectory, windowsHide: true });
  const lines = createInterface({ input: child.stdout, crlfDelay: Infinity });
  lines.on('line', line => pending?.resolve(line));
  child.stderr.pipe(process.stderr);
  await once(child, 'spawn');
  worker = child;
  child.on('error', stopWorker);
  child.stdin.on('error', stopWorker);
  child.on('exit', stopWorker);
}

function stopWorker() {
  failed = true;
  pending?.reject(new Error('Inventory backend stopped'));
}

async function runNative(fields) {
  if (failed) return { status: 'BackendStopped' };
  try {
    await startWorker();
  } catch (error) {
    console.error('Inventory backend could not start:', error.message);
    return { status: 'Unavailable' };
  }

  try {
    const line = await new Promise((resolveLine, reject) => {
      pending = { resolve: resolveLine, reject };
      pending.timer = setTimeout(() => {
        worker.kill();
        stopWorker();
      }, 10_000);
      worker.stdin.write(fields.join('\t') + '\n');
    });
    return JSON.parse(line);
  } catch (error) {
    console.error('Inventory backend failed:', error.message);
    if (!failed) {
      failed = true;
      worker.kill();
    }
    return { status: 'BackendStopped' };
  } finally {
    clearTimeout(pending?.timer);
    pending = undefined;
  }
}

function encode(action) {
  const names = actionFields.get(action?.op);
  if (!names) return null;
  const args = names.map(name => action[name]);
  const valid = args.every((value, index) => numbers.has(names[index])
    ? Number.isSafeInteger(value)
    : typeof value === 'string' && !/[\t\r\n]/u.test(value));
  return valid ? [action.op, ...args] : null;
}

async function run(action) {
  const fields = encode(action);
  if (!fields || (action.op === 'import' && typeof action.text !== 'string')) {
    return { status: 'InvalidInput' };
  }
  if (failed) return { status: 'BackendStopped' };
  if (action.op === 'import' || action.op === 'sample') {
    // The child reads an ASCII relative filename inside a server-owned directory.
    try {
      await startWorker();
    } catch {
      return { status: 'Unavailable' };
    }
    try {
      const content = action.op === 'sample' ? await readFile(sampleFile) : action.text;
      await writeFile(join(workingDirectory, 'import.tsv'), content);
      return await runNative(['import']);
    } finally {
      await rm(join(workingDirectory, 'import.tsv'), { force: true });
    }
  }
  if (action.op === 'export') {
    const result = await runNative(fields);
    if (result.status !== 'Ok') return result;
    const data = await readFile(join(workingDirectory, 'export.tsv'), 'utf8');
    await rm(join(workingDirectory, 'export.tsv'));
    return { status: 'Ok', data };
  }
  return runNative(fields);
}

function reply(response, code, contentType, body) {
  response.writeHead(code, {
    'Content-Type': contentType,
    'Cache-Control': 'no-store',
    'X-Content-Type-Options': 'nosniff'
  });
  response.end(body);
}

function sendJSON(response, code, value) {
  reply(response, code, 'application/json; charset=utf-8', JSON.stringify(value));
}

async function receiveAction(request, response) {
  const origin = request.headers.origin;
  const allowedOrigin = origin === 'http://127.0.0.1:' + port || origin === 'http://localhost:' + port;
  if ((origin && !allowedOrigin) || !/^application\/json(?:\s*;|$)/iu.test(request.headers['content-type'] || '')) {
    return sendJSON(response, 403, { status: 'InvalidInput' });
  }

  const chunks = [];
  let size = 0;
  for await (const chunk of request) {
    size += chunk.length;
    if (size > maxBodyBytes) {
      sendJSON(response, 413, { status: 'UploadTooLarge' });
      return;
    }
    chunks.push(chunk);
  }

  let action;
  try {
    action = JSON.parse(Buffer.concat(chunks).toString('utf8'));
  } catch {
    return sendJSON(response, 400, { status: 'InvalidInput' });
  }
  sendJSON(response, 200, await enqueue(() => run(action)));
}

const server = createServer(async (request, response) => {
  try {
    if (request.headers.host !== '127.0.0.1:' + port && request.headers.host !== 'localhost:' + port) {
      return sendJSON(response, 403, { status: 'InvalidInput' });
    }
    const path = request.url.split('?', 1)[0];
    if (request.method === 'POST' && path === '/api/action') {
      await receiveAction(request, response);
      return;
    }
    if (request.method === 'GET' && path === '/runtime.js') {
      reply(response, 200, 'text/javascript; charset=utf-8', 'window.inventoryLocalServer = true;');
      return;
    }
    const asset = request.method === 'GET' && assets.get(path);
    if (asset) {
      const [name, type] = asset;
      reply(response, 200, type + '; charset=utf-8', await readFile(join(publicDir, name)));
      return;
    }
    reply(response, 404, 'text/plain; charset=utf-8', '未找到页面');
  } catch (error) {
    console.error('Inventory request failed:', error.message);
    if (!response.headersSent) sendJSON(response, 500, { status: 'FileError' });
    else response.end();
  }
});

let shuttingDown = false;
async function shutdown() {
  if (shuttingDown) return;
  shuttingDown = true;
  server.close();
  if (worker && worker.exitCode === null && worker.signalCode === null) {
    const exited = once(worker, 'exit');
    worker.kill();
    await exited;
  }
  if (workingDirectory) await rm(workingDirectory, { recursive: true, force: true });
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
server.listen(port, '127.0.0.1', () => {
  console.log('超市库存工作台 http://127.0.0.1:' + port);
});
