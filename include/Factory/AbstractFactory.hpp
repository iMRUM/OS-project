//
// Created by imry on 3/11/25.
//

#ifndef ABSTRACTFACTORY_HPP
#define ABSTRACTFACTORY_HPP
#include <string>

#include "AbstractProduct.hpp"

class AbstractFactory {
protected:
    AbstractFactory() = default;

    virtual ~AbstractFactory() = default;

public:
    virtual AbstractProduct *createProduct(int id) = 0;
};
#endif //ABSTRACTFACTORY_HPP
