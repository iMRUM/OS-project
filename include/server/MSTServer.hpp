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
#include <cctype>
#include <cmath>
#include <string>
#include <sstream>
#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include "../dsa/Graph.hpp"
#include "../dsa/MST.hpp"
#include "../commands.hpp"

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

// Structure to pass to client handler threads
struct ClientData {
    int socket;
    struct sockaddr_storage address;
};

class MSTServer {
protected:
    int listener;
    bool running;



    virtual void handleClient(ClientData *client_data) = 0;

    virtual void setupSocket();

    virtual void processCommand(int socket_fd, std::string &command) = 0;

public:
    MSTServer(): listener(-1), running(false) {
    }

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual ~MSTServer() = default;
};
#endif //MSTSERVER_HPP
