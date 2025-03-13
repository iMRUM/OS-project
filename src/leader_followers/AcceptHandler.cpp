#include "../../include/leader_followers/AcceptHandler.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

AcceptHandler::AcceptHandler(const Handle& handle, HandleSet& handleSet, ConcreteAlgoFactory& factory)
    : handle_(handle),
      handleSet_(handleSet),
      factory_(factory) {
}

int AcceptHandler::handleEvent(const Handle& handle, int eventType) {
    // Verify that this is our handle
    if (handle.get() != handle_.get()) {
        std::cerr << "Error: Handle mismatch in AcceptHandler" << std::endl;
        return -1;
    }
    
    // Accept a new client connection
    return acceptClient();
}

Handle AcceptHandler::getHandle() const {
    return handle_;
}

int AcceptHandler::acceptClient() {
    struct sockaddr_storage clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    // Accept the new connection
    int clientFd = accept(handle_.get(), (struct sockaddr*)&clientAddr, &addrLen);
    
    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("accept");
        }
        return -1;
    }
    
    // Set the client socket to non-blocking mode
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
    
    // Get client info for logging
    char clientIP[INET6_ADDRSTRLEN];
    int clientPort = 0;
    
    if (clientAddr.ss_family == AF_INET) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)&clientAddr;
        inet_ntop(AF_INET, &ipv4->sin_addr, clientIP, INET6_ADDRSTRLEN);
        clientPort = ntohs(ipv4->sin_port);
    } else {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)&clientAddr;
        inet_ntop(AF_INET6, &ipv6->sin6_addr, clientIP, INET6_ADDRSTRLEN);
        clientPort = ntohs(ipv6->sin6_port);
    }
    
    std::cout << "New client connection from " << clientIP << ":" << clientPort 
              << " on socket " << clientFd << std::endl;
    
    // Create a handle for the client socket
    Handle clientHandle(clientFd);
    
    // Create a command handler for this client
    CommandHandler* handler = new CommandHandler(clientHandle, factory_);
    
    // Register the handler with the handle set
    if (!handleSet_.registerHandler(clientHandle, EventType::READ, handler)) {
        std::cerr << "Error: Failed to register command handler for client" << std::endl;
        delete handler;
        close(clientFd);
        return -1;
    }
    
    return 0;
}

void AcceptHandler::closeClient(int socketFd) {
    // Create a handle for the client socket
    Handle clientHandle(socketFd);
    
    // Remove the handler from the handle set
    handleSet_.removeHandler(clientHandle, EventType::READ);
    
    // Close the socket
    close(socketFd);
    
    std::cout << "Closed client connection on socket " << socketFd << std::endl;
}