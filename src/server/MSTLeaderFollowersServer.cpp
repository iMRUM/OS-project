#include "../../include/server/MSTLeaderFollowersServer.hpp"
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

MSTLeaderFollowersServer::MSTLeaderFollowersServer(size_t numThreads)
    : numThreads_(numThreads) {
    // Create handle set and thread pool
    handleSet_ = std::make_unique<HandleSet>();
    threadPool_ = std::make_unique<ThreadPool>(numThreads_, handleSet_.get());
}

MSTLeaderFollowersServer::~MSTLeaderFollowersServer() {
    if (running) {
        stop();
    }
    
    // Clean up client handlers
    for (auto& pair : clientHandlers_) {
        delete pair.second;
    }
}

void MSTLeaderFollowersServer::start() {
    // Set up the server socket using the base class method
    MSTServer::setupSocket();
    
    if (listener < 0) {
        std::cerr << "Failed to set up listener socket" << std::endl;
        return;
    }
    
    // Set the socket to non-blocking mode (needed for Leader/Followers pattern)
    int flags = fcntl(listener, F_GETFL, 0);
    fcntl(listener, F_SETFL, flags | O_NONBLOCK);
    
    // Create a handle for the listener socket
    Handle listenerHandle(listener);
    
    // Create an accept handler for the listener socket
    acceptHandler_ = std::make_unique<AcceptHandler>(
        listenerHandle, 
        *handleSet_, 
        algoFactory_
    );
    
    // Register the accept handler with the handle set
    handleSet_->registerHandler(listenerHandle, EventType::READ, acceptHandler_.get());
    
    // Set running to true before starting the thread pool
    running = true;

    // Start the thread pool
    threadPool_->start();

    std::cout << "MST Leader/Followers server started on port " << PORT << std::endl;
    std::cout << "Using " << numThreads_ << " threads in the pool" << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    // Keep the main thread alive until the server is stopped
    while (running) {
        sleep(1);
    }
}


void MSTLeaderFollowersServer::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!running) {
        return;
    }
    
    std::cout << "Stopping MST server..." << std::endl;
    running = false;
    
    // Stop the thread pool
    if (threadPool_) {
        threadPool_->stop();
    }
    
    // Clean up client connections
    for (const auto& pair : clientHandlers_) {
        // Remove the handler from the handle set
        Handle clientHandle(pair.first);
        handleSet_->removeHandler(clientHandle, EventType::READ);
        
        // Close the socket
        close(pair.first);
    }
    clientHandlers_.clear();
    
    // Close the listener socket
    if (listener >= 0) {
        close(listener);
        listener = -1;
    }
    
    std::cout << "Server stopped" << std::endl;
}

void MSTLeaderFollowersServer::handleClient(ClientData* clientData) {
    // In the Leader/Followers implementation, client connection acceptance
    // is handled by the AcceptHandler. However, we still need to implement
    // this method to satisfy the base class interface.
    
    int socketFd = clientData->socket;
    char remoteIP[INET6_ADDRSTRLEN];
    
    // Log the connection
    std::cout << "Client connected on socket " << socketFd << " from "
              << inet_ntop(clientData->address.ss_family,
                         get_in_addr((struct sockaddr*)&clientData->address),
                         remoteIP, INET6_ADDRSTRLEN)
              << std::endl;
    
    // The AcceptHandler will take care of creating CommandHandler
    // and registering it with HandleSet
    
    // Clean up the client data
    delete clientData;
}

void MSTLeaderFollowersServer::processCommand(int socketFd, std::string& command) {
    // In the Leader/Followers implementation, command processing is handled
    // by the CommandHandler. However, we still need to implement this method
    // to satisfy the base class interface.
    
    std::cout << "Received command on socket " << socketFd << ": " << command << std::endl;
    std::cout << "This command will be handled by the CommandHandler" << std::endl;
}

void MSTLeaderFollowersServer::closeClient(int socketFd) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find the client handler
    auto it = clientHandlers_.find(socketFd);
    if (it != clientHandlers_.end()) {
        // Remove from our map
        clientHandlers_.erase(it);
    }
    
    // Remove the handler from the handle set
    Handle clientHandle(socketFd);
    handleSet_->removeHandler(clientHandle, EventType::READ);
    
    // Close the socket
    close(socketFd);
    
    std::cout << "Closed client connection on socket " << socketFd << std::endl;
}