#include "inventory.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// 主菜单、基础输入和结果显示已接好。
// 你们主要填写四个业务 .cpp 内的 TODO，函数签名保持与头文件一致。
namespace {

bool readText(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    if (std::getline(std::cin, value)) {
        return true;
    }
    std::cout << "\n输入结束，程序退出。\n";
    return false;
}

template <typename T>
bool readNumber(const std::string& prompt, T& value) {
    std::string line;
    while (readText(prompt, line)) {
        std::istringstream input(line);
        T candidate;
        if (input >> candidate) {
            input >> std::ws;
            if (input.eof()) {
                value = candidate;
                return true;
            }
        }
        std::cout << "请输入范围内的完整整数。\n";
    }
    return false;
}

void printMoney(long long fen) {
    std::cout << fen / 100 << '.' << std::setw(2) << std::setfill('0')
              << fen % 100 << std::setfill(' ');
}

void printProduct(const Product& p) {
    std::cout << p.id << " | " << p.name << " | " << p.category << " | 单价 ";
    printMoney(p.priceFen);
    std::cout << " 元 | 库存 " << p.stock << " | 预警值 " << p.warningStock << '\n';
}

void printProducts(const std::vector<Product>& products) {
    if (products.empty()) {
        std::cout << "没有匹配的商品。\n";
        return;
    }
    for (const Product& p : products) {
        printProduct(p);
    }
}

void printStatistics(const Statistics& value) {
    std::cout << "商品种类数：" << value.productKinds
              << "；库存总数量：" << value.totalStock << "；库存总金额：";
    printMoney(value.totalValueFen);
    std::cout << " 元\n";
}

bool reportStatus(Status status, const std::string& task) {
    switch (status) {
        case Status::Ok:
            std::cout << "操作成功。\n";
            return true;
        case Status::NotImplemented:
            std::cout << "尚未实现：" << task << "。请填写对应 TODO。\n";
            break;
        case Status::NotFound:
            std::cout << "没有找到该编号的商品。\n";
            break;
        case Status::DuplicateId:
            std::cout << "商品编号重复。\n";
            break;
        case Status::InvalidInput:
            std::cout << "输入不合法，请检查字段和数值范围。\n";
            break;
        case Status::CapacityExceeded:
            std::cout << "商品表容量或单项库存数量超过上限。\n";
            break;
        case Status::InsufficientStock:
            std::cout << "库存不足，无法出库。\n";
            break;
        case Status::FileError:
            std::cout << "文件打开、读写或替换失败。\n";
            break;
    }
    return false;
}

bool readProductFields(Product& product, bool includeStock) {
    if (!readText("商品编号：", product.id)
        || !readText("商品名称：", product.name)
        || !readText("商品类别：", product.category)
        || !readNumber("单价（分，例如 300 表示 3.00 元）：", product.priceFen)) {
        return false;
    }
    if (includeStock && !readNumber("初始库存数量：", product.stock)) {
        return false;
    }
    return readNumber("预警数量：", product.warningStock);
}

void showMenu() {
    std::cout << "\n===== 超市库存管理 · 待填写框架 =====\n"
              << " 1. 新增商品（人工输入）\n"
              << " 2. 修改商品名称、类别、价格和预警值\n"
              << " 3. 按编号查询\n"
              << " 4. 商品入库\n"
              << " 5. 商品出库\n"
              << " 6. 低库存预警\n"
              << " 7. 按名称查询\n"
              << " 8. 按类别查询\n"
              << " 9. 排序显示\n"
              << "10. 总体统计\n"
              << "11. 分类统计\n"
              << "12. 文件导入（成功后替换当前商品表）\n"
              << "13. 保存到文件\n"
              << "14. 随机生成商品（追加）\n"
              << "15. 查看全部商品\n"
              << " 0. 退出\n";
}

} // namespace

int main() {
    ProductList list; // 整个系统唯一的商品表，传给所有业务函数。
    std::cout << "这是可编译的程序大纲。业务函数尚未填写，调用后会提示 TODO。\n";

    while (true) {
        showMenu();
        int choice = 0;
        if (!readNumber("请选择：", choice)) {
            return 0;
        }

        switch (choice) {
            case 0:
                std::cout << "已退出。需要保留数据时，请在退出前使用保存功能。\n";
                return 0;
            case 1: {
                Product product;
                if (!readProductFields(product, true)) return 0;
                reportStatus(addProduct(list, product), "A2 addProduct");
                break;
            }
            case 2: {
                Product product;
                if (!readProductFields(product, false)) return 0;
                reportStatus(updateProductInfo(list, product.id, product.name,
                             product.category, product.priceFen, product.warningStock),
                             "A3 updateProductInfo");
                break;
            }
            case 3: {
                std::string id;
                int index = -1;
                if (!readText("商品编号：", id)) return 0;
                if (reportStatus(findById(list, id, index), "A1 findById")) {
                    // 返回 Ok 时，模块必须保证 index 位于有效数据范围内。
                    if (index >= 0 && index < list.size) printProduct(list.data[index]);
                    else std::cout << "接口错误：findById 返回了无效下标，请检查 A1。\n";
                }
                break;
            }
            case 4:
            case 5: {
                std::string id;
                int quantity = 0;
                if (!readText("商品编号：", id)
                    || !readNumber("操作数量：", quantity)) return 0;
                if (choice == 4)
                    reportStatus(stockIn(list, id, quantity), "B1 stockIn");
                else
                    reportStatus(stockOut(list, id, quantity), "B2 stockOut");
                break;
            }
            case 6: {
                std::vector<Product> result;
                if (reportStatus(findLowStock(list, result), "B3 findLowStock"))
                    printProducts(result);
                break;
            }
            case 7:
            case 8: {
                std::string text;
                std::vector<Product> result;
                if (!readText(choice == 7 ? "名称关键词：" : "类别名称：", text)) return 0;
                const Status status = choice == 7
                    ? queryByName(list, text, result)
                    : queryByCategory(list, text, result);
                if (reportStatus(status, choice == 7 ? "C1 queryByName" : "C2 queryByCategory"))
                    printProducts(result);
                break;
            }
            case 9: {
                int key = 0;
                int order = 0;
                if (!readNumber("排序字段：1=价格，2=库存：", key)
                    || !readNumber("排序方向：1=升序，0=降序：", order)) return 0;
                if ((key != 1 && key != 2) || (order != 0 && order != 1)) {
                    reportStatus(Status::InvalidInput, "排序选项");
                    break;
                }
                std::vector<Product> result;
                if (reportStatus(sortProducts(list, key == 1 ? SortKey::Price : SortKey::Stock,
                                              order == 1, result), "C3 sortProducts"))
                    printProducts(result);
                break;
            }
            case 10: {
                Statistics result;
                if (reportStatus(calculateStatistics(list, result), "C4 calculateStatistics"))
                    printStatistics(result);
                break;
            }
            case 11: {
                std::vector<CategoryStatistics> result;
                if (reportStatus(calculateCategoryStatistics(list, result),
                                 "C5 calculateCategoryStatistics")) {
                    if (result.empty()) std::cout << "当前没有商品类别。\n";
                    for (const CategoryStatistics& item : result) {
                        std::cout << "类别：" << item.category << '\n';
                        printStatistics(item.totals);
                    }
                }
                break;
            }
            case 12:
            case 13: {
                std::string filename;
                if (!readText("文件路径（示例 data/sample_products.txt）：", filename)) return 0;
                if (choice == 12)
                    reportStatus(loadFromFile(list, filename), "D1 loadFromFile");
                else
                    reportStatus(saveToFile(list, filename), "D2 saveToFile");
                break;
            }
            case 14: {
                int count = 0;
                if (!readNumber("追加随机商品的数量：", count)) return 0;
                reportStatus(generateRandomProducts(list, count), "D3 generateRandomProducts");
                break;
            }
            case 15:
                std::cout << "当前商品种类数：" << list.size << '\n';
                for (int i = 0; i < list.size; ++i) printProduct(list.data[i]);
                break;
            default:
                std::cout << "没有这个菜单选项，请输入 0 到 15。\n";
                break;
        }
    }
}
