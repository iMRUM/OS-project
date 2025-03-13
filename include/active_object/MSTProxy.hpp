// MSTProxy.hpp
#pragma once

#include "MSTServant.hpp"
#include "MethodRequest.hpp"
#include "Future.hpp"
#include "Scheduler.hpp"

class MSTProxy {
private:
    MSTServant *servant;
    MSTScheduler *scheduler;

public:
    MSTProxy(ConcreteAlgoFactory &factory) {
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
    void initGraph(int n);

    // One-way method to add an edge
    void addEdge(int u, int v, int w);

    // One-way method to remove an edge
    void removeEdge(int u, int v);

    // Two-way method that computes MST and returns a Future
    Future<MST> computeMST(const std::string &algorithm);

    // Two-way method that returns MST weight
    Future<int> getWeight();

    // Two-way method that returns the longest distance in MST
    Future<int> getLongestDist();

    // Two-way method that returns the shortest distance in MST
    Future<int> getShortestDist(const adj_list &originalGraph, int src, int dest);

    // Two-way method that returns the average distance in MST
    Future<double> getAvgDist();

    // Two-way method that returns the string representation of MST
    Future<std::string> toString();
};
