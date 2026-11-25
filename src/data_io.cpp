#include "inventory.h"

// 第2人：文件输入输出和随机数据生成。
// 文件编码：UTF-8；每行恰好 6 个字段，字段之间使用真实制表符（Tab）。
// 首行表头：id\tname\tcategory\tprice_fen\tstock\twarning_stock
// 表头之后依次是商品编号、名称、类别、单价（分）、库存数量、预警数量。
// 字段合法性与商品新增接口相同；失败不得修改原商品表、原文件或输出参数。

Status loadFromFile(ProductList& list, const std::string& filename) {
    // TODO D1：整批导入商品，并替换当前商品表。
    // 输入：目标商品表和文件名；必须完整读取、校验所有记录后才能替换原表。
    // 成功：导入整批商品，返回 Ok；文件仅有合法表头时可导入为空表。
    // 失败：打开或读取失败返回 FileError；表头、字段、数值或文件名非法返回 InvalidInput；
    //       文件内部编号重复返回 DuplicateId；记录超过 MAX_PRODUCTS 返回 CapacityExceeded。
    // 边界：不得留下部分导入结果；不能悄悄跳过坏行或忽略多余字段；数字字段需完整解析。
    (void)list;
    (void)filename;
    return Status::NotImplemented;
}

Status saveToFile(const ProductList& list, const std::string& filename) {
    // TODO D2：把有效商品记录保存为上述 UTF-8、Tab 分隔格式。
    // 输入：只读商品表和目标文件名；需保留表头、整数金额及所有有效记录。
    // 成功：数据完整写入且文件完成替换后返回 Ok，原商品表保持不变。
    // 失败：输入或顺序表结构非法返回 InvalidInput；打开、写入、关闭或替换失败返回 FileError。
    // 边界：空表仍须保存表头；先写同目录临时文件，验证成功后再替换目标，避免损坏旧文件。
    //       失败时保留原文件，并妥善清理本次临时文件，不能提前声称保存成功。
    (void)list;
    (void)filename;
    return Status::NotImplemented;
}

Status generateRandomProducts(ProductList& list, int count) {
    // TODO D3：向现有商品表追加 count 条随机测试商品。
    // 输入：现有商品表和生成条数；count 必须在 1..MAX_PRODUCTS 范围。
    // 成功：整批追加、维护 size，返回 Ok；新编号既不能互相重复，也不能与原表重复。
    // 失败：条数或顺序表结构非法返回 InvalidInput；剩余容量不足返回 CapacityExceeded。
    // 边界：所有生成字段必须符合头文件约定；原有商品不变，失败时不得留下部分新增记录。
    (void)list;
    (void)count;
    return Status::NotImplemented;
}
