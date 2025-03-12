//Based on selectserver.c by Beej

#ifndef MSTSERVER_HPP
#define MSTSERVER_HPP
#define PORT 9034

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

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class MSTServer {
    int listener;
    bool running;
    MSTServer(): listener(-1), running(false) {}
    void start();
    void stop();
    void handleClient(int clientSocket);
    void setupSocket();
};
#endif //MSTSERVER_HPP
