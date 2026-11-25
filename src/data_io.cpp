#include "inventory.h"

#include <cstdio>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

const char* const FILE_HEADER = "id\tname\tcategory\tprice_fen\tstock\twarning_stock";

bool isBlank(const std::string& value) {
    for (unsigned char character : value) {
        if (character != ' ' && character != '\t' && character != '\r'
            && character != '\n' && character != '\f' && character != '\v') {
            return false;
        }
    }
    return true;
}

bool isValidTextField(const std::string& value) {
    return !isBlank(value)
        && value.find('\t') == std::string::npos
        && value.find('\r') == std::string::npos
        && value.find('\n') == std::string::npos;
}

bool isValidProduct(const Product& product) {
    return isValidTextField(product.id)
        && isValidTextField(product.name)
        && isValidTextField(product.category)
        && product.priceFen >= 0 && product.priceFen <= MAX_PRICE_FEN
        && product.stock >= 0 && product.stock <= MAX_STOCK
        && product.warningStock >= 0 && product.warningStock <= MAX_STOCK;
}

bool parseNonNegativeInteger(const std::string& field, long long& value) {
    if (field.empty()) return false;

    long long parsed = 0;
    for (unsigned char character : field) {
        if (character < '0' || character > '9') return false;
        const int digit = character - '0';
        if (parsed > (LLONG_MAX - digit) / 10) return false;
        parsed = parsed * 10 + digit;
    }
    value = parsed;
    return true;
}

bool splitFields(const std::string& line, std::vector<std::string>& fields) {
    fields.clear();
    std::size_t start = 0;
    while (true) {
        const std::size_t tab = line.find('\t', start);
        fields.push_back(line.substr(start,
            tab == std::string::npos ? tab : tab - start));
        if (tab == std::string::npos) return fields.size() == 6;
        start = tab + 1;
    }
}

bool replaceFile(const std::string& temporaryName, const std::string& filename) {
#ifdef _WIN32
    return MoveFileExA(temporaryName.c_str(), filename.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(temporaryName.c_str(), filename.c_str()) == 0;
#endif
}

std::string makeTemporaryName(const std::string& filename, std::mt19937& engine) {
    std::uniform_int_distribution<unsigned long long> suffix(0, ULLONG_MAX);
    std::ostringstream name;
    name << filename << ".tmp." << suffix(engine);
    return name.str();
}

} // namespace

Status loadFromFile(ProductList& list, const std::string& filename) {
    if (filename.empty() || isBlank(filename)) return Status::InvalidInput;

    std::ifstream input(filename.c_str(), std::ios::binary);
    if (!input.is_open()) return Status::FileError;

    std::string line;
    if (!std::getline(input, line))
        return input.bad() ? Status::FileError : Status::InvalidInput;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.size() >= 3
        && static_cast<unsigned char>(line[0]) == 0xEF
        && static_cast<unsigned char>(line[1]) == 0xBB
        && static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }
    if (line != FILE_HEADER) return Status::InvalidInput;

    ProductList imported;
    std::set<std::string> ids;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::vector<std::string> fields;
        if (!splitFields(line, fields)) return Status::InvalidInput;
        if (imported.size == MAX_PRODUCTS) return Status::CapacityExceeded;

        Product product;
        product.id = fields[0];
        product.name = fields[1];
        product.category = fields[2];
        long long stock = 0;
        long long warningStock = 0;
        if (!parseNonNegativeInteger(fields[3], product.priceFen)
            || !parseNonNegativeInteger(fields[4], stock)
            || !parseNonNegativeInteger(fields[5], warningStock)
            || product.priceFen > MAX_PRICE_FEN
            || stock > MAX_STOCK || warningStock > MAX_STOCK) {
            return Status::InvalidInput;
        }
        product.stock = static_cast<int>(stock);
        product.warningStock = static_cast<int>(warningStock);
        if (!isValidProduct(product)) return Status::InvalidInput;
        if (!ids.insert(product.id).second) return Status::DuplicateId;
        imported.data[imported.size++] = product;
    }
    if (input.bad()) return Status::FileError;

    list = imported;
    return Status::Ok;
}

Status saveToFile(const ProductList& list, const std::string& filename) {
    if (list.size < 0 || list.size > MAX_PRODUCTS
        || filename.empty() || isBlank(filename)) {
        return Status::InvalidInput;
    }

    std::set<std::string> ids;
    for (int i = 0; i < list.size; ++i) {
        if (!isValidProduct(list.data[i])) return Status::InvalidInput;
        if (!ids.insert(list.data[i].id).second) return Status::DuplicateId;
    }

    std::random_device seed;
    std::mt19937 engine(seed());
    std::string temporaryName;
    std::ofstream output;
    for (int attempt = 0; attempt < 10 && !output.is_open(); ++attempt) {
        temporaryName = makeTemporaryName(filename, engine);
        std::ifstream existing(temporaryName.c_str(), std::ios::binary);
        if (existing.good()) continue;
        output.open(temporaryName.c_str(),
                    std::ios::out | std::ios::binary | std::ios::trunc);
    }
    if (!output.is_open()) return Status::FileError;

    output << FILE_HEADER << '\n';
    for (int i = 0; i < list.size; ++i) {
        const Product& product = list.data[i];
        output << product.id << '\t' << product.name << '\t' << product.category << '\t'
               << product.priceFen << '\t' << product.stock << '\t'
               << product.warningStock << '\n';
    }
    output.flush();
    bool writeSucceeded = output.good();
    output.close();
    writeSucceeded = writeSucceeded && !output.fail();
    if (!writeSucceeded) {
        std::remove(temporaryName.c_str());
        return Status::FileError;
    }

    if (!replaceFile(temporaryName, filename)) {
        std::remove(temporaryName.c_str());
        return Status::FileError;
    }
    return Status::Ok;
}

Status generateRandomProducts(ProductList& list, int count) {
    if (list.size < 0 || list.size > MAX_PRODUCTS
        || count <= 0 || count > MAX_PRODUCTS) {
        return Status::InvalidInput;
    }
    if (count > MAX_PRODUCTS - list.size) return Status::CapacityExceeded;

    std::set<std::string> ids;
    for (int i = 0; i < list.size; ++i) {
        if (!isValidProduct(list.data[i])) return Status::InvalidInput;
        if (!ids.insert(list.data[i].id).second) return Status::DuplicateId;
    }

    static const char* const names[] = {
        "随机饮料", "随机食品", "随机日用品", "随机零食"
    };
    static const char* const categories[] = {"饮料", "食品", "日用品", "零食"};
    std::random_device seed;
    std::mt19937 engine(seed());
    std::uniform_int_distribution<int> nameIndex(0, 3);
    std::uniform_int_distribution<int> price(100, 100000);
    std::uniform_int_distribution<int> stock(0, 1000);
    std::uniform_int_distribution<int> warning(0, 100);
    std::uniform_int_distribution<unsigned long long> idNumber(0, 999999999999ULL);

    std::vector<Product> generated;
    generated.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        Product product;
        do {
            std::ostringstream id;
            id << 'R' << idNumber(engine);
            product.id = id.str();
        } while (ids.find(product.id) != ids.end());
        ids.insert(product.id);

        const int selected = nameIndex(engine);
        product.name = names[selected];
        product.category = categories[selected];
        product.priceFen = price(engine);
        product.stock = stock(engine);
        product.warningStock = warning(engine);
        generated.push_back(product);
    }

    for (const Product& product : generated) list.data[list.size++] = product;
    return Status::Ok;
}
