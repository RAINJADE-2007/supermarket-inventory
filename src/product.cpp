#include "inventory.h"

// 第1人：商品数据与基础管理。
// 只访问顺序表的有效区间 [0, size)，不得读取未使用的数组位置。
// 所有字符串、价格、数量范围均遵守 inventory.h 的统一约定。
// 占位阶段不修改商品表或输出参数；完成后失败时也必须保留原值。

Status findById(const ProductList& list, const std::string& id, int& index) {
    // TODO A1：按唯一编号精确查找。
    // 输入：商品顺序表、待查编号；输出：匹配商品在顺序表中的下标 index。
    // 成功：找到商品时设置 index，返回 Ok。
    // 失败：编号或顺序表结构非法返回 InvalidInput；未找到返回 NotFound。
    // 边界：空表属于未找到；失败时不得把 index 改成假下标。
    (void)list;
    (void)id;
    (void)index;
    return Status::NotImplemented;
}

Status addProduct(ProductList& list, const Product& product) {
    // TODO A2：在顺序表末尾新增一条商品记录。
    // 输入：现有商品表和完整商品信息；编号必须唯一。
    // 成功：新增记录、维护 size，返回 Ok。
    // 失败：字段或顺序表结构非法返回 InvalidInput；编号重复返回 DuplicateId；
    //       容量已满返回 CapacityExceeded。所有失败都不得改变原表。
    // 边界：初始库存允许为 0；价格、库存、预警值不能超出头文件规定范围。
    (void)list;
    (void)product;
    return Status::NotImplemented;
}

Status updateProductInfo(ProductList& list, const std::string& id,
                         const std::string& name, const std::string& category,
                         long long priceFen, int warningStock) {
    // TODO A3：按编号修改名称、类别、价格和预警数量。
    // 输入：待修改编号及新的四项信息；通过 A1 查找目标商品。
    // 成功：四项信息一起更新，返回 Ok；商品编号和现有库存保持不变。
    // 失败：商品不存在返回 NotFound；字段或顺序表结构非法返回 InvalidInput。
    // 边界：修改类别即完成商品重新分类；任何失败都不能留下只修改一半的记录。
    (void)list;
    (void)id;
    (void)name;
    (void)category;
    (void)priceFen;
    (void)warningStock;
    return Status::NotImplemented;
}
