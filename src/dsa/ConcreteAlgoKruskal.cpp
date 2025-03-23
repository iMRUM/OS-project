#include "../../include/dsa/ConcreteAlgoKruskal.hpp"
#include <algorithm>
#include <iostream>

vector<tuple<int, int, int, int>> ConcreteAlgoKruskal::kruskal(
    const vector<tuple<int, int, int, int>> &graph_edges, int n) {
    // Initialize result vector for MST edges
    vector<tuple<int, int, int, int>> result;

    // Create a UnionFind structure for n vertices
    UnionFind uf(n);

    // Sort edges by weight
    vector<tuple<int, int, int, int>> edges = graph_edges;
    sort(edges.begin(), edges.end(),
         [](const tuple<int, int, int, int> &a, const tuple<int, int, int, int> &b) {
             return get<2>(a) < get<2>(b);
         });

    // Process edges in order of increasing weight
    for (const auto &edge : edges) {
        int u = get<0>(edge);
        int v = get<1>(edge);

        // If including this edge doesn't create a cycle, add it to the MST
        if (uf.find_parent(u) != uf.find_parent(v)) {
            uf.unite(u, v);
            result.push_back(edge);
        }
    }

    return result;
}

MST* ConcreteAlgoKruskal::execute(Graph &graph) {
    // Get edge list and vertex count from graph
    auto [edges, n] = graph.getAsPair();


    // Execute Kruskal's algorithm
    vector<tuple<int, int, int, int>> mst_edges = kruskal(edges, n);

    // Create and return MST object
    return new MST(mst_edges, n);
}