#ifndef ACCEPTHANDLER_HPP
#define ACCEPTHANDLER_HPP

#include "EventHandler.hpp"
#include "HandleSet.hpp"
#include "CommandHandler.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"
#include <functional>

/**
 * AcceptHandler handles the acceptance of new client connections.
 * It accepts connections on a listening socket and creates new
 * CommandHandlers for each client connection.
 */
class AcceptHandler : public EventHandler {
public:
    /**
     * Constructor.
     * @param handle The listening socket handle.
     * @param handleSet Reference to the handle set for registering handlers.
     * @param factory Reference to the algorithm factory.
     */
    AcceptHandler(const Handle& handle, HandleSet& handleSet, ConcreteAlgoFactory& factory);
    
    /**
     * Destructor.
     */
    ~AcceptHandler() override = default;
    
    /**
     * Handle an accept event on the listening socket.
     * @param handle The handle on which the event occurred.
     * @param eventType The type of event that occurred.
     * @return 0 on success, -1 on error.
     */
    int handleEvent(const Handle& handle, int eventType) override;
    
    /**
     * Get the listening socket handle.
     * @return The listening socket handle.
     */
    Handle getHandle() const override;
    
private:
    // The listening socket handle
    Handle handle_;
    
    // Reference to the handle set for registering handlers
    HandleSet& handleSet_;
    
    // Reference to the algorithm factory
    ConcreteAlgoFactory& factory_;
    
    // Accept a new client connection
    int acceptClient();
    
    // Close a client connection
    void closeClient(int socketFd);
};

#endif // ACCEPTHANDLER_HPP