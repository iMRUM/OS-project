#include "../../include/dsa/ConcreteAlgoPrim.hpp"
#include <iostream>
#include <vector>
#include <set>

vector<tuple<int, int, int, int> > ConcreteAlgoPrim::_prim(
    const vector<vector<Edge> > &adj, int n) {
    vector<tuple<int, int, int, int> > result;

    // Priority queue to find minimum weight edge
    std::set<Edge> q;

    // Array to keep track of which vertices are in MST
    vector<bool> selected(n, false);

    // Start with vertex 0
    int src = 0;
    selected[src] = true;

    // Add all edges from source to priority queue
    for (const Edge &e: adj[src]) {
        q.insert(e);
    }
    // Process n-1 edges to build MST
    while (!q.empty() && result.size() < n - 1) {
        // Get minimum weight edge
        Edge minEdge = *q.begin();
        q.erase(q.begin());

        int to = minEdge.to;

        // Skip if destination already in MST
        if (selected[to]) continue;

        // Add edge to MST
        int weight = minEdge.w;
        int id = minEdge.id;

        // Find the source vertex for this edge
        int from = -1;
        for (int i = 0; i < n; i++) {
            if (selected[i]) {
                for (const Edge &e: adj[i]) {
                    if (e.to == to && e.w == weight && e.id == id) {
                        from = i;
                        break;
                    }
                }
                if (from != -1) break;
            }
        }

        if (from == -1) {
            std::cerr << "Prim: Error finding source vertex for edge to " << to << std::endl;
            continue;
        }
        result.push_back(make_tuple(from, to, weight, id));
        selected[to] = true;

        // Add all edges from new vertex
        for (const Edge &e: adj[to]) {
            if (!selected[e.to]) {
                q.insert(e);
            }
        }
    }
    return result;
}

vector<tuple<int, int, int, int> > ConcreteAlgoPrim::prim(
    const vector<tuple<int, int, int, int> > &edges, int n) {
    // Build adjacency list from edge list
    vector<vector<Edge> > adj(n);

    for (const auto &edge: edges) {
        int u = get<0>(edge);
        int v = get<1>(edge);
        int w = get<2>(edge);
        int id = get<3>(edge);

        adj[u].push_back(Edge(w, v, id));
    }
    return _prim(adj, n);
}

MST *ConcreteAlgoPrim::execute(Graph &graph) {
    // Get edge list and vertex count from graph
    auto [edges, n] = graph.getAsPair();
    // Execute Prim's algorithm
    vector<tuple<int, int, int, int> > mst_edges = prim(edges, n);
    // Create and return MST object
    return new MST(mst_edges, n);
}
