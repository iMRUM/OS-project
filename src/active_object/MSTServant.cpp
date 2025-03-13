#include "../../include/active_object/MSTServant.hpp"
#include <queue>
#include <iostream>

void MSTServant::initGraph_i(int n) {
    graph = Graph(n);
    // Reset MST when graph is reinitialized
    mst = MST();
}

void MSTServant::addEdge_i(int u, int v, int w) {
    graph.addEdge(u, v, w);
}

void MSTServant::removeEdge_i(int u, int v) {
    graph.removeEdge(u, v);
}

MST MSTServant::getMST_i(const std::string& algo) {
    // Get correct algorithm implementation from factory
    AbstractProductAlgo* algorithm = algo_factory.createProduct(algo);

    if (algorithm) {
        // Execute MST algorithm
        MST* mst_ptr = algorithm->execute(graph);
        if (mst_ptr) {
            mst = *mst_ptr;
            delete mst_ptr;
        }
        delete algorithm;
    }

    return mst;
}

int MSTServant::getWeight_i() {
    return mst.getTotalWeight();
}

int MSTServant::getLongestDist_i() {
    return mst.findLongestDistance();
}

int MSTServant::getShortestDist_i(const adj_list &original_graph, int src, int dest) {
    return mst.findShortestPathWithMstEdge(original_graph, src, dest);
}

double MSTServant::getAvgDist_i() {
    return mst.findAverageDistance();
}

std::string MSTServant::toString_i() {
    std::stringstream ss;

    // Print graph information
    ss << "Vertices: " << graph.getVertices() << std::endl;
    ss << "Edges:" << std::endl;

    const adj_list& adj_list = graph.getGraph();

    // Since the graph is directed, print all edges
    for (int i = 0; i < graph.getVertices(); i++) {
        for (const auto& edge : adj_list[i]) {
            int neighbor = edge.first;
            int weight = edge.second;
            ss << i << " -> " << neighbor << " (weight: " << weight << ")" << std::endl;
        }
    }

    return ss.str();
}

bool MSTServant::isGraphInitialized_i() const {
    return !graph.isEmpty();
}

bool MSTServant::hasMST_i() const {
    return mst.getNumVertices() > 0;
}