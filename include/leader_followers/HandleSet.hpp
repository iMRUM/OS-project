#ifndef HANDLESET_HPP
#define HANDLESET_HPP

#include <vector>
#include <map>
#include <mutex>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include "Handle.hpp"
#include "EventHandler.hpp"


// Event types for handles
enum class EventType {
    READ,
    WRITE,
    EXCEPTION,
    ACCEPT,
    CONNECT
};

/**
 * HandleSet class manages a collection of handles and their associated
 * event handlers. It provides methods for waiting on events and
 * dispatching them to the appropriate handlers.
 */
class HandleSet {
public:
    /**
     * Constructor.
     */
    HandleSet();

    /**
     * Destructor.
     */
    ~HandleSet() = default;

    /**
     * Register an event handler for a handle and event type.
     * @param handle The handle to register.
     * @param eventType The type of event to register for.
     * @param handler The event handler to associate with the handle.
     * @return True if successful, false otherwise.
     */
    bool registerHandler(const Handle& handle, EventType eventType, EventHandler* handler);

    /**
     * Remove a handler for a specific handle and event type.
     * @param handle The handle to unregister.
     * @param eventType The type of event to unregister.
     * @return True if successful, false otherwise.
     */
    bool removeHandler(const Handle& handle, EventType eventType);

    /**
     * Temporarily suspend a handler for a specific handle and event type.
     * @param handle The handle to suspend.
     * @param eventType The type of event to suspend.
     * @return True if successful, false otherwise.
     */
    bool suspendHandler(const Handle& handle, EventType eventType);

    /**
     * Resume a previously suspended handler.
     * @param handle The handle to resume.
     * @param eventType The type of event to resume.
     * @return True if successful, false otherwise.
     */
    bool resumeHandler(const Handle& handle, EventType eventType);

    /**
     * Wait for events on registered handles, with optional timeout.
     * @param timeout The maximum time to wait for events (in milliseconds).
     *                Use -1 for infinite timeout.
     * @return Handle with an event, or invalid handle if timeout or error.
     */
    Handle waitForEvents(int timeout = -1);

    /**
     * Dispatch the event for a specific handle to its handler.
     * @param handle The handle with an event.
     * @return True if successful, false otherwise.
     */
    bool dispatchEvent(const Handle& handle);

    /**
     * Check if the handle set is empty.
     * @return True if empty, false otherwise.
     */
    bool isEmpty() const;

    /**
     * Get the number of handles in the set.
     * @return The number of handles.
     */
    size_t size() const;
    
private:
    // Structure to store handler and state
    struct HandlerEntry {
        EventHandler* handler;
        bool suspended;
        
        HandlerEntry() : handler(nullptr), suspended(false) {}
        HandlerEntry(EventHandler* h) : handler(h), suspended(false) {}
    };
    
    // Map of handle -> event type -> handler entry
    std::map<Handle, std::map<EventType, HandlerEntry>> handlers;
    
    // Mutex for thread safety
    mutable std::mutex mutex;
    
    // Implementation of select() call
    Handle handleSelect(int timeout);
    
    // Prepare fd_sets for select()
    void prepareFdSets(fd_set& readSet, fd_set& writeSet, fd_set& exceptSet, int& maxFd);
};

#endif // HANDLESET_HPP