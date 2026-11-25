#include "inventory.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#endif

namespace {

const char* statusName(Status status) {
    switch (status) {
        case Status::Ok: return "Ok";
        case Status::NotImplemented: return "NotImplemented";
        case Status::NotFound: return "NotFound";
        case Status::DuplicateId: return "DuplicateId";
        case Status::InvalidInput: return "InvalidInput";
        case Status::CapacityExceeded: return "CapacityExceeded";
        case Status::InsufficientStock: return "InsufficientStock";
        case Status::FileError: return "FileError";
    }
    return "InternalError";
}

void writeString(std::ostream& output, const std::string& value) {
    output << '"';
    for (unsigned char byte : value) {
        if (byte == '"' || byte == '\\') {
            output << '\\' << static_cast<char>(byte);
        } else if (byte < 32) {
            output << "\\u00" << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(byte) << std::dec << std::setfill(' ');
        } else {
            output << static_cast<char>(byte);
        }
    }
    output << '"';
}

void writeProduct(std::ostream& output, const Product& value) {
    output << "{\"id\":";
    writeString(output, value.id);
    output << ",\"name\":";
    writeString(output, value.name);
    output << ",\"category\":";
    writeString(output, value.category);
    output << ",\"priceFen\":" << value.priceFen
           << ",\"stock\":" << value.stock
           << ",\"warningStock\":" << value.warningStock << '}';
}

void writeProducts(std::ostream& output, const Product* products, std::size_t count) {
    output << '[';
    for (std::size_t i = 0; i < count; ++i) {
        if (i > 0) output << ',';
        writeProduct(output, products[i]);
    }
    output << ']';
}

void writeStatistics(std::ostream& output, const Statistics& value) {
    output << "{\"productKinds\":" << value.productKinds
           << ",\"totalStock\":" << value.totalStock
           << ",\"totalValueFen\":\"" << value.totalValueFen << "\"}";
}

void writeCategoryStatistics(std::ostream& output,
                             const std::vector<CategoryStatistics>& categories) {
    output << '[';
    bool first = true;
    for (const CategoryStatistics& row : categories) {
        if (!first) output << ',';
        first = false;
        output << "{\"category\":";
        writeString(output, row.category);
        output << ",\"totals\":";
        writeStatistics(output, row.totals);
        output << '}';
    }
    output << ']';
}

void writeResult(std::ostream& output, Status status, const std::string& data = "") {
    output << "{\"status\":\"" << statusName(status) << '"';
    if (status == Status::Ok && !data.empty()) output << ",\"data\":" << data;
    output << '}';
}

void writeSnapshot(std::ostream& output, const ProductList& list) {
    Statistics totals;
    std::vector<CategoryStatistics> categories;
    std::vector<Product> lowStock;
    const Status totalsStatus = calculateStatistics(list, totals);
    const Status categoryStatus = calculateCategoryStatistics(list, categories);
    const Status lowStockStatus = findLowStock(list, lowStock);

    output << "{\"status\":\"Ok\",\"data\":{\"products\":";
    writeProducts(output, list.data, static_cast<std::size_t>(list.size));

    std::ostringstream data;
    if (totalsStatus == Status::Ok) writeStatistics(data, totals);
    output << ",\"totals\":";
    writeResult(output, totalsStatus, data.str());

    data.str("");
    if (categoryStatus == Status::Ok) writeCategoryStatistics(data, categories);
    output << ",\"categories\":";
    writeResult(output, categoryStatus, data.str());

    data.str("");
    if (lowStockStatus == Status::Ok) writeProducts(data, lowStock.data(), lowStock.size());
    output << ",\"lowStock\":";
    writeResult(output, lowStockStatus, data.str());
    output << "}}";
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t begin = 0;
    std::size_t end = line.find('\t');
    while (end != std::string::npos) {
        fields.push_back(line.substr(begin, end - begin));
        begin = end + 1;
        end = line.find('\t', begin);
    }
    fields.push_back(line.substr(begin));
    return fields;
}

template <typename Integer>
bool parseInteger(const std::string& text, Integer& value) {
    std::istringstream input(text);
    return (input >> std::noskipws >> value) && input.eof();
}

Status editProduct(ProductList& list, const std::vector<std::string>& fields, bool add) {
    if (fields.size() != (add ? 7U : 6U)) return Status::InvalidInput;
    Product product;
    product.id = fields[1];
    product.name = fields[2];
    product.category = fields[3];
    if (!parseInteger(fields[4], product.priceFen) ||
        !parseInteger(fields[5], product.warningStock)) {
        return Status::InvalidInput;
    }
    if (!add) {
        return updateProductInfo(list, product.id, product.name, product.category,
                                 product.priceFen, product.warningStock);
    }
    if (!parseInteger(fields[6], product.stock)) return Status::InvalidInput;
    return addProduct(list, product);
}

Status queryProducts(const ProductList& list, const std::vector<std::string>& fields,
                     std::vector<Product>& products) {
    const std::string& operation = fields[0];
    if (operation == "lowStock" && fields.size() == 1) return findLowStock(list, products);
    if (operation == "sort" && fields.size() == 3) {
        if ((fields[1] != "price" && fields[1] != "stock") ||
            (fields[2] != "asc" && fields[2] != "desc")) {
            return Status::InvalidInput;
        }
        const SortKey key = fields[1] == "price" ? SortKey::Price : SortKey::Stock;
        return sortProducts(list, key, fields[2] == "asc", products);
    }
    if (fields.size() != 2) return Status::InvalidInput;
    if (operation == "category") return queryByCategory(list, fields[1], products);
    if (operation == "name") return queryByName(list, fields[1], products);
    if (operation == "id") {
        int index = -1;
        const Status status = findById(list, fields[1], index);
        if (status != Status::Ok) return status;
        if (index < 0 || index >= list.size) return Status::InvalidInput;
        products.push_back(list.data[index]);
        return Status::Ok;
    }
    return Status::InvalidInput;
}

Status modifyList(ProductList& list, const std::vector<std::string>& fields) {
    const std::string& operation = fields[0];
    if (operation == "add" || operation == "update") {
        return editProduct(list, fields, operation == "add");
    }
    if (operation == "import" && fields.size() == 1) return loadFromFile(list, "import.tsv");
    if (operation == "export" && fields.size() == 1) return saveToFile(list, "export.tsv");
    int quantity = 0;
    if (operation == "generate" && fields.size() == 2 && parseInteger(fields[1], quantity)) {
        return generateRandomProducts(list, quantity);
    }
    if (fields.size() == 3 && parseInteger(fields[2], quantity)) {
        if (operation == "stockIn") return stockIn(list, fields[1], quantity);
        if (operation == "stockOut") return stockOut(list, fields[1], quantity);
    }
    return Status::InvalidInput;
}

void runAction(std::ostream& output, ProductList& list, const std::vector<std::string>& fields) {
    const std::string& operation = fields[0];
    if (operation == "snapshot" && fields.size() == 1) {
        writeSnapshot(output, list);
        return;
    }
    if (operation == "id" || operation == "name" || operation == "category" ||
        operation == "lowStock" || operation == "sort") {
        std::vector<Product> products;
        const Status status = queryProducts(list, fields, products);
        std::ostringstream data;
        if (status == Status::Ok) writeProducts(data, products.data(), products.size());
        writeResult(output, status, data.str());
        return;
    }
    writeResult(output, modifyList(list, fields));
}

} // namespace

int main() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    ProductList list;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        runAction(std::cout, list, splitFields(line));
        std::cout << '\n' << std::flush;
    }
}
