#ifndef METHODREQUEST_HPP
#define METHODREQUEST_HPP
#include "Future.hpp"
#include "MSTServant.hpp"
/**
 * The Method Request abstract class defines an interface for
 * executing methods of an Active Object. It contains guard methods
 * to determine when synchronization constraints are met.
 *
 * For every Active Object method that requires synchronized access,
 * this class is subclassed to create concrete Method Request classes.
 */
class MethodRequest {
public:
    MethodRequest() = default;
    virtual ~MethodRequest() = default;

    // Evaluate synchronization constraint
    // Returns true if the request can be executed
    virtual bool guard() const = 0;

    // Execute the method
    virtual void call() = 0;
};

// Concrete Method Requests
// InitGraphRequest - Initialize a new graph
class InitGraphRequest : public MethodRequest {
private:
    MSTServant* servant;
    int vertices;

public:
    InitGraphRequest(MSTServant* servant, int vertices)
        : servant(servant), vertices(vertices) {}

    bool guard() const override {
        // Can always initialize a graph
        return true;
    }

    void call() override {
        servant->initGraph_i(vertices);
    }
};

// AddEdgeRequest - Add an edge to the graph
class AddEdgeRequest : public MethodRequest {
private:
    MSTServant* servant;
    int u, v, w;

public:
    AddEdgeRequest(MSTServant* servant, int u, int v, int w)
        : servant(servant), u(u), v(v), w(w) {}

    bool guard() const override {
        // Can only add edges if graph is initialized
        return servant->isGraphInitialized_i();
    }

    void call() override {
        servant->addEdge_i(u, v, w);
    }
};

// RemoveEdgeRequest - Remove an edge from the graph
class RemoveEdgeRequest : public MethodRequest {
private:
    MSTServant* servant;
    int u, v;

public:
    RemoveEdgeRequest(MSTServant* servant, int u, int v)
        : servant(servant), u(u), v(v) {}

    bool guard() const override {
        // Can only remove edges if graph is initialized
        return servant->isGraphInitialized_i();
    }

    void call() override {
        servant->removeEdge_i(u, v);
    }
};
class GetMSTRequest : public MethodRequest {
private:
    MSTServant* servant;
    std::string algorithm;
    Future<MST>* result;
    
public:
    GetMSTRequest(MSTServant* servant,
                     const std::string& algo,
                     Future<MST>* result) 
        : servant(servant), algorithm(algo), result(result) {}
    
    bool guard() const override {
        // Can only compute MST if graph is initialized
        return servant->isGraphInitialized_i();
    }
    
    void call() override {
        // Compute MST and store result in the future
        MST mst = servant->getMST_i(algorithm);
        result->set(mst);
    }
};

class GetWeightRequest : public MethodRequest {
private:
    MSTServant* servant;
    Future<int>* result;
    
public:
    GetWeightRequest(MSTServant* servant, Future<int>* result) 
        : servant(servant), result(result) {}
    
    bool guard() const override {
        // Can only get weight if MST has been computed
        return servant->hasMST_i();
    }
    
    void call() override {
        int weight = servant->getWeight_i();
        result->set(weight);
    }
};
// GetLongestDistRequest - Get the longest distance in the MST
class GetLongestDistRequest : public MethodRequest {
private:
    MSTServant* servant;
    Future<int>* result;

public:
    GetLongestDistRequest(MSTServant* servant, Future<int>* result)
        : servant(servant), result(result) {}

    bool guard() const override {
        // Can only get longest distance if MST has been computed
        return servant->hasMST_i();
    }

    void call() override {
        int distance = servant->getLongestDist_i();
        result->set(distance);
    }
};

// GetShortestDistRequest - Get the shortest distance in the MST
class GetShortestDistRequest : public MethodRequest {
private:
    int s, d;
    const adj_list &graph;
    MSTServant* servant;
    Future<int>* result;

public:
    GetShortestDistRequest(MSTServant* servant, Future<int>* result, const adj_list &graph, int s, int d)
        : s(s), d(d), graph(graph), servant(servant), result(result) {}

    bool guard() const override {
        // Can only get shortest distance if MST has been computed
        return servant->hasMST_i();
    }

    void call() override {
        int distance = servant->getShortestDist_i(graph, s, d);
        result->set(distance);
    }
};

// GetAvgDistRequest - Get the average distance in the MST
class GetAvgDistRequest : public MethodRequest {
private:
    MSTServant* servant;
    Future<double>* result;

public:
    GetAvgDistRequest(MSTServant* servant, Future<double>* result)
        : servant(servant), result(result) {}

    bool guard() const override {
        // Can only get average distance if MST has been computed
        return servant->hasMST_i();
    }

    void call() override {
        double avgDistance = servant->getAvgDist_i();
        result->set(avgDistance);
    }
};

// ToStringRequest - Get string representation of the MST
class ToStringRequest : public MethodRequest {
private:
    MSTServant* servant;
    Future<std::string>* result;

public:
    ToStringRequest(MSTServant* servant, Future<std::string>* result)
        : servant(servant), result(result) {}

    bool guard() const override {
        // Can get string representation if graph is initialized
        // (might want to check for MST depending on what toString_i returns)
        return servant->isGraphInitialized_i();
    }

    void call() override {
        std::string str = servant->toString_i();
        result->set(str);
    }
};
#endif //METHODREQUEST_HPP