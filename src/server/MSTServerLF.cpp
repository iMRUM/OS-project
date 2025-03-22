#include <map>
#include "../../include/leader_followers/LFThreadPool.hpp"
#include "../../include/server/Server.hpp"
#define NUM_THREADS 4

int isRunning = 0;
LFThreadPool threadPool;
pthread_mutex_t graph_mtx = PTHREAD_MUTEX_INITIALIZER;

void* worker_function(void* arg) {
    LFThreadPool* pool = static_cast<LFThreadPool*>(arg);
    pool->join(); // Thread joins the pool and follows leader-follower pattern
    return nullptr;
}
void init(){
    int listener;
    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    struct addrinfo hints, *ai, *p;

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(nullptr, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != nullptr; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == nullptr) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
    threadPool.addFd(listener, reactorFunc(handleAcceptClient));
}

int run(){}

void stop(){}

void start(){}

void handleRequest(int clientfd) {
   // int clientfd = *(int*)client_attr;
    char buf[256]; // buffer for client data
    int nbytes;
        if ((nbytes = recv(clientfd, buf, sizeof buf - 1, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
                // connection closed
                printf("Socket %d hung up\n", clientfd);
            } else {
                perror("recv");
            }
        buf[nbytes] = '\0';
        pthread_mutex_lock(&graph_mtx);
        handleCommand(clientfd, buf);
        pthread_mutex_unlock(&graph_mtx);
    }
}

void handleCommand(int clientfd, const std::string &command) {
    // Define a callback function to send responses to the client
    auto sendCallback = [clientfd](std::string response) {
        send(clientfd, response.c_str(), response.length(), 0);
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
            close(clientfd);
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
    }
}

void handleAcceptClient(void* listener_attr){
    int fd_listener = *(int*)listener_attr;
    std::cout << "Accepted connection THREAD, listening on socket " << fd_listener << std::endl;
    int newfd;
    addrlen = sizeof(remoteaddr);
        if ((newfd = accept(fd_listener, (struct sockaddr *) &remoteaddr, &addrlen)) < 0) {
            perror("accept");
        }
        threadPool.addFd(newfd, reactorFunc(handleRequest));
}
int main() {
    // Initialize the server (sets up the socket listener)
    init();

    // Create worker threads for the thread pool
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], nullptr, worker_function, &threadPool) != 0) {
            perror("Failed to create thread");
            return 1;
        }
        std::cout << "Created worker thread " << threads[i] << std::endl;
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}