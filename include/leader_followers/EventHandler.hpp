#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include "Handle.hpp"

/**
 * EventHandler is an abstract base class that defines the interface
 * for handling events on handles. Concrete event handlers must inherit
 * from this class and implement the handle_event method.
 */
class EventHandler {
public:
    /**
     * Constructor.
     */
    EventHandler() = default;

    /**
     * Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~EventHandler() = default;

    /**
     * Handle an event on a handle.
     * @param handle The handle on which the event occurred.
     * @param eventType The type of event that occurred.
     * @return 0 on success, -1 on error.
     */
    virtual int handleEvent(const Handle& handle, int eventType) = 0;

    /**
     * Get the handle associated with this handler.
     * @return The handle associated with this handler.
     */
    virtual Handle getHandle() const = 0;
};

#endif // EVENTHANDLER_HPP