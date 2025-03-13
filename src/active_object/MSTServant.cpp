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
    AbstractProductAlgo*
}

int MSTServant::getWeight_i() {
}

int MSTServant::getLongestDist_i() {
}

int MSTServant::getShortestDist_i() {
}

int MSTServant::getAvgDist_i() {
}

std::string MSTServant::toString_i() {
}

bool MSTServant::isGraphInitialized_i() const {
}

bool MSTServant::hasMST_i() const {
}
