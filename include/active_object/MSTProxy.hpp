// MSTProxy.hpp
#pragma once

#include "MSTServant.hpp"
#include "MethodRequest.hpp"
#include "Future.hpp"
#include "Scheduler.hpp"

class MSTProxy {
private:
    MSTServant* servant;
    MSTScheduler* scheduler;
    
public:
    MSTProxy(ConcreteAlgoFactory& factory) {
        servant = new MSTServant(factory);
        scheduler = new MSTScheduler();
        // Start the scheduler in its own thread
        scheduler->start();
    }

    ~MSTProxy() {
        // Stop the scheduler before deleting
        scheduler->stop();
        delete servant;
        delete scheduler;
    }

    // One-way method to initialize a graph
    void initGraph(int n) {
        MethodRequest* request = new InitGraphRequest(servant, n);
        scheduler->enqueue(request);
    }

    // One-way method to add an edge
    void addEdge(int u, int v, int w) {
        MethodRequest* request = new AddEdgeRequest(servant, u, v, w);
        scheduler->enqueue(request);
    }

    // One-way method to remove an edge
    void removeEdge(int u, int v) {
        MethodRequest* request = new RemoveEdgeRequest(servant, u, v);
        scheduler->enqueue(request);
    }

    // Two-way method that computes MST and returns a Future
    Future<MST> computeMST(const std::string& algorithm) {
        Future<MST> result;
        MethodRequest* request = new GetMSTRequest(servant, algorithm, &result);
        scheduler->enqueue(request);
        return result;
    }

    // Two-way method that returns MST weight
    Future<int> getWeight() {
        Future<int> result;
        MethodRequest* request = new GetWeightRequest(servant, &result);
        scheduler->enqueue(request);
        return result;
    }

    // Two-way method that returns the longest distance in MST
    Future<int> getLongestDist() {
        Future<int> result;
        MethodRequest* request = new GetLongestDistRequest(servant, &result);
        scheduler->enqueue(request);
        return result;
    }

    // Two-way method that returns the shortest distance in MST
    Future<int> getShortestDist(const adj_list &originalGraph, int src, int dest) {
        Future<int> result;
        MethodRequest* request = new GetShortestDistRequest(servant, &result, originalGraph, src, dest);
        scheduler->enqueue(request);
        return result;
    }

    // Two-way method that returns the average distance in MST
    Future<double> getAvgDist() {
        Future<double> result;
        MethodRequest* request = new GetAvgDistRequest(servant, &result);
        scheduler->enqueue(request);
        return result;
    }

    // Two-way method that returns the string representation of MST
    Future<std::string> toString() {
        Future<std::string> result;
        MethodRequest* request = new ToStringRequest(servant, &result);
        scheduler->enqueue(request);
        return result;
    }
};