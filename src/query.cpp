#include "inventory.h"

#include <algorithm>
#include <cctype>
#include <map>

namespace {

bool isValidList(const ProductList& list) {
    return list.size >= 0 && list.size <= MAX_PRODUCTS;
}

bool isBlank(const std::string& value) {
    for (unsigned char character : value) {
        if (!std::isspace(character)) return false;
    }
    return true;
}

} // namespace

Status queryByName(const ProductList& list, const std::string& keyword,
                   std::vector<Product>& result) {
    if (!isValidList(list) || isBlank(keyword)) return Status::InvalidInput;

    std::vector<Product> matches;
    for (int i = 0; i < list.size; ++i) {
        if (list.data[i].name.find(keyword) != std::string::npos)
            matches.push_back(list.data[i]);
    }
    result.swap(matches);
    return Status::Ok;
}

Status queryByCategory(const ProductList& list, const std::string& category,
                       std::vector<Product>& result) {
    if (!isValidList(list) || isBlank(category)) return Status::InvalidInput;

    std::vector<Product> matches;
    for (int i = 0; i < list.size; ++i) {
        if (list.data[i].category == category) matches.push_back(list.data[i]);
    }
    result.swap(matches);
    return Status::Ok;
}

Status sortProducts(const ProductList& list, SortKey key, bool ascending,
                    std::vector<Product>& result) {
    if (!isValidList(list) || (key != SortKey::Price && key != SortKey::Stock))
        return Status::InvalidInput;

    std::vector<Product> sorted(list.data, list.data + list.size);
    std::sort(sorted.begin(), sorted.end(), [key, ascending](const Product& left,
                                                            const Product& right) {
        if (key == SortKey::Price && left.priceFen != right.priceFen)
            return ascending ? left.priceFen < right.priceFen : left.priceFen > right.priceFen;
        if (key == SortKey::Stock && left.stock != right.stock)
            return ascending ? left.stock < right.stock : left.stock > right.stock;
        return left.id < right.id;
    });
    result.swap(sorted);
    return Status::Ok;
}

Status calculateStatistics(const ProductList& list, Statistics& result) {
    if (!isValidList(list)) return Status::InvalidInput;

    Statistics totals;
    totals.productKinds = list.size;
    for (int i = 0; i < list.size; ++i) {
        totals.totalStock += list.data[i].stock;
        totals.totalValueFen += list.data[i].priceFen * list.data[i].stock;
    }
    result = totals;
    return Status::Ok;
}

Status calculateCategoryStatistics(const ProductList& list,
                                  std::vector<CategoryStatistics>& result) {
    if (!isValidList(list)) return Status::InvalidInput;

    std::map<std::string, Statistics> totalsByCategory;
    for (int i = 0; i < list.size; ++i) {
        const Product& product = list.data[i];
        Statistics& totals = totalsByCategory[product.category];
        ++totals.productKinds;
        totals.totalStock += product.stock;
        totals.totalValueFen += product.priceFen * product.stock;
    }

    std::vector<CategoryStatistics> categories;
    categories.reserve(totalsByCategory.size());
    for (const auto& entry : totalsByCategory) {
        CategoryStatistics item;
        item.category = entry.first;
        item.totals = entry.second;
        categories.push_back(item);
    }
    result.swap(categories);
    return Status::Ok;
}
