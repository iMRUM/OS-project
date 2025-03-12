#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <algorithm>
#include <vector>
#include <tuple>
#include <set>
using adjList = std::vector<std::vector<std::pair<int, int>>>;

class Graph {
    adjList graph;
    int vertices, edges;

public:
    Graph(int v, int e);

    explicit Graph(adjList &graph);

    Graph() : vertices(0), edges(0) {}

    void addEdge(int s, int t, int w); // directed (s-source, t-target) weighted (w-weight) graph

    void removeEdge(int s, int t);

    int getVertices() const { return vertices; }

    const adjList& getGraph() { return graph; }

    bool isEmpty() const { return vertices == 0 && edges == 0; }

    std::pair<std::vector<std::tuple<int, int, int, int>>, int> getAsPair();
private:
    bool edgeExists(int u, int v) const;
};
#endif //GRAPH_HPP
