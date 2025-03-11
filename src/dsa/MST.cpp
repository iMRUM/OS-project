#include "../../include/dsa/MST.hpp"

MST::MST(int vertices) : totalWeight(0), numVertices(vertices) {
    if (vertices < 0) {
        throw std::invalid_argument("Number of vertices cannot be negative");
    }
    mstAdjList.resize(vertices);
}

void MST::addEdge(int u, int v, int weight) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {
        throw std::out_of_range("Vertex index out of range");
    }

    if (u > v) std::swap(u, v);
    edges.insert(std::make_tuple(u, v, weight));
    totalWeight += weight;

    mstAdjList[u].emplace_back(v, weight);
    mstAdjList[v].emplace_back(u, weight);
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


const adjList& MST::getMstAdjList() const {
    return mstAdjList;
}

int MST::findLongestDistance() const {
    if (numVertices <= 1) return 0;

    int maxDistance = 0;
    int farthestNode = -1;

    std::vector<bool> visited(numVertices, false);
    dfs(0, 0, visited, maxDistance, farthestNode);

    maxDistance = 0;
    visited.assign(numVertices, false);
    dfs(farthestNode, 0, visited, maxDistance, farthestNode);

    return maxDistance;
}

double MST::findAverageDistance() const {
    if (numVertices <= 1) return 0.0;

    std::vector<std::vector<int>> dist = floydWarshall();

    long long totalDistance = 0;
    int totalPairs = 0;

    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < numVertices; j++) {
            if (i == j) continue;  // Skip self-loops
            if (dist[i][j] >= std::numeric_limits<int>::max() / 2) continue;

            totalDistance += dist[i][j];
            totalPairs++;
        }
    }

    return (totalPairs > 0) ? (double)totalDistance / totalPairs : 0.0;
}

int MST::findShortestPathWithMstEdge(const adjList& originalGraph, int src, int dest) const {
    if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
        throw std::out_of_range("Vertex index out of range");
    }

    // Define constant for infinity
    const int INF = std::numeric_limits<int>::max();

    // Mark MST edges in a set for quick lookup
    std::set<std::pair<int, int>> mstEdges;
    for (int u = 0; u < numVertices; ++u) {
        for (const auto &neighbor : mstAdjList[u]) {
            int v = neighbor.first;
            mstEdges.insert({u, v});
            mstEdges.insert({v, u});  // Because it's an undirected graph
        }
    }

    // Priority queue for Dijkstra's algorithm
    // Elements are tuples of (distance, vertex, usedMstEdge)
    using State = std::tuple<int, int, bool>;  // (distance, vertex, hasUsedMstEdge)
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    // Distance array: dist[node][mstUsed] -> shortest distance to node
    // mstUsed = 1 means path has used an MST edge
    std::vector<std::vector<int>> dist(numVertices, std::vector<int>(2, INF));

    // Start Dijkstra from the source
    dist[src][0] = 0;  // Distance to src without using an MST edge is 0
    pq.push({0, src, false});  // Initially, we haven't used an MST edge

    while (!pq.empty()) {
        auto [currentDist, node, hasUsedMstEdge] = pq.top();
        pq.pop();

        // If we've reached the destination and used an MST edge, return the result
        if (node == dest && hasUsedMstEdge) {
            return currentDist;
        }

        // Explore neighbors of the current node
        for (const auto &neighbor : originalGraph[node]) {
            int adjNode = neighbor.first;
            int weight = neighbor.second;
            bool isMstEdge = (mstEdges.count({node, adjNode}) > 0);  // Check if edge belongs to MST

            // Case 1: If we haven't used an MST edge yet, and this edge is part of the MST, use it
            if (!hasUsedMstEdge && isMstEdge && currentDist + weight < dist[adjNode][1]) {
                dist[adjNode][1] = currentDist + weight;  // Mark this path as using an MST edge
                pq.push({dist[adjNode][1], adjNode, true});
            }

            // Case 2: If we have already used an MST edge, continue exploring
            if (hasUsedMstEdge && currentDist + weight < dist[adjNode][1]) {
                dist[adjNode][1] = currentDist + weight;
                pq.push({dist[adjNode][1], adjNode, true});
            }

            // Case 3: If we haven't used an MST edge yet, explore non-MST edges too
            if (!hasUsedMstEdge && currentDist + weight < dist[adjNode][0]) {
                dist[adjNode][0] = currentDist + weight;  // Mark this path as not using an MST edge yet
                pq.push({dist[adjNode][0], adjNode, false});
            }
        }
    }

    // If no valid path is found, return -1
    return -1;
}

std::string MST::getTotalWeightAsString() const {
    std::ostringstream oss;
    oss << "Total Weight: " << totalWeight;
    return oss.str();
}

std::string MST::getAverageDistanceAsString() const {
    std::ostringstream oss;
    oss << "Average Distance: " << std::fixed << std::setprecision(2) << findAverageDistance();
    return oss.str();
}

std::string MST::getLongestDistanceAsString() const {
    std::ostringstream oss;
    oss << "Longest Distance: " << findLongestDistance();
    return oss.str();
}

void MST::dfs(int node, int distance, std::vector<bool>& visited, int& maxDist, int& farthestNode) const {
    visited[node] = true;

    if (distance > maxDist) {
        maxDist = distance;
        farthestNode = node;
    }

    for (const auto& neighbor : mstAdjList[node]) {
        int nextNode = neighbor.first;
        int weight = neighbor.second;  // Use actual edge weight
        if (!visited[nextNode]) {
            dfs(nextNode, distance + weight, visited, maxDist, farthestNode);  // Add actual weight
        }
    }
}

std::vector<std::vector<int>> MST::floydWarshall() const {
    const int INF = std::numeric_limits<int>::max() / 2;

    std::vector<std::vector<int>> dist(numVertices, std::vector<int>(numVertices, INF));

    for (int i = 0; i < numVertices; i++) {
        dist[i][i] = 0;
    }

    for (int u = 0; u < numVertices; u++) {
        for (const auto& edge : mstAdjList[u]) {
            int v = edge.first;
            int weight = edge.second;
            dist[u][v] = weight;
        }
    }

    for (int k = 0; k < numVertices; k++) {
        for (int i = 0; i < numVertices; i++) {
            for (int j = 0; j < numVertices; j++) {
                if (dist[i][k] < INF && dist[k][j] < INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    }
            }
        }
    }

    return dist;
}