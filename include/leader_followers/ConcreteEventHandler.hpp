#ifndef CONCRETEEVENTHANDLER_HPP
#define CONCRETEEVENTHANDLER_HPP

#include "EventHandler.hpp"
#include "ThreadPool.hpp"
#include <string>
#include <functional>
//DEPRECATED!!!!!
/**
 * ConcreteEventHandler is a concrete implementation of the EventHandler interface.
 * It handles specific events on specific handles and delegates to a callback function.
 */
class ConcreteEventHandler : public EventHandler {
public:
    /**
     * Constructor.
     * @param handle The handle to associate with this handler.
     * @param threadPool The thread pool to use for event processing.
     * @param callbackFunction The function to call when an event occurs.
     */
    ConcreteEventHandler(
        const Handle& handle,
        ThreadPool* threadPool,
        std::function<int(const Handle&, int)> callbackFunction
    );

    /**
     * Destructor.
     */
    ~ConcreteEventHandler() override = default;

    /**
     * Handle an event on a handle.
     * @param handle The handle on which the event occurred.
     * @param eventType The type of event that occurred.
     * @return 0 on success, -1 on error.
     */
    int handleEvent(const Handle& handle, int eventType) override;

    /**
     * Get the handle associated with this handler.
     * @return The handle associated with this handler.
     */
    Handle getHandle() const override;

private:
    // The handle this handler is associated with
    Handle handle_;

    // The thread pool to use for event processing
    ThreadPool* threadPool_;

    // The callback function to invoke when an event occurs
    std::function<int(const Handle&, int)> callbackFunction_;
};

/**
 * MSTCommandHandler is a specialized EventHandler for handling
 * MST server commands from clients.
 */
class MSTCommandHandler : public EventHandler {
public:
    /**
     * Constructor.
     * @param handle The handle to associate with this handler.
     * @param threadPool The thread pool to use for event processing.
     */
    MSTCommandHandler(const Handle& handle, ThreadPool* threadPool);

    /**
     * Destructor.
     */
    ~MSTCommandHandler() override = default;

    /**
     * Handle a command event.
     * @param handle The handle on which the event occurred.
     * @param eventType The type of event that occurred.
     * @return 0 on success, -1 on error.
     */
    int handleEvent(const Handle& handle, int eventType) override;

    /**
     * Get the handle associated with this handler.
     * @return The handle associated with this handler.
     */
    Handle getHandle() const override;

private:
    // The handle this handler is associated with
    Handle handle_;

    // The thread pool to use for event processing
    ThreadPool* threadPool_;

    // Buffer for command data
    char buffer_[1024];

    // Process a command
    int processCommand(const std::string& command);

    // Parse the command string
    std::string parseCommand(const std::string& input, std::vector<std::string>& args);
};

#endif // CONCRETEEVENTHANDLER_HPP