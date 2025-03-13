#include "../../include/dsa/Graph.hpp"

Graph::Graph(int v) : vertices(v), edges(0) {
    graph.resize(v);
}

// Constructor with vertices and edges
Graph::Graph(int v, int e) : vertices(v), edges(e) {
    // Initialize the adjacency list with v empty vectors
    graph.resize(v);
}

// Constructor with existing adjacency list
Graph::Graph(adj_list &input_graph) {
    graph = input_graph;
    vertices = input_graph.size();
    
    // Count the total number of edges
    edges = 0;
    for (const auto &neighbors : graph) {
        edges += neighbors.size();
    }
}

// Add a directed edge from s to t with weight w
void Graph::addEdge(int s, int t, int w) {
    // Check if vertices are valid
    if (s >= vertices || t >= vertices || s < 0 || t < 0) {
        return; // Invalid vertices
    }
    
    // Check if edge already exists
    if (edgeExists(s, t)) {
        // Update the weight if the edge already exists
        for (auto &edge : graph[s]) {
            if (edge.first == t) {
                edge.second = w;
                return;
            }
        }
    } else {
        // Add new edge
        graph[s].emplace_back(t, w);
        edges++;
    }
}

// Remove an edge from s to t
void Graph::removeEdge(int s, int t) {
    if (s < 0 || s >= vertices || t < 0 || t >= vertices) {
        return;
    }

    auto& vertex_edges = graph[s];
    for (auto it = vertex_edges.begin(); it != vertex_edges.end(); ++it) {
        if (it->first == t) {
            vertex_edges.erase(it);
            edges--;
            return;
        }
    }
}

// Check if an edge exists from u to v
bool Graph::edgeExists(int u, int v) const {
    // Check if vertices are valid
    if (u >= vertices || v >= vertices || u < 0 || v < 0) {
        return false; // Invalid vertices
    }
    
    // Search for the edge in the adjacency list
    for (const auto &edge : graph[u]) {
        if (edge.first == v) {
            return true;
        }
    }
    return false;
}

std::pair<std::vector<std::tuple<int, int, int, int>>, int> Graph::getAsPair() {
    std::vector<std::tuple<int, int, int, int>> result;
    int edge_id = 0;

    for (int u = 0; u < vertices; u++) {
        for (const auto& edge : graph[u]) {
            int v = edge.first;
            int weight = edge.second;
            result.push_back(std::make_tuple(u, v, weight, edge_id++));
        }
    }

    return std::make_pair(result, vertices);
}
// Remove an edge from the adjacency list directly within removeEdge method
