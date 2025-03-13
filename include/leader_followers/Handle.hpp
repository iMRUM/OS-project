#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cerrno>
/**
 * Handle class represents a system resource such as a socket.
 * It wraps operating system handles and provides operations
 * for managing these resources in an object-oriented way.
 */
class Handle {
public:
    /**
     * Constructor for an invalid handle.
     */
    Handle() : handle_(-1) {
    }

    /**
     * Constructor with an existing handle.
     * @param handle The system handle to wrap.
     */
    explicit Handle(int handle) : handle_(handle) {
    }

    /**
     * Destructor.
     */
    ~Handle() = default;

    /**
     * Get the underlying system handle.
     * @return The system handle.
     */
    int get() const { return handle_; }

    /**
     * Check if the handle is valid.
     * @return True if the handle is valid, false otherwise.
     */
    bool isValid() const { return handle_ >= 0; }

    /**
     * Set a new handle value.
     * @param handle The new system handle.
     */
    void set(int handle) { handle_ = handle; }

    /**
     * Close the handle.
     * @return True if successful, false otherwise.
     */
    bool close();

    /**
     * Convert to string representation.
     * @return String representation of the handle.
     */
    std::string toString() const;

    /**
     * Compare two handles for equality.
     * @param other The other handle to compare with.
     * @return True if the handles are equal, false otherwise.
     */
    bool operator==(const Handle &other) const {
        return handle_ == other.handle_;
    }

    /**
     * Compare two handles for inequality.
     * @param other The other handle to compare with.
     * @return True if the handles are not equal, false otherwise.
     */
    bool operator!=(const Handle &other) const {
        return !(*this == other);
    }

    /**
         * Compare handles for ordering (needed for map storage).
         * @param other The other handle to compare with.
         * @return True if this handle is less than the other, false otherwise.
         */
    bool operator<(const Handle &other) const {
        return handle_ < other.handle_;
    }

private:
    // The system handle
    int handle_;
};

#endif // HANDLE_HPP
