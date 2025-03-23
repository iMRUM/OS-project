#ifndef SERVER_HPP
#define SERVER_HPP
#define PORT "9034"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <csignal>
#include "../dsa/Graph.hpp"
#include "../dsa/MST.hpp"
#include "../commands.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"
struct sockaddr_storage remoteaddr; // client address
socklen_t addrlen;


char remoteIP[INET6_ADDRSTRLEN];

void *get_in_addr(struct sockaddr *sa);
void signalHandler(int signum);

void handleRequest(int clientfd);

void handleCommand(int clientfd, const std::string &input_command);

void handleAcceptClient(int fd_listener);

void init();

void stop();

void start();

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    std::cout << "Shutting down server...\n";

    // Call the stop function to clean up resources
    stop();

    exit(signum);
}
#endif //SERVER_HPP
