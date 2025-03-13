#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <algorithm>
#include <vector>
#include <tuple>
#include <set>
using adj_list = std::vector<std::vector<std::pair<int, int>>>;

class Graph {
    adj_list graph;
    int vertices, edges;

public:
    Graph(int v);
    Graph(int v, int e);

    explicit Graph(adj_list &graph);

    Graph() : vertices(0), edges(0) {}

    void addEdge(int s, int t, int w); // directed (s-source, t-target) weighted (w-weight) graph

    void removeEdge(int s, int t);

    int getVertices() const { return vertices; }

    const adj_list& getGraph() { return graph; }

    bool isEmpty() const { return vertices == 0 && edges == 0; }

    std::pair<std::vector<std::tuple<int, int, int, int>>, int> getAsPair();
private:
    bool edgeExists(int u, int v) const;
};
#endif //GRAPH_HPP