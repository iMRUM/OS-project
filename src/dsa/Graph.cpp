#include "../../include/dsa/Graph.hpp"
#include <algorithm>

// Constructor with vertices and edges
Graph::Graph(int v, int e) : vertices(v), edges(0) {
    // Initialize the adjacency list with v empty vectors
    graph.resize(v);
}

// Constructor with existing adjacency list
Graph::Graph(adjList &inputGraph) {
    graph = inputGraph;
    vertices = inputGraph.size();
    
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
        graph[s].push_back({t, w});
        edges++;
    }
}

// Remove an edge from s to t
void Graph::removeEdge(int s, int t) {
    // Check if vertices are valid
    if (s >= vertices || t >= vertices || s < 0 || t < 0) {
        return; // Invalid vertices
    }
    
    // Check if edge exists
    if (edgeExists(s, t)) {
        // Find and remove the edge
        auto &neighbors = graph[s];
        neighbors.erase(
            std::remove_if(
                neighbors.begin(), 
                neighbors.end(),
                [t](const std::pair<int, int> &edge) { return edge.first == t; }
            ),
            neighbors.end()
        );
        edges--;
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

// Remove an edge from the adjacency list directly within removeEdge method