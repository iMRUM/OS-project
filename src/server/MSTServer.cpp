#include "../../include/server/MSTServer.hpp"

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void MSTServer::start() {
    setupSocket();
    struct sockaddr_storage client_addr; // client address
    socklen_t addrlen;
    std::cout << "Threaded MST server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    // Main accept() loop
    while (1) {
        addrlen = sizeof client_addr;
        int new_fd = accept(listener, (struct sockaddr *) &client_addr, &addrlen);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Create client data structure to pass to the thread
        ClientData *client_data = new ClientData;
        client_data->socket = new_fd;
        client_data->address = client_addr;

        // Create a new thread to handle this client
        std::thread client_thread(&MSTServer::handleClient, this, client_data);
        client_thread.detach(); // Detach the thread so it can run independently
    }
}

void MSTServer::stop() {
    std::cout << "Stopping MST server..." << std::endl;
    running = false;
    if (listener >= 0) {
        close(listener);
        listener = -1;
    }
}

void MSTServer::handleClient(ClientData *client_data) {
    int socket_fd = client_data->socket;
    char remoteIP[INET6_ADDRSTRLEN];

    // Print client connection information
    printf("Thread started for client on socket %d from %s\n total of %lu threads",
           socket_fd,
           inet_ntop(client_data->address.ss_family,
                     get_in_addr((struct sockaddr *) &client_data->address),
                     remoteIP, INET6_ADDRSTRLEN), client_threads.size());

    // Free the client_data structure as we've extracted what we need
    delete client_data;

    // Send welcome message to the client
    std::string welcome = "Welcome to the MST Server.\nType 'help' for available commands.\n";
    send(socket_fd, welcome.c_str(), welcome.length(), 0);

    // Buffer for receiving data
    char buf[256];
    int nbytes;

    // Main client handling loop
    while (running) {
        // Receive data from client
        nbytes = recv(socket_fd, buf, sizeof(buf) - 1, 0);

        if (nbytes <= 0) {
            // Connection closed or error
            break;
        }

        // Null-terminate the received data
        buf[nbytes] = '\0';

        // Process the received data line by line
        std::string data(buf);
        processCommand(socket_fd, data);
    }

    // Clean up and exit thread
    close(socket_fd);
    printf("Socket %d disconnected\n", socket_fd);
}

void MSTServer::setupSocket() {
    int yes = 1; // for setsockopt() SO_REUSEADDR
    int rv;
    struct addrinfo hints, *ai, *p;

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

void MSTServer::processCommand(int socket_fd, std::string &command) {
    // Client state variables - using class member variables

    // Initialize state for new clients
    if (clientCommandState.find(socket_fd) == clientCommandState.end()) {
        clientCommandState[socket_fd] = 0; // 0 = normal, 1 = expecting edges
        clientPointsNeeded[socket_fd] = 0;
        clientPendingLines[socket_fd] = std::vector<std::string>();
    }

    // Get the client's state
    int &commandState = clientCommandState[socket_fd];
    int &pointsNeeded = clientPointsNeeded[socket_fd];
    std::vector<std::string> &pendingLines = clientPendingLines[socket_fd];

    std::istringstream stream(command);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove carriage return if present (handles Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (commandState == 0) {
            // Normal command processing mode
            if (line.substr(0, 8) == "Newgraph") {
                // Extract the number of edges
                std::istringstream iss(line);
                std::string cmd;
                int n;
                iss >> cmd >> n;

                // Check if there's already a graph in the server
                bool graphExists = false; {
                    std::lock_guard<std::mutex> lock(calculator_mutex);
                    graphExists = !shared_graph.isEmpty();
                }

                if (graphExists) {
                    // Graph already exists, inform the client
                    std::string response = "A graph already exists in the server. Use PrintGraph to view it.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                } else if (n > 0) {
                    commandState = 1; // Switch to edge collection mode
                    pointsNeeded = n; // Set the number of edges to collect
                    pendingLines.clear(); // Clear any existing edges

                    // Send acknowledgment
                    std::string response = "Please enter " + std::to_string(n) +
                                           " edges in format 'source target weight', one per line:\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                } else {
                    std::string response = "Invalid number of edges.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                }
            } else if (line == "help") {
                // Handle help command
                std::string helpText = "Available commands:\n"
                        "  Newgraph n - Create a new graph with n edges\n"
                        "  PrintGraph - Display the current graph structure\n"
                        "  MST algo - Calculate MST using specified algorithm (Kruskal/Prim)\n"
                        "  ResetGraph - Reset the current graph\n"
                        "  help - Display this help text\n"
                        "  exit - Close connection\n";
                send(socket_fd, helpText.c_str(), helpText.length(), 0);
            } else if (line == "PrintGraph") {
                // Print the current graph
                bool graphExists = false; {
                    std::lock_guard<std::mutex> lock(calculator_mutex);
                    graphExists = !shared_graph.isEmpty();
                }

                if (!graphExists) {
                    std::string response = "No graph available. Please create a graph first using 'Newgraph'.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                } else {
                    std::string graphInfo = printGraph();
                    send(socket_fd, graphInfo.c_str(), graphInfo.length(), 0);
                }
            } else if (line.substr(0, 3) == "MST") {
                // Extract the algorithm name
                std::istringstream iss(line);
                std::string cmd, algo;
                iss >> cmd >> algo;

                bool graphExists = false; {
                    std::lock_guard<std::mutex> lock(calculator_mutex);
                    graphExists = !shared_graph.isEmpty();
                }

                if (!graphExists) {
                    std::string response = "No graph available. Please create a graph first using 'Newgraph'.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                } else if (algo == "Kruskal" || algo == "Prim") {
                    // Calculate MST using the specified algorithm
                    std::string result = calculateMST(algo);
                    send(socket_fd, result.c_str(), result.length(), 0);
                } else {
                    std::string response = "Invalid algorithm. Please use 'Kruskal' or 'Prim'.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                }
            } else if (line == "exit") {
                // Handle exit command
                std::string response = "Goodbye!\n";
                send(socket_fd, response.c_str(), response.length(), 0);

                // Clean up resources for this client
                cleanupClient(socket_fd);
                return;
            } else if (line == "ResetGraph") {
                // Reset the shared graph
                resetGraph();
                std::string response = "Graph has been reset. You can create a new one with 'Newgraph'.\n";
                send(socket_fd, response.c_str(), response.length(), 0);
            } else {
                // Process other commands (to be implemented later)
                std::string response = "Unknown command: " + line + "\n";
                send(socket_fd, response.c_str(), response.length(), 0);
            }
        } else if (commandState == 1) {
            // Collecting edges for Newgraph command
            pendingLines.push_back(line);

            if (pendingLines.size() >= pointsNeeded) {
                // We have all edges, process the Newgraph command
                {
                    // Process edges and create the graph
                    // Note: processGraphEdges acquires graph_mutex internally
                    bool success = processGraphEdges(socket_fd, pendingLines);

                    if (success) {
                        // Send confirmation
                        std::string response = "Graph created with " +
                                               std::to_string(pointsNeeded) +
                                               " edges.\n";
                        send(socket_fd, response.c_str(), response.length(), 0);
                    } else {
                        std::string response = "Error creating graph. Please check edge format.\n";
                        send(socket_fd, response.c_str(), response.length(), 0);
                    }
                }

                // Reset client state
                commandState = 0;
                pointsNeeded = 0;
                pendingLines.clear();
            } else {
                // Acknowledge edge receipt
                int remaining = pointsNeeded - pendingLines.size();
                std::string response = "Edge received. " +
                                       std::to_string(remaining) +
                                       " more edge(s) needed.\n";
                send(socket_fd, response.c_str(), response.length(), 0);
            }
        }
    }
}

bool MSTServer::processGraphEdges(int socket_fd, const std::vector<std::string> &edges) {
    // Find the maximum vertex id to determine graph size
    int maxVertex = -1;
    std::vector<std::tuple<int, int, int> > parsedEdges;

    for (const auto &edgeLine: edges) {
        std::istringstream iss(edgeLine);
        int source, target, weight;

        if (!(iss >> source >> target >> weight)) {
            return false; // Parsing failed
        }

        // Check for valid vertex ids (must be non-negative)
        if (source < 0 || target < 0) {
            return false;
        }

        maxVertex = std::max(maxVertex, std::max(source, target));
        parsedEdges.emplace_back(source, target, weight);
    }

    // Create a new graph with proper size
    int numVertices = maxVertex + 1; // +1 because vertices are 0-indexed
    Graph newGraph(numVertices, parsedEdges.size());

    // Add all edges to the graph
    for (const auto &edge: parsedEdges) {
        int source = std::get<0>(edge);
        int target = std::get<1>(edge);
        int weight = std::get<2>(edge);

        newGraph.addEdge(source, target, weight);
    }

    // Store the graph as the shared graph
    std::lock_guard<std::mutex> lock(calculator_mutex);
    shared_graph = newGraph;
    return true;
}

std::string MSTServer::calculateMST(const std::string &algorithm) {
    std::stringstream result;

    std::lock_guard<std::mutex> lock(calculator_mutex);

    if (shared_graph.isEmpty()) {
        return "Error: Graph is empty. Please create a graph first.\n";
    }

    result << "Calculating MST using " << algorithm << " algorithm\n";
    result << "Graph has " << shared_graph.getVertices() << " vertices.\n";

    MST *mst_res = nullptr;
    AbstractProductAlgo *algo = nullptr;

    try {
        if (algorithm == "Prim") {
            std::cout << "Creating Prim algorithm object" << std::endl;
            algo = algo_factory.createProduct(0); // Prim algorithm
        } else if (algorithm == "Kruskal") {
            std::cout << "Creating Kruskal algorithm object" << std::endl;
            algo = algo_factory.createProduct(1); // Kruskal algorithm
        }

        if (!algo) {
            return "Error: Failed to create algorithm object.\n";
        }

        // Execute the algorithm on our graph
        std::cout << "Executing algorithm" << std::endl;
        mst_res = algo->execute(shared_graph);
        std::cout << "Algorithm execution complete" << std::endl;

        if (!mst_res) {
            std::cout << "Algorithm execution failed" << std::endl;
            delete algo; // Clean up algorithm before returning
            return "Error: Algorithm execution failed.\n";
        }

        // Get the MST information
        std::cout << "Getting MST results" << std::endl;
        result << "MST Edges (source -> target : weight):\n";
        auto edges = mst_res->getEdges();
        for (const auto &edge: edges) {
            int source = std::get<0>(edge);
            int target = std::get<1>(edge);
            int weight = std::get<2>(edge);

            result << "  " << source << " -> " << target << " : " << weight << "\n";
        }
        // Clean up - delete in reverse order of creation
        std::cout << "Cleaning up resources" << std::endl;
        delete mst_res;
        delete algo; // Move this after all usage of mst_res is done

        std::cout << "Cleanup complete" << std::endl;
    } catch (const std::exception &e) {
        // Clean up in case of exception
        std::cout << "Exception caught: " << e.what() << std::endl;
        delete mst_res;
        delete algo;
        return "Error calculating MST: " + std::string(e.what()) + "\n";
    } catch (...) {
        // Clean up in case of unknown exception
        std::cout << "Unknown exception caught" << std::endl;
        delete mst_res;
        delete algo;
        return "Unknown error calculating MST.\n";
    }

    return result.str();
}
std::string MSTServer::printGraph() {
    std::stringstream result;

    std::lock_guard<std::mutex> lock(calculator_mutex);

    int vertices = shared_graph.getVertices();
    auto graphPair = shared_graph.getAsPair();
    auto edges = graphPair.first;

    result << "Graph Information:\n";
    result << "Number of vertices: " << vertices << "\n";
    result << "Number of edges: " << edges.size() << "\n\n";

    result << "Edges (source -> target : weight):\n";
    for (const auto &edge: edges) {
        int source = std::get<0>(edge);
        int target = std::get<1>(edge);
        int weight = std::get<2>(edge);

        result << "  " << source << " -> " << target << " : " << weight << "\n";
    }

    result << "\n";
    return result.str();
}

void MSTServer::cleanupClient(int socket_fd) {
    // Clean up all data structures related to this client
    std::lock_guard<std::mutex> lock(calculator_mutex);

    // Note: We don't erase the shared graph here as it's meant to be preserved
    clientCommandState.erase(socket_fd);
    clientPointsNeeded.erase(socket_fd);
    clientPendingLines.erase(socket_fd);

    // Close the socket if not already closed
    close(socket_fd);
}

void MSTServer::resetGraph() {
    std::lock_guard<std::mutex> lock(calculator_mutex);
    shared_graph = Graph();
}
