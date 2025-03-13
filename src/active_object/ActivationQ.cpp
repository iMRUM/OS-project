#include "../../include/active_object/ActivationQ.hpp"

void ActivationQ::enqueue(MethodRequest *request) {
    std::unique_lock<std::mutex> lock(mutex);

    // Wait until there's space in the queue
    not_full.wait(lock, [this] {
        return requests.size() < capacity;
    });

    // Add the request to the queue
    requests.push(request);

    // Notify threads waiting to dequeue
    not_empty.notify_one();
}

MethodRequest *ActivationQ::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);

    // Wait until there's at least one request in the queue
    not_empty.wait(lock, [this] {
        return !requests.empty();
    });

    // Remove and return the next request
    MethodRequest *request = requests.front();
    requests.pop();

    // Notify threads waiting to enqueue
    not_full.notify_one();
    return request;
}

bool ActivationQ::isEmpty() const {
    std::lock_guard<std::mutex> lock(mutex);
    return requests.empty();
}

size_t ActivationQ::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return requests.size();
}
