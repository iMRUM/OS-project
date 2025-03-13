#include "../../include/dsa/ConcreteAlgoPrim.hpp"

vector<tuple<int, int, int, int>> ConcreteAlgoPrim::_prim(const vector<vector<Edge>> &adj, int n) {
    std::cout << "Running _prim with " << n << " vertices" << std::endl;
    vector<tuple<int, int, int, int>> spanning_tree;

    if (n <= 0) {
        std::cout << "Invalid number of vertices in _prim" << std::endl;
        return spanning_tree;
    }

    // Check if the graph has any edges
    bool has_edges = false;
    for (int i = 0; i < n; i++) {
        if (!adj[i].empty()) {
            has_edges = true;
            break;
        }
    }

    if (!has_edges) {
        std::cout << "Graph has no edges" << std::endl;
        return spanning_tree;
    }

    // Find a good starting vertex (one with edges)
    int start_vertex = 0;
    for (int i = 0; i < n; i++) {
        if (!adj[i].empty()) {
            start_vertex = i;
            break;
        }
    }

    std::cout << "Starting Prim's algorithm from vertex " << start_vertex << std::endl;

    // Initialize data structures
    vector<Edge> min_e(n, Edge()); // Initialize all with Edge() constructor
    min_e[start_vertex].w = 0;  // Start vertex has zero weight

    set<Edge> q;
    q.insert({0, start_vertex, -1});  // {weight, vertex, edge_id}

    std::cout << "Initialized queue with start vertex" << std::endl;

    vector<bool> selected(n, false);

    try {
        while (!q.empty()) {
            Edge current = *q.begin();
            int v = current.to;

            std::cout << "Processing vertex " << v << " with weight " << current.w << std::endl;

            if (v < 0 || v >= n) {
                std::cout << "Invalid vertex index: " << v << std::endl;
                q.erase(q.begin());
                continue;
            }

            if (selected[v]) {
                // If we've already selected this vertex, skip it
                q.erase(q.begin());
                continue;
            }

            selected[v] = true;
            q.erase(q.begin());

            if (min_e[v].to != -1) {
                std::cout << "Adding edge to MST: " << min_e[v].to << " -> " << v
                          << " with weight " << min_e[v].w << std::endl;
                spanning_tree.emplace_back(min_e[v].to, v, min_e[v].w, min_e[v].id);
            }

            // Process adjacent edges
            for (const Edge& e: adj[v]) {
                if (e.to < 0 || e.to >= n) {
                    std::cout << "Skipping edge with invalid target: " << e.to << std::endl;
                    continue;
                }

                if (!selected[e.to] && e.w < min_e[e.to].w) {
                    // Try-catch to handle any issues with set operations
                    try {
                        // Only try to erase if the vertex is already in the queue
                        if (min_e[e.to].w != INF) {
                            auto it = q.find({min_e[e.to].w, e.to, min_e[e.to].id});
                            if (it != q.end()) {
                                q.erase(it);
                            }
                        }

                        min_e[e.to] = {e.w, v, e.id};
                        q.insert({e.w, e.to, e.id});

                        std::cout << "Updated edge to " << e.to << " with new weight "
                                  << e.w << " from vertex " << v << std::endl;
                    } catch (const std::exception& ex) {
                        std::cout << "Exception in queue operations: " << ex.what() << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cout << "Exception in Prim's algorithm: " << ex.what() << std::endl;
    }

    std::cout << "Prim's algorithm complete, found " << spanning_tree.size()
              << " edges" << std::endl;
    return spanning_tree;
}

vector<tuple<int, int, int, int> > ConcreteAlgoPrim::prim(const vector<tuple<int, int, int, int> > &edges, int n) {
    {
        vector<vector<Edge> > adj(n);
        for (const auto &e: edges) {
            int a, b, c, id;
            tie(a, b, c, id) = e;
            adj[a].push_back(Edge(c, b, id));
            adj[b].push_back(Edge(c, a, id));
        }

        vector<tuple<int, int, int, int> > res = _prim(adj, n);

        return res;
    }
}

MST *ConcreteAlgoPrim::execute(Graph &graph) {
    if (graph.isEmpty()) {
        return new MST(0);
    }
    auto [edges, num_vertices] = graph.getAsPair();
    auto mst_edges = prim(edges, num_vertices);
    return new MST(mst_edges, num_vertices);
}