#include "../../include/leader_followers/HandleSet.hpp"


HandleSet::HandleSet() {
    // Constructor initializes with empty sets
}

bool HandleSet::registerHandler(const Handle& handle, EventType eventType, EventHandler* handler) {
    if (!handle.isValid() || handler == nullptr) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    // Add the handler to the map
    handlers[handle][eventType] = HandlerEntry(handler);
    
    return true;
}

bool HandleSet::removeHandler(const Handle& handle, EventType eventType) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto handleIt = handlers.find(handle);
    if (handleIt == handlers.end()) {
        return false; // Handle not found
    }
    
    auto& eventMap = handleIt->second;
    auto eventIt = eventMap.find(eventType);
    if (eventIt == eventMap.end()) {
        return false; // Event type not found
    }
    
    // Remove the handler for this event type
    eventMap.erase(eventIt);
    
    // If there are no more event types for this handle, remove the handle
    if (eventMap.empty()) {
        handlers.erase(handleIt);
    }
    
    return true;
}

bool HandleSet::suspendHandler(const Handle& handle, EventType eventType) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto handleIt = handlers.find(handle);
    if (handleIt == handlers.end()) {
        return false; // Handle not found
    }
    
    auto& eventMap = handleIt->second;
    auto eventIt = eventMap.find(eventType);
    if (eventIt == eventMap.end()) {
        return false; // Event type not found
    }
    
    // Suspend the handler
    eventIt->second.suspended = true;
    
    return true;
}

bool HandleSet::resumeHandler(const Handle& handle, EventType eventType) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto handleIt = handlers.find(handle);
    if (handleIt == handlers.end()) {
        return false; // Handle not found
    }
    
    auto& eventMap = handleIt->second;
    auto eventIt = eventMap.find(eventType);
    if (eventIt == eventMap.end()) {
        return false; // Event type not found
    }
    
    // Resume the handler
    eventIt->second.suspended = false;
    
    return true;
}

Handle HandleSet::waitForEvents(int timeout) {
    return handleSelect(timeout);
}

bool HandleSet::dispatchEvent(const Handle& handle) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto handleIt = handlers.find(handle);
    if (handleIt == handlers.end()) {
        std::cerr << "Warning: No handlers found for handle " << handle.get() << std::endl;
        return false; // Handle not found
    }

    bool eventDispatched = false;

    // For each event type registered for this handle
    for (auto& eventPair : handleIt->second) {
        EventType eventType = eventPair.first;
        auto& entry = eventPair.second;

        // Skip suspended handlers
        if (entry.suspended) {
            continue;
        }

        // Make sure handler is valid
        if (entry.handler == nullptr) {
            std::cerr << "Error: Null handler for handle " << handle.get() << std::endl;
            continue;
        }

        // Dispatch to the handler
        try {
            // Convert EventType enum to int for the handler interface
            int eventTypeInt = static_cast<int>(eventType);
            int result = entry.handler->handleEvent(handle, eventTypeInt);
            eventDispatched = true;

            // If handler returns error, it may have closed the connection
            if (result < 0) {
                std::cout << "Handler for handle " << handle.get()
                          << " returned error, may need cleanup" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in event handler: " << e.what() << std::endl;
        }
    }
    
    return eventDispatched;
}

Handle HandleSet::handleSelect(int timeout) {
    fd_set readSet, writeSet, exceptSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);
    
    int maxFd = -1;
    
    // Prepare the fd_sets for select()
    prepareFdSets(readSet, writeSet, exceptSet, maxFd);
    
    // If there are no handles, return an invalid handle
    if (maxFd == -1) {
        return Handle();
    }
    
    // Set up the timeout
    struct timeval tv;
    struct timeval* tvPtr = nullptr;
    
    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        tvPtr = &tv;
    }
    
    // Wait for events
    int result = select(maxFd + 1, &readSet, &writeSet, &exceptSet, tvPtr);
    
    if (result < 0) {
        // Error
        std::cerr << "Error in select(): " << strerror(errno) << std::endl;
        return Handle();
    } else if (result == 0) {
        // Timeout
        return Handle();
    }
    
    // Find the handle with an event
    std::lock_guard<std::mutex> lock(mutex);
    
    for (const auto& handlePair : handlers) {
        const Handle& handle = handlePair.first;
        int fd = handle.get();
        
        if (FD_ISSET(fd, &readSet) || FD_ISSET(fd, &writeSet) || FD_ISSET(fd, &exceptSet)) {
            return handle;
        }
    }
    
    // No handle found with an event (should not happen)
    return Handle();
}

void HandleSet::prepareFdSets(fd_set& readSet, fd_set& writeSet, fd_set& exceptSet, int& maxFd) {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (const auto& handlePair : handlers) {
        const Handle& handle = handlePair.first;
        const auto& eventMap = handlePair.second;
        int fd = handle.get();
        
        if (fd < 0) {
            continue; // Skip invalid handles
        }
        
        // Update the maximum file descriptor
        maxFd = std::max(maxFd, fd);
        
        // Add the file descriptor to the appropriate set(s)
        for (const auto& eventPair : eventMap) {
            EventType eventType = eventPair.first;
            const HandlerEntry& entry = eventPair.second;
            
            // Skip suspended handlers
            if (entry.suspended) {
                continue;
            }
            
            if (eventType == EventType::READ || eventType == EventType::ACCEPT) {
                FD_SET(fd, &readSet);
            } else if (eventType == EventType::WRITE || eventType == EventType::CONNECT) {
                FD_SET(fd, &writeSet);
            } else if (eventType == EventType::EXCEPTION) {
                FD_SET(fd, &exceptSet);
            }
        }
    }
}

bool HandleSet::isEmpty() const {
    std::lock_guard<std::mutex> lock(mutex);
    return handlers.empty();
}

size_t HandleSet::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return handlers.size();
}