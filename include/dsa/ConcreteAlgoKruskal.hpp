#ifndef CONCRETEALGOKRUSKAL_HPP
#define CONCRETEALGOKRUSKAL_HPP
#include "../factory/AbstractProductAlgo.hpp"
#include "UnionFind.hpp"
class ConcreteAlgoKruskal : public AbstractProductAlgo {
private:
    // Implementation of Kruskal's algorithm for finding MST
    // Assumptions: We receive the edges of a connected graph.
    // Complexity: O(m log n)
    vector<tuple<int, int, int, int>> kruskal(const vector<tuple<int, int, int, int>>& graph_edges,
                                                        int n);


public:
    ~ConcreteAlgoKruskal() override = default;
    MST * execute(Graph &graph) override;
};
#endif //CONCRETEALGOKRUSKAL_HPP