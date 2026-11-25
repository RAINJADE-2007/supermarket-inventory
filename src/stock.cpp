#include "inventory.h"

// 第2人：出入库与库存预警。
// 商品查找统一调用 A1，不另建第二份商品表或另一套编号规则。
// 只访问 [0, size)；失败时原表和输出参数均保持原值。

Status stockIn(ProductList& list, const std::string& id, int quantity) {
    // TODO B1：给指定商品增加库存。
    // 输入：商品表、商品编号、入库数量；quantity 必须在 1..MAX_STOCK 范围。
    // 成功：增加目标商品库存，返回 Ok，其他商品和字段保持不变。
    // 失败：商品不存在返回 NotFound；输入非法返回 InvalidInput；
    //       合法操作会使库存超过 MAX_STOCK 时返回 CapacityExceeded。
    // 边界：增加后的库存仍须合法；检查上限时不能先做可能溢出的整数加法。
    (void)list;
    (void)id;
    (void)quantity;
    return Status::NotImplemented;
}

Status stockOut(ProductList& list, const std::string& id, int quantity) {
    // TODO B2：从指定商品扣减库存。
    // 输入：商品表、商品编号、出库数量；quantity 必须在 1..MAX_STOCK 范围。
    // 成功：扣减目标商品库存，返回 Ok，其他商品和字段保持不变。
    // 失败：商品不存在返回 NotFound；输入非法返回 InvalidInput；
    //       合法的出库数量大于现有库存时返回 InsufficientStock。
    // 边界：允许恰好出完后库存为 0；不得出现负库存，也不能修改失败操作前的库存。
    (void)list;
    (void)id;
    (void)quantity;
    return Status::NotImplemented;
}

Status findLowStock(const ProductList& list, std::vector<Product>& result) {
    // TODO B3：查询达到或低于预警数量的商品。
    // 输入：只读商品表；输出：按原表顺序排列的商品副本。
    // 成功：用所有满足 stock <= warningStock 的商品替换 result，返回 Ok。
    // 失败：顺序表结构非法返回 InvalidInput，result 保持原值。
    // 边界：仅查询有效 size 范围；空表或无匹配项时返回 Ok 和空结果；不修改原表。
    (void)list;
    (void)result;
    return Status::NotImplemented;
}
