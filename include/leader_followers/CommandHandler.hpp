#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "EventHandler.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"
#include "../dsa/Graph.hpp"
#include "../dsa/MST.hpp"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <sstream>
#include <functional>
#include <iostream>

/**
 * CommandHandler processes MST commands received from a client.
 * It interprets and executes commands like new_graph, add_edge, etc.
 */
class CommandHandler : public EventHandler {
public:
    /**
     * Constructor.
     * @param handle The client socket handle.
     * @param factory Reference to the algorithm factory.
     */
    CommandHandler(const Handle& handle, ConcreteAlgoFactory& factory);
    
    /**
     * Destructor.
     */
    ~CommandHandler() override = default;
    
    /**
     * Handle an event on the client socket.
     * @param handle The handle on which the event occurred.
     * @param eventType The type of event that occurred.
     * @return 0 on success, -1 on error.
     */
    int handleEvent(const Handle& handle, int eventType) override;
    
    /**
     * Get the client socket handle.
     * @return The client socket handle.
     */
    Handle getHandle() const override;
    
private:
    // The client socket handle
    Handle handle_;
    
    // Reference to the algorithm factory
    ConcreteAlgoFactory& algoFactory_;
    
    // Current graph instance for this client
    std::unique_ptr<Graph> graph_;
    
    // Current MST instance for this client
    std::unique_ptr<MST> mst_;
    
    // Buffer for reading commands
    char buffer_[1024];
    
    // Process a command
    int processCommand(const std::string& command);
    
    // Parse the command string into command type and arguments
    std::string parseCommand(const std::string& input, std::vector<std::string>& args);
    
    // Command handlers for each command type
    using CommandFunc = std::function<int(CommandHandler*, const std::vector<std::string>&)>;
    std::map<std::string, CommandFunc> commandHandlers;
    
    // Initialize command handlers
    void initCommandHandlers();
    
    // Command handler methods
    int handleNewGraph(const std::vector<std::string>& args);
    int handleAddEdge(const std::vector<std::string>& args);
    int handleRemoveEdge(const std::vector<std::string>& args);
    int handleMSTKruskal(const std::vector<std::string>& args);
    int handleMSTPrim(const std::vector<std::string>& args);
    int handlePrintGraph(const std::vector<std::string>& args);
    
    // Send a response to the client
    void sendResponse(const std::string& response);
};

#endif // COMMANDHANDLER_HPP