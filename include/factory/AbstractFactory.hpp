#ifndef ABSTRACTFACTORY_HPP
#define ABSTRACTFACTORY_HPP
#include <string>

#include "AbstractProduct.hpp"

class AbstractFactory {
public:
    AbstractFactory() = default;

    virtual ~AbstractFactory() = default;
    virtual AbstractProduct *createProduct(const std::string &id) = 0;
};
#endif //ABSTRACTFACTORY_HPP
