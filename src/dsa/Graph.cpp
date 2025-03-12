#include "../../include/dsa/Graph.hpp"


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

std::pair<std::vector<std::tuple<int, int, int, int>>, int> Graph::getAsPair() {
    std::vector<std::tuple<int, int, int, int>> edgesList;
    int n = getVertices();

    if (n == 0) {
        // Return empty edges list and 0 vertices if graph is empty
        return {edgesList, 0};
    }

    // Extract edges from the Graph's adjacency list
    // To avoid duplicate edges in undirected graphs, we'll only add each edge once
    // by ensuring source < target
    std::set<std::pair<int, int>> addedEdges; // To track edges we've already added

    int edgeId = 0;
    for (int source = 0; source < n; ++source) {
        for (const auto& edge : graph[source]) {
            int target = edge.first;
            int weight = edge.second;

            // Create a canonical representation of the edge (smaller vertex first)
            std::pair<int, int> canonicalEdge;
            if (source < target) {
                canonicalEdge = {source, target};
            } else {
                canonicalEdge = {target, source};
            }

            // Only add this edge if we haven't added it yet
            if (addedEdges.find(canonicalEdge) == addedEdges.end()) {
                edgesList.emplace_back(source, target, weight, edgeId++);
                addedEdges.insert(canonicalEdge);
            }
        }
    }

    return {edgesList, n};
}

// Remove an edge from the adjacency list directly within removeEdge method
