// Future.hpp
#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class Future {
private:
    bool ready = false;
    std::optional<T> value;
    mutable std::mutex mutex;
    std::condition_variable cv;

public:
    // Default constructor - creates an unresolved future
    Future() : ready(false) {}

    // Constructor that directly sets a value (for immediate results)
    Future(const T& initialValue) : ready(true), value(initialValue) {}

    // Copy constructor
    Future(const Future& other) {
        std::lock_guard<std::mutex> lock(other.mutex);
        ready = other.ready;
        value = other.value;
    }

    // Assignment operator
    Future& operator=(const Future& other) {
        if (this != &other) {
            std::lock_guard<std::mutex> lock1(mutex);
            std::lock_guard<std::mutex> lock2(other.mutex);
            ready = other.ready;
            value = other.value;
        }
        return *this;
    }

    // Set the result and notify waiting threads
    void set(const T& v) {
        std::lock_guard<std::mutex> lock(mutex);
        value = v;
        ready = true;
        cv.notify_all();
    }

    // Get the result, blocking if necessary until it's available
    T get() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]{ return ready; });
        return *value;
    }

    // Check if the result is available without blocking
    bool isReady() const {
        std::lock_guard<std::mutex> lock(mutex);
        return ready;
    }
};