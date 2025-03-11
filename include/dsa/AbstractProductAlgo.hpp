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
    virtual MST *execute(Graph &graph);

public:
    AbstractProductAlgo() = default;

    virtual ~AbstractProductAlgo() = default;
};
#endif //MSTALGO_HPP
