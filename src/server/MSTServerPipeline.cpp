#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <atomic>
#include "../../include/server/Server.hpp"
#include "../../include/active_object/MSTPipeline.hpp"
// Forward declaration
class MSTPipeline;
//==============================================================================
// Global variables
//==============================================================================

MSTPipeline *pl = nullptr;
int listener = -1;
std::mutex pipeline_mtx;
std::mutex clients_mtx;
pthread_mutex_t listener_mtx = PTHREAD_MUTEX_INITIALIZER;
std::vector<int> active_clients;
std::atomic<bool> running{false};

//==============================================================================
// Client handling
//==============================================================================

void *request_worker_function(void *arg) {
    int clientfd = *(int *) arg;
    delete (int *) arg;

    {
        std::lock_guard<std::mutex> lock(clients_mtx);
        active_clients.push_back(clientfd);
    }

    handleRequest(clientfd);

    {
        std::lock_guard<std::mutex> lock(clients_mtx);
        active_clients.erase(std::remove(active_clients.begin(), active_clients.end(), clientfd),
                             active_clients.end());
    }

    close(clientfd);
    return nullptr;
}

void handleRequest(int clientfd) {
    std::string welcome = "Welcome to the MST Server.\nType 'help' for available commands.\n";
    send(clientfd, welcome.c_str(), welcome.length(), 0);

    char buf[256];
    int nbytes;

    while (running) {
        nbytes = recv(clientfd, buf, sizeof(buf) - 1, 0);
        if (nbytes <= 0) {
            break;
        }
        buf[nbytes] = '\0';
        std::string data(buf);
        handleCommand(clientfd, data);
    }
}

void handleCommand(int clientfd, const std::string &input_command) {
    auto sendCallback = [clientfd](std::string response) {
        send(clientfd, response.c_str(), response.length(), 0);
    };

    thread_local std::stringstream stream;
    stream.clear();
    stream.str(input_command);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
        std::string processedLine;

        if (lowerLine.substr(0, 8) == "newgraph") {
            std::istringstream iss(line);
            std::string cmd;
            int n;
            iss >> cmd;
            if (iss >> n && n >= 0) {
                processedLine = "new_graph " + std::to_string(n);
                int m;
                if (iss >> m && m >= 0) {
                    processedLine += " " + std::to_string(m);
                }
            } else {
                sendCallback("Invalid number of vertices. Usage: Newgraph <vertices> [<edges>]\n");
                continue;
            }
        } else if (lowerLine == "printgraph") {
            processedLine = "print_graph";
        } else if (lowerLine.substr(0, 3) == "mst") {
            std::string algo;
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd >> algo;
std::transform(algo.begin(), algo.end(), algo.begin(), ::tolower);

            if (algo == "kruskal") {
                processedLine = "mst_kruskal";
            } else if (algo == "prim") {
                processedLine = "mst_prim";
            } else {
                sendCallback("Invalid algorithm. Please use 'Kruskal' or 'Prim'.\n");
                continue;
            }
        } else if (lowerLine == "exit") {
            sendCallback("Goodbye!\n");
            close(clientfd);
            {
                std::lock_guard<std::mutex> lock(clients_mtx);
                active_clients.erase(std::remove(active_clients.begin(), active_clients.end(), clientfd),
                                     active_clients.end());
            }
            {
                std::lock_guard<std::mutex> lock(pipeline_mtx);
                if (pl) {
                    pl->removeProxy(clientfd);
                }
            }

            return;
        } else if (lowerLine == "resetgraph") {
            processedLine = "new_graph 0";
        } else if (lowerLine == "help") {
            std::string helpText = "Available commands:\n"
                    "  Newgraph <vertices> [<edges>] - Create a new graph with vertices and optional edges count\n"
                    "  AddEdge <source> <target> <weight> - Add an edge to the graph\n"
                    "  PrintGraph - Display the current graph structure\n"
                    "  MST Kruskal - Calculate MST using Kruskal's algorithm\n"
                    "  MST Prim - Calculate MST using Prim's algorithm\n"
                    "  ResetGraph - Reset the current graph\n"
                    "  help - Display this help text\n"
                    "  exit - Close connection\n";
            sendCallback(helpText);
            continue;
        } else if (lowerLine.substr(0, 7) == "addedge") {
            std::istringstream iss(line);
            std::string cmd;
            int source, target, weight;
            iss >> cmd;

            if (iss >> source >> target >> weight) {
                processedLine = "add_edge " + std::to_string(source) + " " +
                                std::to_string(target) + " " + std::to_string(weight);
            } else {
                sendCallback("Invalid edge format. Usage: AddEdge <source> <target> <weight>\n");
                continue;
            }
        } else {
std::istringstream iss(line);
            int source, target, weight;
            if (iss >> source >> target >> weight) {
                processedLine = "add_edge " + std::to_string(source) + " " +
                                std::to_string(target) + " " + std::to_string(weight);
            } else {
                sendCallback("Invalid command: " + line + "\n");
                continue;
            }
        } {
            std::lock_guard<std::mutex> lock(pipeline_mtx);
            if (pl) {
                pl->processCommand(processedLine, clientfd, sendCallback);
            } else {
                sendCallback("Server error: Pipeline not initialized\n");
            }
        }
    }
}

void handleAcceptClient() {
    struct sockaddr_storage client_addr;
    socklen_t addrlen;
    int clientfd = -1;
    char remoteIP[INET6_ADDRSTRLEN];
    while (running) {
        pthread_mutex_lock(&listener_mtx);
        if (listener >= 0) {
            addrlen = sizeof client_addr;
            clientfd = accept(listener, (struct sockaddr *) &client_addr, &addrlen);
        }
        pthread_mutex_unlock(&listener_mtx);
        if (clientfd == -1) {
            if (!running) break;
            perror("accept");
            continue;
        }
        pthread_t client_thread;
        int *client_fd_ptr = new int(clientfd); // Allocate new memory for the fd
        pthread_create(&client_thread, nullptr, request_worker_function, client_fd_ptr);
        pthread_detach(client_thread);
    }
}

//==============================================================================
// Server lifecycle
//==============================================================================

void init() {
    int yes = 1; // for setsockopt() SO_REUSEADDR
    int rv;
    struct addrinfo hints, *ai, *p;

    {
        std::lock_guard<std::mutex> lock(pipeline_mtx);
        pl = new MSTPipeline();
    }

    // Set up the address info structure
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(nullptr, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Loop through all the results and bind to the first one we can
    for (p = ai; p != nullptr; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Avoid "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break; // Successfully bound
    }

    // If we got here and p is NULL, it means we couldn't bind
    if (p == nullptr) {
        fprintf(stderr, "server: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // All done with this structure

    // Start listening
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    running = true;
}

void stop() {
    running = false;
    pthread_mutex_lock(&listener_mtx);
    if (listener >= 0) {
        close(listener);
        listener = -1;
    }
    pthread_mutex_unlock(&listener_mtx);
    {
        std::lock_guard<std::mutex> lock(clients_mtx);
        for (int fd: active_clients) {
            close(fd);
        }
        active_clients.clear();
    }

    {
        std::lock_guard<std::mutex> lock(pipeline_mtx);
        if (pl) {
            pl->shutdown();
            delete pl;
            pl = nullptr;
        }
    }
    pthread_mutex_destroy(&listener_mtx);
}

void start() {
    init();
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, nullptr, [](void *) -> void *{
        handleAcceptClient();
        return nullptr;
    }, nullptr) != 0) {
        perror("Failed to create accept thread");
        stop();
        exit(1);
    }
pthread_join(accept_thread, nullptr);
}

//==============================================================================
// Main function
//==============================================================================

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    start();

    return 0;
}
