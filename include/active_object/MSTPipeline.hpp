#ifndef MSTPIPELINE_HPP
#define MSTPIPELINE_HPP

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include "MSTProxy.hpp"
#include "../commands.hpp"
#include <iostream>

class MSTPipeline {
private:
    std::map<int, std::unique_ptr<MSTProxy> > proxies;
    std::mutex proxies_mutex;
    ConcreteAlgoFactory algoFactory;

public:
    MSTPipeline() = default;

    ~MSTPipeline() = default;

    // Get or create proxy for a client
    MSTProxy *getProxy(int client_fd);

    // Remove a client's proxy when disconnected
    void removeProxy(int client_fd);

    // Process a command from a client
    void processCommand(const std::string &input, int client_fd, function<void(std::string)> callback);

    // Stop all proxies and cleanup
    void shutdown();
};

#endif // MSTPIPELINE_HPP
