#include "../../include/active_object/MSTProxy.hpp"

void MSTProxy::initGraph(int n) {
    MethodRequest *request = new InitGraphRequest(servant, n);
    scheduler->enqueue(request);
}

void MSTProxy::addEdge(int u, int v, int w) {
    MethodRequest *request = new AddEdgeRequest(servant, u, v, w);
    scheduler->enqueue(request);
}

void MSTProxy::removeEdge(int u, int v) {
    MethodRequest *request = new RemoveEdgeRequest(servant, u, v);
    scheduler->enqueue(request);
}

Future<MST> MSTProxy::computeMST(const std::string &algorithm) {
    Future<MST> result;
    MethodRequest *request = new GetMSTRequest(servant, algorithm, &result);
    scheduler->enqueue(request);
    return result;
}

Future<int> MSTProxy::getWeight() {
    Future<int> result;
    MethodRequest *request = new GetWeightRequest(servant, &result);
    scheduler->enqueue(request);
    return result;
}

Future<int> MSTProxy::getLongestDist() {
    Future<int> result;
    MethodRequest *request = new GetLongestDistRequest(servant, &result);
    scheduler->enqueue(request);
    return result;
}

Future<int> MSTProxy::getShortestDist(const adj_list &originalGraph, int src, int dest) {
    Future<int> result;
    MethodRequest *request = new GetShortestDistRequest(servant, &result, originalGraph, src, dest);
    scheduler->enqueue(request);
    return result;
}

Future<double> MSTProxy::getAvgDist() {
    Future<double> result;
    MethodRequest *request = new GetAvgDistRequest(servant, &result);
    scheduler->enqueue(request);
    return result;
}

Future<std::string> MSTProxy::toString() {
    Future<std::string> result;
    MethodRequest *request = new ToStringRequest(servant, &result);
    scheduler->enqueue(request);
    return result;
}
