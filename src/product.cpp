#include "inventory.h"

namespace 
{
bool isValidText(const std::string& text) 
{
    bool hasVisibleChar = false;
    for (std::string::size_type i = 0; i < text.size(); ++i) 
    {
        char c = text[i];
        if (c == '\t' || c == '\r' || c == '\n') 
        {
            return false;
        }
        if (c != ' ') 
        {
            hasVisibleChar = true;
        }
    }
    return hasVisibleChar;
}
bool isValidList(const ProductList& list) 
{
    return list.size >= 0 && list.size <= MAX_PRODUCTS;
}

} 

Status findById(const ProductList& list, const std::string& id, int& index) 
{
    if (!isValidList(list) || !isValidText(id)) {
        return Status::InvalidInput;
    }

    for (int i = 0; i < list.size; ++i) 
    {
        if (list.data[i].id == id) {
            index = i;
            return Status::Ok;
        }
    }

    return Status::NotFound;
}

Status addProduct(ProductList& list, const Product& product) 
{
    (void)list;
    (void)product;
    return Status::NotImplemented;
}


Status updateProductInfo(ProductList& list, const std::string& id,
                         const std::string& name, const std::string& category,
                         long long priceFen, int warningStock) 
{
    (void)list;
    (void)id;
    (void)name;
    (void)category;
    (void)priceFen;
    (void)warningStock;
    return Status::NotImplemented;
}
