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

    // Create the MSTPipeline
    mstPipeline = std::make_unique<MSTPipeline>();

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

    // Shutdown the pipeline
    if (mstPipeline) {
        mstPipeline->shutdown();
    }
}

void MSTServer::handleClient(ClientData *client_data) {
    int socket_fd = client_data->socket;
    char remoteIP[INET6_ADDRSTRLEN];

    // Print client connection information
    printf("Thread started for client on socket %d from %s\n",
           socket_fd,
           inet_ntop(client_data->address.ss_family,
                     get_in_addr((struct sockaddr *) &client_data->address),
                     remoteIP, INET6_ADDRSTRLEN));

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

        // Process the received data
        std::string data(buf);
        processCommand(socket_fd, data);
    }

    // Clean up and exit thread
    close(socket_fd);
    printf("Socket %d disconnected\n", socket_fd);

    // Clean up client's proxy in the pipeline
    mstPipeline->removeProxy(socket_fd);
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
    // Define a callback function to send responses to the client
    auto sendCallback = [socket_fd](std::string response) {
        send(socket_fd, response.c_str(), response.length(), 0);
    };

    // Process commands line by line
    std::istringstream stream(command);
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
            iss >> cmd;  // Extract "Newgraph"

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
        }
        else if (lowerLine == "printgraph") {
            processedLine = "print_graph";
        }
        else if (lowerLine.substr(0, 3) == "mst") {
            std::string algo;
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd >> algo;

            // Convert algorithm name to lowercase for case-insensitive comparison
            std::transform(algo.begin(), algo.end(), algo.begin(), ::tolower);

            if (algo == "kruskal") {
                processedLine = "mst_kruskal";
            }
            else if (algo == "prim") {
                processedLine = "mst_prim";
            }
            else {
                sendCallback("Invalid algorithm. Please use 'Kruskal' or 'Prim'.\n");
                continue;
            }
        }
        else if (lowerLine == "exit") {
            sendCallback("Goodbye!\n");
            close(socket_fd);
            return;
        }
        else if (lowerLine == "resetgraph") {
            // Reset by creating a new empty graph
            processedLine = "new_graph 0";
        }
        else if (lowerLine == "help") {
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
        }
        else if (lowerLine.substr(0, 7) == "addedge") {
            // Parse "AddEdge <source> <target> <weight>"
            std::istringstream iss(line);
            std::string cmd;
            int source, target, weight;
            iss >> cmd; // Extract "AddEdge"

            if (iss >> source >> target >> weight) {
                processedLine = "add_edge " + std::to_string(source) + " " +
                               std::to_string(target) + " " + std::to_string(weight);
            }
            else {
                sendCallback("Invalid edge format. Usage: AddEdge <source> <target> <weight>\n");
                continue;
            }
        }
        else {
            // Check if this is just a raw edge definition (when collecting edges)
            std::istringstream iss(line);
            int source, target, weight;
            if (iss >> source >> target >> weight) {
                // This is likely an edge being added
                processedLine = "add_edge " + std::to_string(source) + " " +
                               std::to_string(target) + " " + std::to_string(weight);
            }
            else {
                sendCallback("Invalid command: " + line + "\n");
                continue;
            }
        }

        std::cout << "Processing command: " << processedLine << std::endl;  // Debug output

        // Process the command through the pipeline
        mstPipeline->processCommand(processedLine, socket_fd, sendCallback);
    }
}
