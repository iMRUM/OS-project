#ifndef ACTIVATIONQ_HPP
#define ACTIVATIONQ_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include "MethodRequest.hpp"

class ActivationQ {
private:
    std::queue<MethodRequest *> requests;
    size_t capacity;
    mutable std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

public:
    // Constructor with parameter for maximum capacity
    ActivationQ(size_t capacity = SIZE_MAX)
        : capacity(capacity) {
    }

    // Enqueue a method request, blocking if the queue is full
    void enqueue(MethodRequest *request);

    // Dequeue a method request, blocking if the queue is empty
    MethodRequest *dequeue();

    // Check if the queue is empty
    bool isEmpty() const;

    // Get the current size of the queue
    size_t size() const;
};
#endif //ACTIVATIONQ_HPP
