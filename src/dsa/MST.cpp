#include "../../include/dsa/MST.hpp"
#include <iostream>

MST::MST(int vertices) : totalWeight(0), numVertices(vertices) {
    // Initialize empty MST
    mstAdjList.resize(vertices);
}

MST::MST(const std::vector<std::tuple<int, int, int, int>>& edges, int vertices)
    : totalWeight(0), numVertices(vertices) {

    // Initialize MST adjacency list
    mstAdjList.resize(vertices);

    // Add edges to MST
    for (const auto& edge : edges) {
        int u = std::get<0>(edge);
        int v = std::get<1>(edge);
        int weight = std::get<2>(edge);
        // Add edge to MST
        addEdge(u, v, weight);
    }
}

void MST::addEdge(int u, int v, int weight) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {
        std::cerr << "MST: Invalid vertices in addEdge: " << u << ", " << v << std::endl;
        return;
    }

    // Add to edge set
    edges.insert(std::make_tuple(u, v, weight));

    // Update total weight
    totalWeight += weight;

    // Add to adjacency list (bidirectional for MST)
    mstAdjList[u].push_back(std::make_pair(v, weight));
    mstAdjList[v].push_back(std::make_pair(u, weight));
}

int MST::getTotalWeight() const {
    return totalWeight;
}

const std::set<edge>& MST::getEdges() const {
    return edges;
}

int MST::getNumVertices() const {
    return numVertices;
}

const adj_list& MST::getMstAdjList() const {
    return mstAdjList;
}

int MST::findLongestDistance() const {
    if (numVertices == 0) return 0;

    int maxDistance = 0;

    for (int i = 0; i < numVertices; i++) {
        std::vector<bool> visited(numVertices, false);
        int farthestNode = -1;
        int distance = 0;

        // DFS from each node to find longest path
        dfs(i, 0, visited, distance, farthestNode);

        if (distance > maxDistance) {
            maxDistance = distance;
        }
    }

    return maxDistance;
}

void MST::dfs(int node, int distance, std::vector<bool>& visited, int& maxDist, int& farthestNode) const {
    visited[node] = true;

    if (distance > maxDist) {
        maxDist = distance;
        farthestNode = node;
    }

    for (const auto& neighbor : mstAdjList[node]) {
        int nextNode = neighbor.first;
        int edgeWeight = neighbor.second;

        if (!visited[nextNode]) {
            dfs(nextNode, distance + edgeWeight, visited, maxDist, farthestNode);
        }
    }
}

double MST::findAverageDistance() const {
    if (numVertices <= 1) return 0.0;

    // Use Floyd-Warshall to compute all pairs shortest paths
    std::vector<std::vector<int>> dist = floydWarshall();

    // Calculate sum of all distances
    long long sum = 0;
    int count = 0;

    for (int i = 0; i < numVertices; i++) {
        for (int j = i + 1; j < numVertices; j++) {
            if (dist[i][j] < std::numeric_limits<int>::max()) {
                sum += dist[i][j];
                count++;
            }
        }
    }

    // Return average
    return (count > 0) ? static_cast<double>(sum) / count : 0.0;
}

std::vector<std::vector<int>> MST::floydWarshall() const {
    const int INF = std::numeric_limits<int>::max();

    // Initialize distance matrix
    std::vector<std::vector<int>> dist(numVertices, std::vector<int>(numVertices, INF));

    // Set distances for direct edges
    for (int i = 0; i < numVertices; i++) {
        dist[i][i] = 0;  // Distance to self is 0

        for (const auto& edge : mstAdjList[i]) {
            int j = edge.first;
            int weight = edge.second;
            dist[i][j] = weight;
        }
    }

    // Floyd-Warshall algorithm
    for (int k = 0; k < numVertices; k++) {
        for (int i = 0; i < numVertices; i++) {
            for (int j = 0; j < numVertices; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    return dist;
}

int MST::findShortestPathWithMstEdge(const adj_list& originalGraph, int src, int dest) const {
    if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
        std::cerr << "Invalid source/destination for shortest path: "
                  << src << " to " << dest << std::endl;
        return -1;
    }

    // Use Dijkstra's algorithm for shortest path
    std::vector<int> dist(numVertices, std::numeric_limits<int>::max());
    std::vector<bool> visited(numVertices, false);

    // Priority queue for Dijkstra
    std::priority_queue<std::pair<int, int>,
                       std::vector<std::pair<int, int>>,
                       std::greater<std::pair<int, int>>> pq;

    // Initialize source
    dist[src] = 0;
    pq.push(std::make_pair(0, src));


    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (visited[u]) continue;
        visited[u] = true;

        // If destination reached
        if (u == dest) {
            return dist[dest];
        }

        // Check all neighbors in MST
        for (const auto& edge : mstAdjList[u]) {
            int v = edge.first;
            int weight = edge.second;

            if (!visited[v] && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }

    // If no path found
    return -1;
}

std::string MST::toString() const {
    std::stringstream ss;

    ss << "Vertices: " << numVertices << std::endl;
    ss << "Edges:" << std::endl;

    // Print MST edges (using a set to avoid duplicates in undirected representation)
    std::set<std::pair<int, int>> printed;

    for (int i = 0; i < numVertices; i++) {
        for (const auto& edge : mstAdjList[i]) {
            int neighbor = edge.first;
            int weight = edge.second;

            // For undirected MST, only print each edge once
            if (printed.find(std::make_pair(std::min(i, neighbor), std::max(i, neighbor))) == printed.end()) {
                ss << i << " -- " << neighbor << " (weight: " << weight << ")" << std::endl;
                printed.insert(std::make_pair(std::min(i, neighbor), std::max(i, neighbor)));
            }
        }
    }

    return ss.str();
}

std::string MST::getTotalWeightAsString() const {
    return "Weight: " + std::to_string(totalWeight);
}

std::string MST::getAverageDistanceAsString() const {
    return "Average distance: " + std::to_string(findAverageDistance());
}

std::string MST::getLongestDistanceAsString() const {
    return "Longest distance: " + std::to_string(findLongestDistance());
}