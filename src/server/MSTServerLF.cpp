#include <functional>
#include <map>

#include "../../include/active_object/MSTServant.hpp"
#include "../../include/leader_followers/LFThreadPool.hpp"
#include "../../include/server/Server.hpp"
//==============================================================================
// Global variables
//==============================================================================
#define NUM_THREADS 4

void executeCommand(const std::string &processedLine, int clientfd, std::function<void(std::string)> sendCallback);

std::atomic<bool> running{false};
std::map<int, MSTServant *> client_servants;
ConcreteAlgoFactory algoFactory;
LFThreadPool *tp = new LFThreadPool();
pthread_mutex_t servants_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tp_mtx = PTHREAD_MUTEX_INITIALIZER;

void *worker_function(void *arg) {
    LFThreadPool *pool = static_cast<LFThreadPool *>(arg);
    pool->join();
    return nullptr;
}

//==============================================================================
// Server lifecycle
//==============================================================================
void init() {
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
    tp->addFd(listener, reactorFunc(&handleAcceptClient));
    running = true;
}

void stop() {
    running = false;
    pthread_mutex_lock(&servants_mtx);
    for (auto &pair: client_servants) {
        delete pair.second;
    }
    client_servants.clear();
    pthread_mutex_unlock(&servants_mtx);
    pthread_mutex_lock(&tp_mtx);
    delete tp;
    pthread_mutex_unlock(&tp_mtx);
}

void start() {
    init();
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], nullptr, worker_function, tp) != 0) {
            perror("Failed to create thread");
            return;
        }
    }
    while (running) {
        sleep(1);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }
}

//==============================================================================
// Client handling
//==============================================================================

void handleRequest(int clientfd) {
    pthread_mutex_lock(&servants_mtx);
    if (client_servants.find(clientfd) == client_servants.end()) {
        client_servants[clientfd] = new MSTServant(algoFactory);
    }
    pthread_mutex_unlock(&servants_mtx);
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
    pthread_mutex_lock(&servants_mtx);
    if (client_servants.find(clientfd) != client_servants.end()) {
        delete client_servants[clientfd];
        client_servants.erase(clientfd);
    }
    pthread_mutex_unlock(&servants_mtx);
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
            // Check if this is just a raw edge definition (when collecting edges)
            std::istringstream iss(line);
            int source, target, weight;
            if (iss >> source >> target >> weight) {
                processedLine = "add_edge " + std::to_string(source) + " " +
                                std::to_string(target) + " " + std::to_string(weight);
            } else {
                sendCallback("Invalid command: " + line + "\n");
                continue;
            }
        }
        executeCommand(processedLine, clientfd, sendCallback);
    }
}

void handleAcceptClient(int fd_listener) {
    int clientfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof(remoteaddr);

    if ((clientfd = accept(fd_listener, (struct sockaddr *) &remoteaddr, &addrlen)) < 0) {
        perror("accept");
        return;
    }

    std::string welcome = "Welcome to the MST Server. Type 'help' for commands.\n";
    send(clientfd, welcome.c_str(), welcome.length(), 0);
    // Create a new thread to handle this client
    pthread_mutex_lock(&tp_mtx);
    tp->addFd(clientfd, &handleRequest);
    pthread_mutex_unlock(&tp_mtx);
}

void executeCommand(const std::string &processedLine, int clientfd, std::function<void(std::string)> sendCallback) {
    std::istringstream iss(processedLine);
    std::string cmd;
    iss >> cmd;

    pthread_mutex_lock(&servants_mtx);
    auto it = client_servants.find(clientfd);
    if (it == client_servants.end()) {
        sendCallback("Error: Client session not found\n");
        pthread_mutex_unlock(&servants_mtx);
        return;
    }
    MSTServant *servant = it->second;
    pthread_mutex_unlock(&servants_mtx);

    if (cmd == "new_graph") {
        int vertices = 0;
        iss >> vertices;
        servant->initGraph_i(vertices);
        sendCallback("Created new graph with " + std::to_string(vertices) + " vertices\n");
    } else if (cmd == "add_edge") {
        int src, dest, weight;
        if (iss >> src >> dest >> weight) {
            servant->addEdge_i(src, dest, weight);
            sendCallback("Added edge: " + std::to_string(src) + " -> " +
                         std::to_string(dest) + " (weight: " + std::to_string(weight) + ")\n");
        }
    } else if (cmd == "print_graph") {
        std::string graphStr = servant->toString_i();
        sendCallback("Graph structure:\n" + graphStr);
    } else if (cmd == "mst_kruskal") {
        MST result = servant->getMST_i("kruskal");
        std::string response = "MST using Kruskal's algorithm:\n";
        response += "Total weight: " + std::to_string(result.getTotalWeight()) + "\n";
        sendCallback(response);
    } else if (cmd == "mst_prim") {
        MST result = servant->getMST_i("prim");
        std::string response = "MST using Prim's algorithm:\n";
        response += "Total weight: " + std::to_string(result.getTotalWeight()) + "\n";
        sendCallback(response);
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    start();

    return 0;
}
