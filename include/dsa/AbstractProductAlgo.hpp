//
// Created by imry on 3/11/25.
//

#ifndef MSTALGO_HPP
#define MSTALGO_HPP
#include "../Factory/AbstractProduct.hpp"
#include <tuple>
#include "MST.hpp"
using namespace std;

class AbstractProductAlgo : public AbstractProduct {
protected:
    virtual MST *execute(Graph &graph) = 0;

public:
    AbstractProductAlgo() = default;

    ~AbstractProductAlgo() override = 0;
};
#endif //MSTALGO_HPP
