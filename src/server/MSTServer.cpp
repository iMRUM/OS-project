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
            if (nbytes == 0) {
                printf("Socket %d hung up\n", socket_fd);
            } else {
                perror("recv");
            }
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
    printf("Thread ended for client on socket %d, now there are %lu threads\n", socket_fd, client_threads.size());
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

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Loop through all the results and bind to the first one we can
    for (p = ai; p != NULL; p = p->ai_next) {
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
    if (p == NULL) {
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
    // Client state variables
    std::vector<std::string> pendingLines;
    int commandState = 0; // 0 = normal, 1 = expecting points for Newgraph
    int pointsNeeded = 0; // For Newgraph command
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
                // Extract the number of points
                std::istringstream iss(line);
                std::string cmd;
                int n;
                iss >> cmd >> n;

                if (n > 0) {
                    commandState = 1; // Switch to point collection mode
                    pointsNeeded = n; // Set the number of points to collect
                    pendingLines.clear(); // Clear any existing points

                    // Send acknowledgment
                    std::string response = "Please enter " + std::to_string(n) + " points, one per line:\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                } else {
                    std::string response = "Invalid number of points.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                }
            } else {
                // Process regular command
                std::vector<std::string> empty;
                std::string result;

                // Lock the calculator while processing the command
                {
                    std::lock_guard<std::mutex> lock(calculator_mutex);
                    //result = calculator.processCommand(line, empty);
                }

                result += "\n"; // Add newline for client display
                send(socket_fd, result.c_str(), result.length(), 0);

                // Check if this was an exit command
                if (result == "exit\n") {
                    printf("Client %d requested exit\n", socket_fd);
                    close(socket_fd);
                    return;
                }
            }
        } else if (commandState == 1) {
            // Collecting points for Newgraph command
            pendingLines.push_back(line);

            if (pendingLines.size() >= pointsNeeded) {
                // We have all points, process the Newgraph command
                {
                    std::lock_guard<std::mutex> lock(calculator_mutex);
                    //calculator.commandNewGraph(pointsNeeded, pendingLines);
                }

                // Send confirmation
                std::string response = "Graph created with " +
                                       std::to_string(pointsNeeded) +
                                       " points.\n";
                send(socket_fd, response.c_str(), response.length(), 0);

                // Reset client state
                commandState = 0;
                pointsNeeded = 0;
                pendingLines.clear();
            } else {
                // Acknowledge point receipt
                int remaining = pointsNeeded - pendingLines.size();
                std::string response = "Point received. " +
                                       std::to_string(remaining) +
                                       " more point(s) needed.\n";
                send(socket_fd, response.c_str(), response.length(), 0);
            }
        }
    }
}
