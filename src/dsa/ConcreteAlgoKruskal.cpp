#include "../../include/dsa/ConcreteAlgoKruskal.hpp"

#include <iostream>

vector<tuple<int, int, int, int> > ConcreteAlgoKruskal::kruskal(const vector<tuple<int, int, int, int> > &graph_edges,
                                                                int n) {
    // Guard against empty input
    std::cout << "kruskal" << std::endl;
    if (graph_edges.empty() || n <= 0) {
        return {};
    }

    UnionFind graph(n);
    vector<tuple<int, int, int, int> > edges;
    vector<tuple<int, int, int, int> > spanning_tree;

    // Create a copy of the edges
    edges = graph_edges;

    // Sort edges by weight
    sort(edges.begin(), edges.end(),
         [&](const tuple<int, int, int, int> &a,
             const tuple<int, int, int, int> &b) {
             return get<2>(a) < get<2>(b);
         });

    // Process each edge
    for (const auto &edge: edges) {
        int from, to, cost, id;
        tie(from, to, cost, id) = edge;

        // Safety check for vertex indices
        if (from < 0 || from >= n || to < 0 || to >= n) {
            continue; // Skip invalid edges
        }

        // Try to unite vertices
        if (graph.unite(from, to)) {
            spanning_tree.emplace_back(from, to, cost, id);
        }
    }

    return spanning_tree;
}

MST *ConcreteAlgoKruskal::execute(Graph &graph) {
    std::cout << "execute" << std::endl;
    // Check for empty graph
    if (graph.isEmpty()) {
        return new MST(0);
    }

    // Get graph data
    auto graphData = graph.getAsPair();
    auto edges = graphData.first;
    int num_vertices = graphData.second;

    // Check for valid graph data
    if (num_vertices <= 0) {
        return new MST(0);
    }

    // Run Kruskal's algorithm
    auto mst_edges = kruskal(edges, num_vertices);

    // Create and return MST
    return new MST(mst_edges, num_vertices);
}