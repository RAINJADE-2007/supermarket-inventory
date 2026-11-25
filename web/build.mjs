import { spawnSync } from 'node:child_process';
import { mkdirSync } from 'node:fs';
import { basename, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = fileURLToPath(new URL('../', import.meta.url));
const outputDir = join('build', 'bin');
const objectDir = join('build', 'obj');
const suffix = process.platform === 'win32' ? '.exe' : '';
const compiler = process.env.CXX || 'g++';
const modules = ['product', 'stock', 'query', 'data_io'].map(name => 'src/' + name + '.cpp');
const env = { ...process.env };

if (env.INVENTORY_BUILD_TEMP) {
  const workingTemp = resolve(env.INVENTORY_BUILD_TEMP);
  mkdirSync(workingTemp, { recursive: true });
  env.TMP = workingTemp;
  env.TEMP = workingTemp;
  env.TMPDIR = workingTemp;
}

function runCompiler(args) {
  const result = spawnSync(compiler, args, { cwd: root, env, stdio: 'inherit', windowsHide: true });
  if (result.error) {
    console.error('无法启动 C++ 编译器 ' + compiler + '。请将 g++ 加入 PATH，或设置 CXX 指向兼容的编译器。');
    process.exit(1);
  }
  if (result.status !== 0) process.exit(result.status ?? 1);
}

function compile(source) {
  const object = join(objectDir, basename(source, '.cpp') + '.o');
  runCompiler(['-std=c++11', '-Wall', '-Wextra', '-Wpedantic', '-pipe', '-Iinclude', '-c', source, '-o', object]);
  return object;
}

mkdirSync(join(root, outputDir), { recursive: true });
mkdirSync(join(root, objectDir), { recursive: true });
const commonObjects = modules.map(compile);
for (const [entry, name] of [['src/main.cpp', 'supermarket'], ['web/native/inventory_web.cpp', 'inventory_web']]) {
  const executable = join(outputDir, name + suffix);
  runCompiler([compile(entry), ...commonObjects, '-o', executable]);
  console.log('已生成 ' + executable);
}
