#ifndef SERVANT_HPP
#define SERVANT_HPP
#include "../dsa/Graph.hpp"
#include "../dsa/MST.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"

class MSTServant {
private:
    Graph graph;
    MST mst;
    ConcreteAlgoFactory& algo_factory;
public:
    MSTServant(ConcreteAlgoFactory& algo_factory): algo_factory(algo_factory) {}
    // Core operations that will be called by Method Requests
    void initGraph_i(int n);
    void addEdge_i(int u, int v, int w);
    void removeEdge_i(int u, int v);
    MST getMST_i(const std::string& algo);
    int getWeight_i();
    int getLongestDist_i();
    int getShortestDist_i(const adj_list &original_graph, int src, int dest);

    double getAvgDist_i();
    std::string toString_i();
    // Predicates that can be used in guards
    bool isGraphInitialized_i() const;
    bool hasMST_i() const;
};
#endif //SERVANT_HPP
