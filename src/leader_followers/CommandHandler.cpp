#include "../../include/leader_followers/CommandHandler.hpp"
#include "../../include/commands.hpp"
#include <unistd.h>
#include <cstring>
#include <algorithm>

CommandHandler::CommandHandler(const Handle& handle, ConcreteAlgoFactory& factory)
    : handle_(handle),
      algoFactory_(factory) {
    
    // Initialize buffer
    memset(buffer_, 0, sizeof(buffer_));
    
    // Initialize command handlers
    initCommandHandlers();
    
    // Send welcome message
    std::string welcome = "Welcome to the MST Server.\nType 'help' for available commands.\n";
    sendResponse(welcome);
}

int CommandHandler::handleEvent(const Handle& handle, int eventType) {
    // Check that this is our handle
    if (handle.get() != handle_.get()) {
        std::cerr << "Error: Handle mismatch in CommandHandler" << std::endl;
        return -1;
    }
    
    // Read command from socket
    ssize_t bytesRead = read(handle.get(), buffer_, sizeof(buffer_) - 1);
    
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            // Connection closed
            std::cout << "Client disconnected" << std::endl;
        } else {
            // Error
            std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
        }
        return -1;
    }
    
    // Null-terminate the buffer
    buffer_[bytesRead] = '\0';
    
    // Process the command
    std::string command(buffer_);
    // Remove trailing \r\n
    if (!command.empty() && command.back() == '\n') {
        command.pop_back();
    }
    if (!command.empty() && command.back() == '\r') {
        command.pop_back();
    }
    return processCommand(command);
}

Handle CommandHandler::getHandle() const {
    return handle_;
}

int CommandHandler::processCommand(const std::string& command) {
    std::vector<std::string> args;
    std::string cmdType = parseCommand(command, args);
    
    // Convert to lowercase for case-insensitive comparison
    std::string lowerCmd = cmdType;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    // Handle client command formats (similar to MSTPipelineServer)
    if (lowerCmd == "newgraph" || lowerCmd == "new_graph") {
        if (args.empty()) {
            sendResponse("Error: Newgraph requires at least one argument (number of vertices)\n");
            return 0;
        }

        try {
            int numVertices = std::stoi(args[0]);

            // Create a new graph
            graph_ = std::make_unique<Graph>(numVertices);

            // Reset MST
            mst_.reset();

            std::string response = "Created new graph with " + std::to_string(numVertices) + " vertices\n";
            sendResponse(response);

            // Handle optional edge count
            if (args.size() > 1) {
                int numEdges = std::stoi(args[1]);
                if (numEdges > 0) {
                    sendResponse("Ready to receive " + std::to_string(numEdges) + " edges\n");
                }
            }
        }
        catch (const std::exception& e) {
            sendResponse("Error parsing arguments: " + std::string(e.what()) + "\n");
        }

        return 0;
    }
    else if (lowerCmd == "addedge" || lowerCmd == "add_edge") {
        if (!graph_) {
            sendResponse("Error: No graph initialized. Use 'newgraph' command first.\n");
            return 0;
        }

        if (args.size() < 3) {
            sendResponse("Error: AddEdge requires 3 arguments (source, target, weight)\n");
            return 0;
        }

        try {
            int source = std::stoi(args[0]);
            int target = std::stoi(args[1]);
            int weight = std::stoi(args[2]);

            // Add edge to the graph
            graph_->addEdge(source, target, weight);

            // Reset MST since graph changed
            mst_.reset();

            std::string response = "Added edge from " + std::to_string(source) +
                                 " to " + std::to_string(target) +
                                 " with weight " + std::to_string(weight) + "\n";
            sendResponse(response);
        }
        catch (const std::exception& e) {
            sendResponse("Error parsing arguments: " + std::string(e.what()) + "\n");
        }

        return 0;
    }
    else if (lowerCmd == "mst" || lowerCmd == "mst_kruskal" || lowerCmd == "mst_prim") {
        if (!graph_) {
            sendResponse("Error: No graph initialized. Use 'newgraph' command first.\n");
            return 0;
        }

        std::string algo;
        if (lowerCmd == "mst") {
            if (args.empty()) {
                sendResponse("Error: MST command requires algorithm name (Kruskal or Prim)\n");
                return 0;
            }

            algo = args[0];
            std::transform(algo.begin(), algo.end(), algo.begin(), ::tolower);
        } else if (lowerCmd == "mst_kruskal") {
            algo = "kruskal";
        } else { // mst_prim
            algo = "prim";
        }

        if (algo != "kruskal" && algo != "prim") {
            sendResponse("Error: Invalid MST algorithm. Use 'Kruskal' or 'Prim'\n");
            return 0;
        }

        try {
            // Create the appropriate algorithm using factory
            AbstractProductAlgo* algorithm = algoFactory_.createProduct(algo);

            if (!algorithm) {
                sendResponse("Error: Failed to create " + algo + " algorithm\n");
                return 0;
            }

            // Execute the algorithm to create MST
            mst_.reset(algorithm->execute(*graph_));

            // Clean up algorithm
            delete algorithm;

            // Check if MST was created successfully
            if (!mst_) {
                sendResponse("Error: Failed to compute MST\n");
                return 0;
            }

            // Send MST details
            std::string response = "MST using " + algo + "'s algorithm:\n";
            response += mst_->toString();
            response += mst_->getTotalWeightAsString() + "\n";
            response += mst_->getAverageDistanceAsString() + "\n";
            response += mst_->getLongestDistanceAsString() + "\n";

            sendResponse(response);
        }
        catch (const std::exception& e) {
            sendResponse("Error computing MST: " + std::string(e.what()) + "\n");
        }

        return 0;
    }
    else if (lowerCmd == "printgraph" || lowerCmd == "print_graph") {
        if (!graph_) {
            sendResponse("Error: No graph initialized. Use 'newgraph' command first.\n");
            return 0;
        }

        try {
            // Get the graph details as a string
            std::stringstream ss;

            ss << "Graph with " << graph_->getVertices() << " vertices:" << std::endl;

            // Get the adjacency list
            const adj_list& adjList = graph_->getGraph();

            // Print each edge
            for (int i = 0; i < graph_->getVertices(); i++) {
                for (const auto& edge : adjList[i]) {
                    int target = edge.first;
                    int weight = edge.second;
                    ss << i << " -> " << target << " (weight: " << weight << ")" << std::endl;
                }
            }

            // If MST exists, print it too
            if (mst_) {
                ss << std::endl << "Current MST:" << std::endl;
                ss << mst_->toString();
                ss << mst_->getTotalWeightAsString() << std::endl;
            }

            sendResponse(ss.str());
        }
        catch (const std::exception& e) {
            sendResponse("Error printing graph: " + std::string(e.what()) + "\n");
        }

        return 0;
    }
    else if (lowerCmd == "resetgraph") {
        // Reset by creating a new empty graph
        graph_ = std::make_unique<Graph>(0);
        mst_.reset();
        sendResponse("Graph has been reset\n");
        return 0;
    }
    else if (lowerCmd == "help") {
        std::string helpText = "Available commands:\n"
                "  Newgraph <vertices> [<edges>] - Create a new graph with vertices and optional edges count\n"
                "  AddEdge <source> <target> <weight> - Add an edge to the graph\n"
                "  PrintGraph - Display the current graph structure\n"
                "  MST Kruskal - Calculate MST using Kruskal's algorithm\n"
                "  MST Prim - Calculate MST using Prim's algorithm\n"
                "  ResetGraph - Reset the current graph\n"
                "  help - Display this help text\n"
                "  exit - Close connection\n";
        sendResponse(helpText);
        return 0;
    }
    else if (lowerCmd == "exit") {
        sendResponse("Goodbye!\n");
        return -1; // Signal to close the connection
    }
    else {
        // Check if this is a raw edge definition (source target weight)
        std::istringstream iss(command);
        int source, target, weight;
        if (iss >> source >> target >> weight) {
            // This is likely an edge being added
            if (!graph_) {
                sendResponse("Error: No graph initialized. Use 'newgraph' command first.\n");
                return 0;
            }

            try {
                // Add edge to the graph
                graph_->addEdge(source, target, weight);

                // Reset MST since graph changed
                mst_.reset();

                std::string response = "Added edge from " + std::to_string(source) +
                                     " to " + std::to_string(target) +
                                     " with weight " + std::to_string(weight) + "\n";
                sendResponse(response);
            }
            catch (const std::exception& e) {
                sendResponse("Error adding edge: " + std::string(e.what()) + "\n");
            }

            return 0;
        }

        sendResponse("Unknown command: " + command + "\nType 'help' for available commands.\n");
    }

    return 0;
}

std::string CommandHandler::parseCommand(const std::string& input, std::vector<std::string>& args) {
    std::istringstream iss(input);
    std::string token;

    // Extract the command type
    std::string cmdType;
    iss >> cmdType;

    // Extract the arguments
    while (iss >> token) {
        args.push_back(token);
    }

    return cmdType;
}

void CommandHandler::initCommandHandlers() {
    // Map commands to handler methods
    commandHandlers[NEWGRAPH] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handleNewGraph(args);
    };

    commandHandlers[ADDEDGE] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handleAddEdge(args);
    };

    commandHandlers[REMOVEEDGE] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handleRemoveEdge(args);
    };

    commandHandlers[MSTKRUSKAL] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handleMSTKruskal(args);
    };

    commandHandlers[MSTPRIM] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handleMSTPrim(args);
    };

    commandHandlers[PRINTGRAPH] = [](CommandHandler* handler, const std::vector<std::string>& args) {
        return handler->handlePrintGraph(args);
    };
}

int CommandHandler::handleNewGraph(const std::vector<std::string>& args) {
    if (args.empty()) {
        sendResponse("Error: " + NEWGRAPH + " requires at least one argument (number of vertices)\n");
        return 0;
    }

    try {
        int numVertices = std::stoi(args[0]);

        if (numVertices < 0) {
            sendResponse("Error: Number of vertices must be non-negative\n");
            return 0;
        }

        // Create a new graph
        graph_ = std::make_unique<Graph>(numVertices);

        // Reset MST
        mst_.reset();

        std::string response = "Created new graph with " + std::to_string(numVertices) + " vertices\n";
        sendResponse(response);

        // If there's a second argument for edge count, we can optionally handle that
        if (args.size() > 1) {
            int numEdges = std::stoi(args[1]);
            if (numEdges > 0) {
                sendResponse("Ready to receive " + std::to_string(numEdges) + " edges\n");
            }
        }
    }
    catch (const std::exception& e) {
        sendResponse("Error parsing arguments: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

int CommandHandler::handleAddEdge(const std::vector<std::string>& args) {
    if (!graph_) {
        sendResponse("Error: No graph initialized. Use 'new_graph' command first.\n");
        return 0;
    }
    
    if (args.size() < 3) {
        sendResponse("Error: " + ADDEDGE + " requires 3 arguments (source, target, weight)\n");
        return 0;
    }
    
    try {
        int source = std::stoi(args[0]);
        int target = std::stoi(args[1]);
        int weight = std::stoi(args[2]);
        
        if (source < 0 || source >= graph_->getVertices() ||
            target < 0 || target >= graph_->getVertices()) {
            sendResponse("Error: Source or target vertex out of range\n");
            return 0;
        }
        
        // Add edge to the graph
        graph_->addEdge(source, target, weight);
        
        // Reset MST since graph changed
        mst_.reset();
        
        std::string response = "Added edge from " + std::to_string(source) + 
                               " to " + std::to_string(target) + 
                               " with weight " + std::to_string(weight) + "\n";
        sendResponse(response);
    }
    catch (const std::exception& e) {
        sendResponse("Error parsing arguments: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

int CommandHandler::handleRemoveEdge(const std::vector<std::string>& args) {
    if (!graph_) {
        sendResponse("Error: No graph initialized. Use 'new_graph' command first.\n");
        return 0;
    }
    
    if (args.size() < 2) {
        sendResponse("Error: " + REMOVEEDGE + " requires 2 arguments (source, target)\n");
        return 0;
    }
    
    try {
        int source = std::stoi(args[0]);
        int target = std::stoi(args[1]);
        
        if (source < 0 || source >= graph_->getVertices() ||
            target < 0 || target >= graph_->getVertices()) {
            sendResponse("Error: Source or target vertex out of range\n");
            return 0;
        }
        
        // Remove edge from the graph
        graph_->removeEdge(source, target);
        
        // Reset MST since graph changed
        mst_.reset();
        
        std::string response = "Removed edge from " + std::to_string(source) + 
                               " to " + std::to_string(target) + "\n";
        sendResponse(response);
    }
    catch (const std::exception& e) {
        sendResponse("Error parsing arguments: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

int CommandHandler::handleMSTKruskal(const std::vector<std::string>& args) {
    if (!graph_) {
        sendResponse("Error: No graph initialized. Use 'new_graph' command first.\n");
        return 0;
    }
    
    try {
        // Create Kruskal algorithm using factory
        AbstractProductAlgo* algorithm = algoFactory_.createProduct("kruskal");
        
        if (!algorithm) {
            sendResponse("Error: Failed to create Kruskal algorithm\n");
            return 0;
        }
        
        // Execute the algorithm to create MST
        mst_.reset(algorithm->execute(*graph_));
        
        // Clean up algorithm
        delete algorithm;
        
        // Check if MST was created successfully
        if (!mst_) {
            sendResponse("Error: Failed to compute MST\n");
            return 0;
        }
        
        // Send MST details
        std::string response = "MST using Kruskal's algorithm:\n";
        response += mst_->toString();
        response += mst_->getTotalWeightAsString() + "\n";
        response += mst_->getAverageDistanceAsString() + "\n";
        response += mst_->getLongestDistanceAsString() + "\n";
        
        sendResponse(response);
    }
    catch (const std::exception& e) {
        sendResponse("Error computing MST: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

int CommandHandler::handleMSTPrim(const std::vector<std::string>& args) {
    if (!graph_) {
        sendResponse("Error: No graph initialized. Use 'new_graph' command first.\n");
        return 0;
    }
    
    try {
        // Create Prim algorithm using factory
        AbstractProductAlgo* algorithm = algoFactory_.createProduct("prim");
        
        if (!algorithm) {
            sendResponse("Error: Failed to create Prim algorithm\n");
            return 0;
        }
        
        // Execute the algorithm to create MST
        mst_.reset(algorithm->execute(*graph_));
        
        // Clean up algorithm
        delete algorithm;
        
        // Check if MST was created successfully
        if (!mst_) {
            sendResponse("Error: Failed to compute MST\n");
            return 0;
        }
        
        // Send MST details
        std::string response = "MST using Prim's algorithm:\n";
        response += mst_->toString();
        response += mst_->getTotalWeightAsString() + "\n";
        response += mst_->getAverageDistanceAsString() + "\n";
        response += mst_->getLongestDistanceAsString() + "\n";
        
        sendResponse(response);
    }
    catch (const std::exception& e) {
        sendResponse("Error computing MST: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

int CommandHandler::handlePrintGraph(const std::vector<std::string>& args) {
    if (!graph_) {
        sendResponse("Error: No graph initialized. Use 'new_graph' command first.\n");
        return 0;
    }
    
    try {
        // Get the graph details as a string
        std::stringstream ss;
        
        ss << "Graph with " << graph_->getVertices() << " vertices:" << std::endl;
        
        // Get the adjacency list
        const adj_list& adjList = graph_->getGraph();
        
        // Print each edge
        for (int i = 0; i < graph_->getVertices(); i++) {
            for (const auto& edge : adjList[i]) {
                int target = edge.first;
                int weight = edge.second;
                ss << i << " -> " << target << " (weight: " << weight << ")" << std::endl;
            }
        }
        
        // If MST exists, print it too
        if (mst_) {
            ss << std::endl << "Current MST:" << std::endl;
            ss << mst_->toString();
            ss << mst_->getTotalWeightAsString() << std::endl;
        }
        
        sendResponse(ss.str());
    }
    catch (const std::exception& e) {
        sendResponse("Error printing graph: " + std::string(e.what()) + "\n");
    }
    
    return 0;
}

void CommandHandler::sendResponse(const std::string& response) {
    if (handle_.isValid()) {
        write(handle_.get(), response.c_str(), response.length());
    }
}