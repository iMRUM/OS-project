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

    /*Try to dequeue with a timeout, returns nullptr if timed out
    MethodRequest* dequeue(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        
        // Wait until there's at least one request in the queue or timeout
        bool success = not_empty.wait_for(lock, timeout, [this] { 
            return !requests.empty(); 
        });
        
        if (!success) {
            return nullptr;  // Timed out
        }
        
        // Remove and return the next request
        MethodRequest* request = requests.front();
        requests.pop();
        
        // Notify threads waiting to enqueue
        not_full.notify_one();
        return request;
    }*/

    // Check if the queue is empty
    bool isEmpty() const;

    // Get the current size of the queue
    size_t size() const;
};
#endif //ACTIVATIONQ_HPP
