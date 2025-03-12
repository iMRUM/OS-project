//
// Created by imry on 3/11/25.
//

#ifndef ABSTRACTFACTORY_HPP
#define ABSTRACTFACTORY_HPP
#include <string>

#include "AbstractProduct.hpp"

class AbstractFactory {
public:
    AbstractFactory() = default;

    virtual ~AbstractFactory() = default;
    virtual AbstractProduct *createProduct(int id) = 0;
};
#endif //ABSTRACTFACTORY_HPP
