(() => {
  const { request, preview, previewFilter } = window.inventoryApi;
  const $ = id => document.getElementById(id);
  const state = { mode: 'preview', page: 'inventory', live: null, filtered: null, revision: 0 };
  const demo = preview();
  const statusText = {
    Ok: '操作完成。',
    NotImplemented: '对应的 C++ 业务函数待小组填写。',
    NotFound: '没有找到该编号的商品。',
    DuplicateId: '商品编号已存在。',
    InvalidInput: '输入不合法，请检查商品字段、整数范围或文件格式。',
    CapacityExceeded: '商品数量或库存超过上限。',
    InsufficientStock: '当前库存不足，出库未执行。',
    FileError: '文件打开、读取、写入或替换失败。',
    UploadTooLarge: '导入请求超过 8 MB。',
    StaticPreview: '请通过 npm start 打开本地工作台以连接 C++。',
    Unavailable: '未连接 C++。请先运行 npm run build，再重试。',
    BackendStopped: 'C++ 进程已停止，请重启工作台。'
  };
  const codeLabels = {
    id: 'A1', add: 'A2', update: 'A3', stockIn: 'B1', stockOut: 'B2',
    lowStock: 'B3', name: 'C1', category: 'C2', sort: 'C3',
    import: 'D1', sample: 'D1', export: 'D2', generate: 'D3'
  };
  const pageInfo = {
    inventory: ['库存总览', '查看商品库存、预警和库存总值。'],
    alerts: ['低库存预警', '显示库存小于或等于预警值的商品。'],
    categories: ['分类统计', '按类别查看商品种类、库存与库存总值。']
  };

  function el(tag, text, className) {
    const item = document.createElement(tag);
    if (text !== undefined) item.textContent = text;
    if (className) item.className = className;
    return item;
  }

  function number(value) {
    return value === undefined ? '—' : Number(value).toLocaleString('zh-CN');
  }

  function money(value) {
    if (value === undefined) return '—';
    const fen = BigInt(value);
    return '¥' + (fen / 100n).toLocaleString('zh-CN') + '.' + String(fen % 100n).padStart(2, '0');
  }

  function tell(id, result, operation) {
    const item = $(id);
    item.hidden = !result;
    if (!result) return;
    item.dataset.error = result.status !== 'Ok';
    const code = result.status === 'NotImplemented' && codeLabels[operation];
    item.textContent = (code ? code + ' · ' : '') + (statusText[result.status] || '操作失败。');
  }

  function renderSource() {
    document.querySelectorAll('[data-mode]').forEach(button => {
      button.setAttribute('aria-pressed', String(button.dataset.mode === state.mode));
    });
    const [title, text] = state.mode === 'preview'
      ? ['示例数据', '以下为随课题提供的示例，可筛选和排序。切换到 C++ 工作区可验证业务接口。']
      : ['C++ 数据', window.inventoryLocalServer
        ? '当前数据保存在本地 C++ 进程。出入库成功后会刷新，退出前可通过数据文件导出。'
        : '静态预览无法连接进程。运行项目后再打开本地工作台。'];
    $('mode-note').replaceChildren(el('b', title), el('span', text));
  }

  function renderPage() {
    const inventory = state.page === 'inventory';
    const alerts = state.page === 'alerts';
    const [title, subtitle] = pageInfo[state.page];
    $('main-title').textContent = title;
    $('subtitle').textContent = subtitle;
    $('catalog').hidden = !inventory && !alerts;
    $('categories').hidden = alerts;
    $('metrics').hidden = !inventory;
    $('query-form').hidden = !inventory;
    $('chart-footer').hidden = !inventory;
    $('workspace-grid').dataset.page = state.page;
    $('catalog-title').textContent = alerts ? '待补货商品' : '商品清单';
    $('table-description').textContent = alerts ? '按预警规则识别需补货的商品。' : '按商品编号、名称或类别查询。';
    $('table-caption').textContent = alerts ? '待补货商品明细' : '库存商品明细';
    document.querySelectorAll('nav [data-page]').forEach(button => {
      if (button.dataset.page === state.page) button.setAttribute('aria-current', 'page');
      else button.removeAttribute('aria-current');
    });
  }

  function renderMetrics(data) {
    const totals = data?.totals?.data;
    const alerts = data?.lowStock?.data?.length;
    $('metric-kinds').textContent = number(totals?.productKinds);
    $('metric-stock').textContent = number(totals?.totalStock);
    $('metric-value').textContent = money(totals?.totalValueFen);
    $('metric-alerts').textContent = number(alerts);
    $('nav-alerts').hidden = !alerts;
    $('nav-alerts').textContent = number(alerts);
  }

  function productRow(product, index, lowIds) {
    const row = el('tr');
    const icon = el('span', Array.from(product.name)[0], 'product-icon');
    icon.dataset.style = index % 3;
    icon.setAttribute('aria-hidden', 'true');
    const label = el('div');
    label.append(el('strong', product.name), el('small', '#' + product.id));
    const productCell = el('td');
    const productBox = el('div', undefined, 'product');
    productBox.append(icon, label);
    productCell.append(productBox);
    const stock = el('td', number(product.stock));
    stock.append(el('small', '预警值 ' + number(product.warningStock), 'stock-threshold'));

    const low = lowIds?.has(product.id);
    const indicator = el('td');
    const indicatorText = el('span', lowIds ? (low ? '库存偏低' : '充足') : '待评估', 'chip');
    indicatorText.dataset.low = Boolean(low);
    indicator.append(indicatorText);

    const actions = el('td');
    const editable = state.mode === 'live';
    const labelAction = editable ? '编辑' : '查看';
    const edit = el('button', labelAction, 'text-button');
    edit.type = 'button';
    edit.dataset.open = editable ? 'update' : 'inspect';
    edit.dataset.id = product.id;
    edit.setAttribute('aria-label', labelAction + ' ' + product.name);
    actions.append(edit);
    row.append(productCell, el('td', product.category), el('td', money(product.priceFen)),
      stock, indicator, actions);
    return row;
  }

  function renderTable(data) {
    const alerts = state.page === 'alerts';
    const lowStock = data?.lowStock?.data;
    const lowIds = lowStock ? new Set(lowStock.map(product => product.id)) : undefined;
    const products = alerts ? lowStock : (state.filtered ?? data?.products);
    const rows = products ?? [];
    $('product-rows').replaceChildren(...rows.map((product, index) => productRow(product, index, lowIds)));
    $('table-wrap').hidden = rows.length === 0;
    $('empty-table').hidden = rows.length > 0;
    let emptyMessage = 'C++ 工作区暂无数据。';
    if (alerts && data?.lowStock?.status === 'NotImplemented') {
      emptyMessage = 'B3 待实现，暂无法查看预警。';
    } else if (alerts && lowStock) {
      emptyMessage = '当前没有低库存商品。';
    } else if (state.filtered) {
      emptyMessage = '没有符合条件的结果。';
    } else if (data && !alerts) {
      emptyMessage = '工作区没有商品，可新增商品或导入文件。';
    }
    $('empty-table').textContent = emptyMessage;
    $('row-count').textContent = products ? number(rows.length) + ' 条' : '—';
    $('table-footer').textContent = products ? '显示 ' + number(rows.length) + ' 条记录' : '等待 C++ 数据';
  }

  function renderChart(data) {
    const rows = data?.categories?.data ?? [];
    const details = state.page === 'categories';
    const totalStock = rows.reduce((sum, row) => sum + row.totals.totalStock, 0);
    $('category-bars').hidden = rows.length === 0;
    $('empty-chart').hidden = rows.length > 0;
    $('empty-chart').textContent = data?.categories?.status === 'NotImplemented'
      ? 'C5 待实现，暂无法生成分类统计。' : '暂无分类统计。';

    $('category-bars').replaceChildren(...rows.map(({ category, totals }, index) => {
      const row = el('div', undefined, 'chart-row');
      row.dataset.style = index % 3;
      const label = el('div', undefined, 'chart-label');
      label.append(el('span', category), el('b', number(totals.totalStock) + ' 件'));
      const bar = el('progress');
      bar.max = totalStock || 1;
      bar.value = totals.totalStock;
      bar.setAttribute('aria-label', category + '库存占比');
      row.append(label, bar);
      return row;
    }));

    $('category-detail').hidden = !details || rows.length === 0;
    $('category-rows').replaceChildren(...rows.map(({ category, totals }) => {
      const row = el('tr');
      row.append(el('td', category), el('td', number(totals.productKinds)),
        el('td', number(totals.totalStock)), el('td', money(totals.totalValueFen)));
      return row;
    }));
  }

  function render() {
    const data = state.mode === 'preview' ? demo : state.live;
    renderSource();
    renderPage();
    renderMetrics(data);
    renderTable(data);
    renderChart(data);
  }

  function resetQuery() {
    state.revision += 1;
    state.filtered = null;
    $('query-form').reset();
    tell('status');
  }

  async function loadWorkplace() {
    const revision = state.revision;
    const result = await request('snapshot');
    if (state.mode !== 'live') return;
    state.live = result.data ?? null;
    $('server-label').textContent = result.status === 'Ok' ? 'C++ 已连接'
      : window.inventoryLocalServer ? 'C++ 待连接' : '静态演示';
    if (revision === state.revision && result.status !== 'Ok') tell('status', result, 'snapshot');
    render();
  }

  function switchMode(mode) {
    resetQuery();
    state.mode = mode;
    render();
    if (mode === 'live') return loadWorkplace();
  }

  async function tableAction(op, fields) {
    const revision = ++state.revision;
    if (state.mode === 'preview') {
      state.filtered = previewFilter(op, fields);
    } else {
      const result = await request(op, fields);
      if (revision !== state.revision) return;
      state.filtered = result.status === 'Ok' ? result.data : null;
      tell('status', result.status === 'Ok' ? undefined : result, op);
    }
    renderTable(state.mode === 'preview' ? demo : state.live);
  }

  const dialog = $('operation-dialog');
  const modalSettings = {
    inspect: ['示例商品', '查看随课题提供的示例商品。', ''],
    add: ['新增商品', '录入商品主档及初始库存。', '新增商品'],
    update: ['编辑商品', '修改商品信息时保留原有编号与库存。', '保存信息'],
    stockIn: ['出入库操作', '按商品编号更新 C++ 工作区中的库存。', '更新库存'],
    generate: ['追加随机商品', '生成新的商品记录并追加至当前商品表。', '生成商品'],
    import: ['导入商品文件', '需要 UTF-8、Tab 分隔及固定表头。导入成功后替换当前商品表。', '导入文件'],
    sample: ['导入示例', '把随课题提供的三条示例记录导入 C++ 工作区，成功后替换当前商品表。', '导入示例'],
    export: ['导出商品文件', '将当前 C++ 工作区中的商品表保存为制表符分隔文本。', '导出文件'],
    data: ['数据文件', '为 C++ 工作区导入、导出或生成商品记录。', '']
  };

  function inputField(fields, name, label, options = {}) {
    const wrapper = el('label', label);
    if (options.full) wrapper.className = 'full-width';
    const input = el('input');
    input.name = name;
    input.type = options.type ?? 'text';
    input.required = true;
    if (options.value !== undefined) input.value = options.value;
    if (options.min !== undefined) input.min = options.min;
    if (options.max !== undefined) input.max = options.max;
    if (options.accept) input.accept = options.accept;
    if (options.readOnly) input.readOnly = true;
    wrapper.append(input);
    fields.append(wrapper);
  }

  function productFields(fields, op, product) {
    inputField(fields, 'id', '商品编号', { value: product?.id, readOnly: op === 'update' });
    inputField(fields, 'name', '商品名称', { value: product?.name });
    inputField(fields, 'category', '商品类别', { value: product?.category });
    inputField(fields, 'priceFen', '单价（分）', { type: 'number', value: product?.priceFen, min: 0, max: 100000000 });
    inputField(fields, 'warningStock', '预警值', { type: 'number', value: product?.warningStock, min: 0, max: 1000000 });
    if (op === 'add') inputField(fields, 'stock', '初始库存', { type: 'number', min: 0, max: 1000000 });
  }

  function openModal(op, product) {
    const [title, description, submit] = modalSettings[op];
    dialog.dataset.op = op;
    $('dialog-title').textContent = title;
    $('dialog-description').textContent = description;
    $('submit-action').textContent = submit;
    $('submit-action').hidden = !submit;
    tell('dialog-status');
    const fields = $('dialog-fields');
    fields.replaceChildren();

    if (op === 'inspect' && product) {
      const details = el('dl', undefined, 'product-detail full-width');
      for (const [label, value] of [
        ['商品编号', product.id], ['商品名称', product.name],
        ['商品类别', product.category], ['单价', money(product.priceFen)],
        ['库存', number(product.stock)], ['预警值', number(product.warningStock)]
      ]) {
        details.append(el('dt', label), el('dd', value));
      }
      fields.append(details);
    }
    if (op === 'add' || op === 'update') productFields(fields, op, product);
    if (op === 'stockIn') {
      const stockMethod = el('label', '操作类型', 'full-width');
      const select = el('select');
      select.name = 'method';
      for (const [value, label] of [['stockIn', '入库'], ['stockOut', '出库']]) {
        const option = el('option', label);
        option.value = value;
        select.append(option);
      }
      stockMethod.append(select);
      fields.append(stockMethod);
      inputField(fields, 'id', '商品编号');
      inputField(fields, 'quantity', '数量', { type: 'number', min: 1, max: 1000000 });
    }
    if (op === 'generate') inputField(fields, 'count', '追加数量', { type: 'number', min: 1, max: 1000, full: true });
    if (op === 'import') inputField(fields, 'file', '选择 UTF-8 文件', { type: 'file', accept: '.txt,.tsv,text/plain,text/tab-separated-values', full: true });
    if (op === 'data') {
      for (const [action, details] of [
        ['import', '从 .txt 或 .tsv 文件替换商品表'],
        ['sample', '载入可乐、面包与洗衣液示例'],
        ['export', '下载当前商品表并留存'],
        ['generate', '批量创建记录，验证表容量']
      ]) {
        const row = el('div', undefined, 'data-action full-width');
        row.append(el('p', details));
        const button = el('button', modalSettings[action][0], 'button quiet');
        button.type = 'button';
        button.dataset.open = action;
        row.append(button);
        fields.append(row);
      }
    }
    if (!dialog.open) dialog.showModal();
    else dialog.querySelector('input, select, #submit-action:not([hidden])')?.focus();
  }

  function download(text) {
    const url = URL.createObjectURL(new Blob([text], { type: 'text/tab-separated-values;charset=utf-8' }));
    const link = el('a');
    link.href = url;
    link.download = 'inventory.tsv';
    link.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }

  document.addEventListener('click', event => {
    const button = event.target.closest('button');
    if (event.target.closest('.brand')) {
      event.preventDefault();
      state.page = 'inventory';
      resetQuery();
      render();
      return;
    }
    if (!button) return;
    if (button.dataset.page) {
      state.page = button.dataset.page;
      resetQuery();
      render();
    }
    if (button.dataset.mode) switchMode(button.dataset.mode);
    if (button.dataset.open) {
      const data = state.mode === 'preview' ? demo : state.live;
      const product = data?.products.find(item => item.id === button.dataset.id);
      openModal(button.dataset.open, product);
    }
  });

  for (const id of ['close-dialog', 'cancel-dialog']) {
    $(id).addEventListener('click', () => {
      if (!$('submit-action').disabled) dialog.close();
    });
  }
  dialog.addEventListener('cancel', event => {
    if ($('submit-action').disabled) event.preventDefault();
  });

  $('query-form').addEventListener('submit', event => {
    event.preventDefault();
    $('sort').value = '';
    const text = $('query-text').value;
    if (!text.trim()) {
      resetQuery();
      render();
      return;
    }
    tableAction($('query-kind').value, { text });
  });

  $('query-text').addEventListener('search', () => {
    if (!$('query-text').value) {
      resetQuery();
      render();
    }
  });

  $('sort').addEventListener('change', event => {
    if (!event.target.value) {
      resetQuery();
      render();
      return;
    }
    $('query-text').value = '';
    const [key, order] = event.target.value.split(':');
    tableAction('sort', { key, order });
  });

  $('operation-form').addEventListener('submit', async event => {
    event.preventDefault();
    const op = dialog.dataset.op;
    const submit = $('submit-action');
    if (submit.disabled || submit.hidden) return;
    submit.disabled = true;
    const form = new FormData(event.target);
    const action = op === 'stockIn' ? form.get('method') : op;
    try {
      const fields = {};
      for (const [name, value] of form.entries()) {
        if (name === 'method') continue;
        if (name === 'file') {
          if (value.size > 8 * 1024 * 1024) {
            tell('dialog-status', { status: 'UploadTooLarge' }, action);
            return;
          }
          try {
            fields.text = new TextDecoder('utf-8', { fatal: true, ignoreBOM: true })
              .decode(await value.arrayBuffer());
          } catch {
            tell('dialog-status', { status: 'InvalidInput' }, action);
            return;
          }
        } else {
          const input = event.target.elements.namedItem(name);
          fields[name] = input.type === 'number' ? Number(value) : value;
        }
      }
      const result = await request(action, fields);
      if (result.status !== 'Ok') {
        tell('dialog-status', result, action);
        return;
      }
      if (action === 'export') download(result.data);
      dialog.close();
      await switchMode('live');
      if ($('status').hidden) tell('status', result, action);
    } catch {
      tell('dialog-status', { status: 'FileError' }, action);
    } finally {
      submit.disabled = false;
    }
  });

  if (window.inventoryLocalServer) $('server-label').textContent = '本地工作台';
  render();
})();
