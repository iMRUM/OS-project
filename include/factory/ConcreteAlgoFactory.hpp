//
// Created by imry on 3/11/25.
//

#ifndef CONCRETEALGOFACTORY_HPP
#define CONCRETEALGOFACTORY_HPP
#define KRUSKAL "kruskal"
#define PRIM "prim"
#include <map>

#include "AbstractProductAlgo.hpp"
#include "../dsa/ConcreteAlgoKruskal.hpp"
#include "../dsa/ConcreteAlgoPrim.hpp"
#include "AbstractFactory.hpp"

typedef void (*MSTAlgo)(const vector<tuple<int, int, int, int> > &edges, int n);


class ConcreteAlgoFactory : public AbstractFactory {
public:
    ~ConcreteAlgoFactory() override = default;

    ConcreteAlgoFactory() = default;

    AbstractProductAlgo *createProduct(const std::string &algo) override {
        if (algo == KRUSKAL) {
            return new ConcreteAlgoKruskal();
        }
        if (algo == PRIM) {
            return new ConcreteAlgoPrim();
        }
        return nullptr;
    }
};
#endif //CONCRETEALGOFACTORY_HPP
