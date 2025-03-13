#include "../../include/dsa/ConcreteAlgoKruskal.hpp"
#include <algorithm>
#include <iostream>

vector<tuple<int, int, int, int>> ConcreteAlgoKruskal::kruskal(
    const vector<tuple<int, int, int, int>> &graph_edges, int n) {

    // Debug: Print input edges
    std::cout << "Kruskal input: " << graph_edges.size() << " edges, " << n << " vertices" << std::endl;

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
        int w = get<2>(edge);
        int id = get<3>(edge);

        // If including this edge doesn't create a cycle, add it to the MST
        if (uf.find_parent(u) != uf.find_parent(v)) {
            uf.unite(u, v);
            result.push_back(edge);

            // Debug: Print added edge
            std::cout << "Kruskal: Adding edge " << u << " -> " << v << " (weight: " << w << ")" << std::endl;
        }
    }

    // Debug: Print result size
    std::cout << "Kruskal result: " << result.size() << " edges in MST" << std::endl;

    return result;
}

MST* ConcreteAlgoKruskal::execute(Graph &graph) {
    // Get edge list and vertex count from graph
    auto [edges, n] = graph.getAsPair();

    // Debug: Print graph information
    std::cout << "Executing Kruskal's algorithm on graph with " << n << " vertices and "
              << edges.size() << " edges" << std::endl;

    // Execute Kruskal's algorithm
    vector<tuple<int, int, int, int>> mst_edges = kruskal(edges, n);

    // Create and return MST object
    return new MST(mst_edges, n);
}