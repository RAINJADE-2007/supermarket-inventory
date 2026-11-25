(() => {
  const sample = window.inventorySample;

  async function request(op, fields = {}) {
    if (!window.inventoryLocalServer) return { status: 'StaticPreview' };
    try {
      const response = await fetch('/api/action', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ op, ...fields })
      });
      return await response.json();
    } catch {
      return { status: 'Unavailable' };
    }
  }

  function summarize(products) {
    let totalStock = 0;
    let totalValueFen = 0n;
    for (const product of products) {
      totalStock += product.stock;
      totalValueFen += BigInt(product.priceFen) * BigInt(product.stock);
    }
    return { productKinds: products.length, totalStock, totalValueFen: String(totalValueFen) };
  }

  function preview() {
    const categories = Array.from(new Set(sample.map(product => product.category)));
    return {
      products: sample,
      totals: { status: 'Ok', data: summarize(sample) },
      lowStock: { status: 'Ok', data: sample.filter(product => product.stock <= product.warningStock) },
      categories: {
        status: 'Ok',
        data: categories.map(category => ({
          category, totals: summarize(sample.filter(product => product.category === category))
        }))
      }
    };
  }

  function previewFilter(op, fields) {
    switch (op) {
      case 'id': return sample.filter(product => product.id === fields.text);
      case 'name': return sample.filter(product => product.name.includes(fields.text));
      case 'category': return sample.filter(product => product.category === fields.text);
      case 'sort': {
        const key = fields.key === 'price' ? 'priceFen' : 'stock';
        const direction = fields.order === 'asc' ? 1 : -1;
        return sample.slice().sort((left, right) =>
          direction * (left[key] - right[key]) || left.id.localeCompare(right.id));
      }
      default: return sample;
    }
  }

  window.inventoryApi = { request, preview, previewFilter };
})();
