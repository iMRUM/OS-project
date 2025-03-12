//
// Created by imry on 3/11/25.
//

#ifndef CONCRETEALGOFACTORY_HPP
#define CONCRETEALGOFACTORY_HPP
#include <map>
#include <memory>

#include "AbstractProductAlgo.hpp"
#include "ConcreteAlgoKruskal.hpp"
#include "ConcreteAlgoPrim.hpp"
#include "../Factory/AbstractFactory.hpp"

//typedef void (*MSTAlgo)(const vector<tuple<int, int, int, int>>& edges, int n);

class ConcreteAlgoFactory : public AbstractFactory {
public:
    ~ConcreteAlgoFactory() override = default;
    ConcreteAlgoFactory() = default;

    AbstractProductAlgo* createProduct(int id) override {
        if (id == 0) {
            return new ConcreteAlgoPrim();
        }
        if (id == 1) {
            return new ConcreteAlgoKruskal();
        }
        return nullptr;
    }
};
#endif //CONCRETEALGOFACTORY_HPP
