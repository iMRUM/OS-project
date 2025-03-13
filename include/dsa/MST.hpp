#ifndef MST_HPP
#define MST_HPP

#include "Graph.hpp"
#include <vector>
#include <set>
#include <tuple>
#include <queue>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using edge = std::tuple<int, int, int>;

class MST {
private:
    std::set<edge> edges;
    int totalWeight;
    int numVertices;
    adjList mstAdjList;

public:
    MST(){}
    MST(int vertices);
    MST(const std::vector<std::tuple<int, int, int, int>>& edges, int vertices);
    void addEdge(int u, int v, int weight);
    
    int getTotalWeight() const;
    
    const std::set<edge>& getEdges() const;
    
    int getNumVertices() const;
    
    const adjList& getMstAdjList() const;
    
    int findLongestDistance() const;
    
    double findAverageDistance() const;
    
    int findShortestPathWithMstEdge(const adjList& originalGraph, int src, int dest) const;
    
    std::string getTotalWeightAsString() const;
    
    std::string getAverageDistanceAsString() const;
    
    std::string getLongestDistanceAsString() const;
    std::string toString() const;
private:
    void dfs(int node, int distance, std::vector<bool>& visited, int& maxDist, int& farthestNode) const;
    
    std::vector<std::vector<int>> floydWarshall() const;
};

#endif // MST_HPP