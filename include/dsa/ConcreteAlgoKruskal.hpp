//
// Created by imry on 3/11/25.
//

#ifndef CONCRETEALGOKRUSKAL_HPP
#define CONCRETEALGOKRUSKAL_HPP
#include "AbstractProductAlgo.hpp"
#include "UnionFind.hpp"
class ConcreteAlgoKruskal : public AbstractProductAlgo {
public:
    // Implementation of Kruskal's algorithm for finding MST
    // Assumptions: We receive the edges of a connected graph.
    // Complexity: O(m log n)
    vector<tuple<int, int, int, int>> kruskal(const vector<tuple<int, int, int, int>>& graph_edges,
                                                        int n);
};
#endif //CONCRETEALGOKRUSKAL_HPP
