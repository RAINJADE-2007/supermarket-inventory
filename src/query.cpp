#include "inventory.h"

// 第2人：查询、排序与统计。
// 只查询 [0, size) 内的有效商品，所有操作都不能修改原商品表。
// 返回 Ok 时输出本次完整结果；失败或尚未实现时，输出参数保持原值。

Status queryByName(const ProductList& list, const std::string& keyword,
                   std::vector<Product>& result) {
    // TODO C1：按商品名称中的关键词进行部分匹配。
    // 输入：只读商品表和非空关键词；输出：按原表顺序排列的匹配商品副本。
    // 成功：用本次结果替换 result，返回 Ok；不能把结果追加到上一次查询之后。
    // 失败：关键词或顺序表结构非法返回 InvalidInput。
    // 边界：空表或无匹配项时返回 Ok 和空结果；匹配商品名称，不匹配编号或类别。
    (void)list;
    (void)keyword;
    (void)result;
    return Status::NotImplemented;
}

Status queryByCategory(const ProductList& list, const std::string& category,
                       std::vector<Product>& result) {
    // TODO C2：按商品类别精确匹配。
    // 输入：只读商品表和完整类别名称；输出：按原表顺序排列的匹配商品副本。
    // 成功：用本次结果替换 result，返回 Ok。
    // 失败：类别或顺序表结构非法返回 InvalidInput。
    // 边界：空表或无匹配项时返回 Ok 和空结果；“饮料”不匹配“饮料食品”。
    (void)list;
    (void)category;
    (void)result;
    return Status::NotImplemented;
}

Status sortProducts(const ProductList& list, SortKey key, bool ascending,
                    std::vector<Product>& result) {
    // TODO C3：按价格或库存数量排列商品副本，用于排序展示。
    // 输入：只读商品表、排序字段、是否升序；输出：包含全部有效商品的排序结果。
    // 成功：替换 result，返回 Ok；相同价格或库存值时，一律按 id 字符串升序排列。
    // 失败：顺序表结构或排序字段非法返回 InvalidInput。
    // 边界：升序、降序均遵守相同的 id 次序规则；空表返回 Ok 和空结果；原表顺序不变。
    (void)list;
    (void)key;
    (void)ascending;
    (void)result;
    return Status::NotImplemented;
}

Status calculateStatistics(const ProductList& list, Statistics& result) {
    // TODO C4：统计全部商品。
    // 输入：只读商品表；输出：商品种类数、库存总数量、库存总金额（分）。
    // 成功：用本次统计值替换 result，返回 Ok；金额为各商品 priceFen * stock 的总和。
    // 失败：顺序表结构非法返回 InvalidInput。
    // 边界：空表返回 Ok，三项统计均为 0；乘法与累计使用足够宽的整数类型，避免溢出。
    (void)list;
    (void)result;
    return Status::NotImplemented;
}

Status calculateCategoryStatistics(const ProductList& list,
                                  std::vector<CategoryStatistics>& result) {
    // TODO C5：按类别分别统计商品种类数、库存总数量和库存总金额（分）。
    // 输入：只读商品表；输出：每个已出现类别对应一项 CategoryStatistics。
    // 成功：用本次统计结果替换 result，返回 Ok；各项按类别名称升序排列。
    // 失败：顺序表结构非法返回 InvalidInput。
    // 边界：同一类别只出现一项；空表返回 Ok 和空结果；各类别汇总应与 C4 一致。
    (void)list;
    (void)result;
    return Status::NotImplemented;
}
