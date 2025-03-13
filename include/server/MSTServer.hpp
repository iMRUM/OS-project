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
#include "../factory/ConcreteAlgoFactory.hpp"
#include "../active_object/MSTPipeline.hpp"
#include "../commands.hpp"
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
    std::vector<std::thread> client_threads;
    std::unique_ptr<MSTPipeline> mstPipeline;

    void stop();
    void handleClient(ClientData *client_data);
    void setupSocket();
    void processCommand(int socket_fd, std::string &command);

public:
    MSTServer(): listener(-1), running(false) {}
    void start();
};
#endif //MSTSERVER_HPP