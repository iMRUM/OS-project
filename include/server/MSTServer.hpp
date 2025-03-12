//Based on selectserver.c by Beej

#ifndef MSTSERVER_HPP
#define MSTSERVER_HPP
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
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <map>
#include <mutex>
#include <thread>
#include "../dsa/Graph.hpp"
#include "../dsa/MST.hpp"
#include "../dsa/ConcreteAlgoFactory.hpp"

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

// Structure to pass to client handler threads
struct ClientData {
    int socket;
    struct sockaddr_storage address;
};

class MSTServer {
private:
    int listener;
    bool running;
    Graph shared_graph;
    std::mutex calculator_mutex;
    std::vector<std::thread> client_threads;
    ConcreteAlgoFactory algo_factory;
    std::map<int, int> clientCommandState; // Maps socket_fd to command state
    std::map<int, int> clientPointsNeeded; // Maps socket_fd to points needed
    std::map<int, std::vector<std::string> > clientPendingLines; // Maps socket_fd to pending lines

    void stop();

    void handleClient(ClientData *client_data);

    void setupSocket();

    void processCommand(int socket_fd, std::string &command);

    bool processGraphEdges(int socket_fd, const std::vector<std::string> &edges);

    std::string calculateMST(const std::string &algorithm);

    std::string printGraph();

    void cleanupClient(int socket_fd);

    void resetGraph();

public:
    MSTServer(): listener(-1), running(false),shared_graph(0,0){
    }

    void start();
};
#endif //MSTSERVER_HPP
