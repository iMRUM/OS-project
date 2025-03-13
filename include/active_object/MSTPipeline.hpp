#ifndef MSTPIPELINE_HPP
#define MSTPIPELINE_HPP

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include "MSTProxy.hpp"
#include "../commands.hpp"

class MSTPipeline {
private:
    std::map<int, std::unique_ptr<MSTProxy>> proxies;
    std::mutex proxies_mutex;
    ConcreteAlgoFactory algoFactory;

public:
    MSTPipeline() = default;
    ~MSTPipeline() = default;

    // Get or create proxy for a client
    MSTProxy* getProxy(int client_fd) {
        std::lock_guard<std::mutex> lock(proxies_mutex);
        auto it = proxies.find(client_fd);
        if (it == proxies.end()) {
            // Create new proxy for this client
            auto proxy = std::make_unique<MSTProxy>(algoFactory);
            MSTProxy* result = proxy.get();
            proxies[client_fd] = std::move(proxy);
            return result;
        }
        return it->second.get();
    }

    // Remove a client's proxy when disconnected
    void removeProxy(int client_fd) {
        std::lock_guard<std::mutex> lock(proxies_mutex);
        proxies.erase(client_fd);
    }

    // Process a command from a client
    #include "../../include/active_object/MSTPipeline.hpp"
#include <iostream>

// Process a command from a client
void processCommand(const std::string& input, int client_fd, function<void(std::string)> callback) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;

    std::cout << "MSTPipeline received command: '" << command << "'" << std::endl;

    MSTProxy* proxy = getProxy(client_fd);

    if (command == "new_graph") {
        int n;
        iss >> n;
        proxy->initGraph(n);

        // Process edges if provided
        int m;
        if (iss >> m) {
            for (int i = 0; i < m; i++) {
                int u, v, w;
                iss >> u >> v >> w;
                proxy->addEdge(u, v, w);
            }
        }

        callback("New graph created\n");
    }
    else if (command == "add_edge") {
        int u, v, w;
        iss >> u >> v >> w;
        proxy->addEdge(u, v, w);
        callback("Edge added\n");
    }
    else if (command == "remove_edge") {
        int u, v;
        iss >> u >> v;
        proxy->removeEdge(u, v);
        callback("Edge removed\n");
    }
    else if (command == "mst_kruskal" || command == "mst_prim") {
        std::string algo = (command == "mst_kruskal") ? "kruskal" : "prim";

        // Start a new thread to wait for the Future result
        std::thread([this, proxy, algo, callback]() {
            Future<MST> result = proxy->computeMST(algo);

            // Wait for MST computation to complete
            MST mst = result.get();

            // Calculate MST metrics asynchronously
            Future<int> weightResult = proxy->getWeight();
            Future<int> longestResult = proxy->getLongestDist();
            Future<int> shortestResult = proxy->getShortestDist(mst.getMstAdjList(), 0, mst.getNumVertices()-1);
            Future<double> avgResult = proxy->getAvgDist();
            Future<std::string> mstStrResult = proxy->toString();

            // Build response with all metrics
            std::string response = mstStrResult.get();
            response += "Weight: " + std::to_string(weightResult.get()) + "\n";
            response += "Longest distance: " + std::to_string(longestResult.get()) + "\n";
            response += "Shortest distance: " + std::to_string(shortestResult.get()) + "\n";
            response += "Average distance: " + std::to_string(avgResult.get()) + "\n";

            // Send response back to client
            callback(response);
        }).detach();
    }
    else if (command == "print_graph") {
        // Get string representation asynchronously
        std::thread([proxy, callback]() {
            Future<std::string> result = proxy->toString();
            callback(result.get());
        }).detach();
    }
    else {
        callback("Invalid command: " + input + "\n");
    }
}
    
    // Stop all proxies and cleanup
    void shutdown() {
        std::lock_guard<std::mutex> lock(proxies_mutex);
        proxies.clear();
    }
};

#endif // MSTPIPELINE_HPP