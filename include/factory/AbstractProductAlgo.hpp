#ifndef MSTALGO_HPP
#define MSTALGO_HPP
#include "../factory/AbstractProduct.hpp"
#include <tuple>
#include <set>
#include <vector>
#include <algorithm>
#include "../dsa/MST.hpp"
using namespace std;

class AbstractProductAlgo : public AbstractProduct {

public:
    AbstractProductAlgo() = default;
    virtual MST *execute(Graph &graph) = 0;
    ~AbstractProductAlgo() override = 0;
};
#endif //MSTALGO_HPP