//
// Created by imry on 3/11/25.
//

#ifndef CONCRETEALGOFACTORY_HPP
#define CONCRETEALGOFACTORY_HPP
#include <map>

#include "AbstractProductAlgo.hpp"
#include "ConcreteAlgoKruskal.hpp"
#include "ConcreteAlgoPrim.hpp"
#include "../Factory/AbstractFactory.hpp"

typedef void (*MSTAlgo)(const vector<tuple<int, int, int, int>>& edges, int n);

class ConcreteAlgoFactory : public AbstractFactory {
private:
    std::map<int, AbstractProductAlgo*> _algorithms = {{0, new ConcreteAlgoPrim()},{1,new ConcreteAlgoKruskal()}};
protected:
    ~ConcreteAlgoFactory() override = default;

public:
    AbstractProduct * createProduct(int id) override;
};
#endif //CONCRETEALGOFACTORY_HPP
