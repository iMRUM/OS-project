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
int listener = -1; // Main server socket
std::mutex pipelineMutex; // Protect access to the pipeline
std::mutex clientsMutex; // Protect access to the client list
std::vector<int> activeClients; // Track active client sockets
std::atomic<bool> running{false}; // Server running state

//==============================================================================
// Client handling
//==============================================================================

void *request_worker_function(void *arg) {
    // Copy the client fd and free the memory
    int clientfd = *(int *) arg;
    delete (int *) arg;

    // Add client to active list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        activeClients.push_back(clientfd);
    }

    // Handle the client request
    handleRequest(clientfd);

    // Remove client from active list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        activeClients.erase(std::remove(activeClients.begin(), activeClients.end(), clientfd),
                            activeClients.end());
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
        // Receive data from client
        nbytes = recv(clientfd, buf, sizeof(buf) - 1, 0);

        if (nbytes <= 0) {
            // Connection closed or error
            break;
        }

        // Null-terminate the received data
        buf[nbytes] = '\0';

        // Process the received data
        std::string data(buf);
        handleCommand(clientfd, data);
    }

    std::cout << "Client on socket " << clientfd << " disconnected" << std::endl;
}

void handleCommand(int clientfd, const std::string &input_command) {
    auto sendCallback = [clientfd](std::string response) {
        send(clientfd, response.c_str(), response.length(), 0);
    };

    // Process commands line by line
    std::istringstream stream(input_command);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Convert to lowercase for case-insensitive comparison
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

        // Map client commands to our internal command format
        std::string processedLine;

        if (lowerLine.substr(0, 8) == "newgraph") {
            // Extract the number of vertices
            std::istringstream iss(line);
            std::string cmd;
            int n;
            iss >> cmd; // Extract "Newgraph"

            if (iss >> n && n >= 0) {
                processedLine = "new_graph " + std::to_string(n);

                // If there's also an edge count, include it
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

            // Convert algorithm name to lowercase for case-insensitive comparison
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

            // Remove client from active list
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                activeClients.erase(std::remove(activeClients.begin(), activeClients.end(), clientfd),
                                    activeClients.end());
            }

            // Notify pipeline of client disconnect
            {
                std::lock_guard<std::mutex> lock(pipelineMutex);
                if (pl) {
                    pl->removeProxy(clientfd);
                }
            }

            return;
        } else if (lowerLine == "resetgraph") {
            // Reset by creating a new empty graph
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
            // Parse "AddEdge <source> <target> <weight>"
            std::istringstream iss(line);
            std::string cmd;
            int source, target, weight;
            iss >> cmd; // Extract "AddEdge"

            if (iss >> source >> target >> weight) {
                processedLine = "add_edge " + std::to_string(source) + " " +
                                std::to_string(target) + " " + std::to_string(weight);
            } else {
                sendCallback("Invalid edge format. Usage: AddEdge <source> <target> <weight>\n");
                continue;
            }
        } else {
            // Check if this is just a raw edge definition (when collecting edges)
            std::istringstream iss(line);
            int source, target, weight;
            if (iss >> source >> target >> weight) {
                // This is likely an edge being added
                processedLine = "add_edge " + std::to_string(source) + " " +
                                std::to_string(target) + " " + std::to_string(weight);
            } else {
                sendCallback("Invalid command: " + line + "\n");
                continue;
            }
        }

        std::cout << "Processing command: " << processedLine << std::endl; // Debug output

        // Process the command through the pipeline - with mutex protection
        {
            std::lock_guard<std::mutex> lock(pipelineMutex);
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
    char remoteIP[INET6_ADDRSTRLEN];

    std::cout << "Server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    while (running) {
        addrlen = sizeof client_addr;
        int clientfd = accept(listener, (struct sockaddr *) &client_addr, &addrlen);

        if (clientfd == -1) {
            perror("accept");
            continue;
        }

        // Print connection info
        printf("Connection from %s\n",
               inet_ntop(client_addr.ss_family,
                         get_in_addr((struct sockaddr *) &client_addr),
                         remoteIP, INET6_ADDRSTRLEN));

        // Create a new thread to handle this client
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

    // Create pipeline (with mutex protection)
    {
        std::lock_guard<std::mutex> lock(pipelineMutex);
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
    std::cout << "Stopping server..." << std::endl;
    running = false;

    // Close listener socket
    if (listener >= 0) {
        close(listener);
        listener = -1;
    }

    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int fd: activeClients) {
            close(fd);
        }
        activeClients.clear();
    }

    // Shutdown and delete the pipeline
    {
        std::lock_guard<std::mutex> lock(pipelineMutex);
        if (pl) {
            pl->shutdown();
            delete pl;
            pl = nullptr;
        }
    }

    std::cout << "Server stopped" << std::endl;
}

void start() {
    // Initialize the server
    init();

    // Start client accept thread
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, nullptr, [](void *) -> void *{
        handleAcceptClient();
        return nullptr;
    }, nullptr) != 0) {
        perror("Failed to create accept thread");
        stop();
        exit(1);
    }

    // Wait for the accept thread to finish (will only happen if running becomes false)
    pthread_join(accept_thread, nullptr);
}

//==============================================================================
// Main function
//==============================================================================

int main(int argc, char *argv[]) {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Start the server
    start();

    return 0;
}
