#include "../../include/active_object/MSTServant.hpp"

void MSTServant::initGraph_i(int n) {
    graph = Graph(n);
}

void MSTServant::addEdge_i(int u, int v, int w) {
    graph.addEdge(u-1, v-1, w);
    graph.addEdge(v-1, u-1, w);  // Undirected graph
}

void MSTServant::removeEdge_i(int u, int v) {
    graph.removeEdge(u-1, v-1);
    graph.removeEdge(v-1, u-1);  // Undirected graph
}

MST MSTServant::getMST_i(const std::string &algo) {
    AbstractProductAlgo* mst_algo = algo_factory.createProduct(algo);
    mst = *(mst_algo->execute(graph));
    delete mst_algo;
    return mst;
}

int MSTServant::getWeight_i() {
    return mst.getTotalWeight();
}

int MSTServant::getLongestDist_i() {
    return mst.findLongestDistance();
}

int MSTServant::getShortestDist_i(const adjList &original_graph, int src, int dest) {
    return mst.findShortestPathWithMstEdge(original_graph, src, dest);
}

double MSTServant::getAvgDist_i() {
    return mst.findAverageDistance();
}

std::string MSTServant::toString_i() {
    if (!isGraphInitialized_i()) {
        return "Graph not initialized\n";
    }

    std::stringstream ss;
    ss << "Vertices: " << graph.getVertices() << "\n";
    ss << "Edges:\n";

    const adjList& adj = graph.getGraph();
    for (int u = 0; u < graph.getVertices(); ++u) {
        for (const auto& edge : adj[u]) {
            int v = edge.first;
            int w = edge.second;
            // Only print each edge once (for undirected graphs)
            if (u < v) {
                ss << u << " -- " << v << " (weight: " << w << ")\n";
            }
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
