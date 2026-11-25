#ifndef SUPERMARKET_INVENTORY_H
#define SUPERMARKET_INVENTORY_H

#include <string>
#include <vector>

// 四人共用的接口。先约定接口，再分别填写各自的 .cpp 文件。
// 商品必须保存在下面的顺序表中；vector 只用于返回查询结果。
constexpr int MAX_PRODUCTS = 1000;
constexpr int MAX_STOCK = 1000000;
constexpr long long MAX_PRICE_FEN = 100000000;

struct Product {
    std::string id;                  // 唯一编号，例如 "1001"，不允许修改编号
    std::string name;
    std::string category;
    long long priceFen = 0;          // 金额统一按“分”保存，300 表示 3.00 元
    int stock = 0;                  // 0 <= stock <= MAX_STOCK
    int warningStock = 0;           // 0 <= warningStock <= MAX_STOCK
};

struct ProductList {
    Product data[MAX_PRODUCTS];
    int size = 0;                   // 只有 data[0] 到 data[size - 1] 有效
};

enum class Status {
    Ok,
    NotImplemented,                 // TODO 占位专用，不代表业务操作成功
    NotFound,
    DuplicateId,
    InvalidInput,
    CapacityExceeded,
    InsufficientStock,
    FileError
};

enum class SortKey { Price, Stock };

struct Statistics {
    int productKinds = 0;           // 商品记录数，不是所有商品库存数量之和
    long long totalStock = 0;
    long long totalValueFen = 0;
};

struct CategoryStatistics {
    std::string category;
    Statistics totals;
};

// 全部接口统一约定：
// 1. 成功返回 Ok；失败返回具体原因，不能只打印提示后返回 Ok。
// 2. index/result 等输出参数，仅在返回 Ok 时有效。
// 3. 输入不合法或失败时，保留原商品表；查询/统计/排序不修改原表。
// 4. 查询成功但没有匹配项时，返回 Ok，并给出空结果。
// 5. 字符串不允许为空或仅空格，不能包含制表符、回车或换行。
// 6. 价格范围为 0..MAX_PRICE_FEN；库存及预警值范围为 0..MAX_STOCK。

// ========== 第1人：src/product.cpp，A1—A3 ==========
Status findById(const ProductList& list, const std::string& id, int& index);
Status addProduct(ProductList& list, const Product& product);
Status updateProductInfo(ProductList& list, const std::string& id,
                         const std::string& name, const std::string& category,
                         long long priceFen, int warningStock);

// ========== 第2人：src/stock.cpp，B1—B3 ==========
Status stockIn(ProductList& list, const std::string& id, int quantity);
Status stockOut(ProductList& list, const std::string& id, int quantity);
Status findLowStock(const ProductList& list, std::vector<Product>& result);

// ========== 第3人：src/query.cpp，C1—C5 ==========
Status queryByName(const ProductList& list, const std::string& keyword,
                   std::vector<Product>& result);
Status queryByCategory(const ProductList& list, const std::string& category,
                       std::vector<Product>& result);
Status sortProducts(const ProductList& list, SortKey key, bool ascending,
                    std::vector<Product>& result);
Status calculateStatistics(const ProductList& list, Statistics& result);
Status calculateCategoryStatistics(const ProductList& list,
                                  std::vector<CategoryStatistics>& result);

// ========== 第4人：src/data_io.cpp，D1—D3 ==========
// 文件导入：全部读取和校验成功后，替换当前商品表；失败时保留原表。
Status loadFromFile(ProductList& list, const std::string& filename);
Status saveToFile(const ProductList& list, const std::string& filename);
// 随机生成：向原表追加 count 条商品；容量不足或输入无效时不追加。
Status generateRandomProducts(ProductList& list, int count);

#endif
