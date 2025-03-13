#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <thread>
#include <atomic>
#include "MethodRequest.hpp"
#include "ActivationQ.hpp"

class MSTScheduler {
public:
    MSTScheduler(size_t queueCapacity = SIZE_MAX)
        : activation_q(queueCapacity), running(false) {}

    ~MSTScheduler() {
        // Ensure thread is stopped
        if (running) {
            stop();
        }
    }

    // Start the scheduler in its own thread
    void start() {
        running = true;
        scheduler_thread = std::thread(&MSTScheduler::dispatch, this);
    }

    // Stop the scheduler thread
    void stop() {
        running = false;

        // Unblock the thread if it's waiting on an empty queue
        // by adding a dummy request that will be discarded
        enqueue(new DummyRequest());

        // Wait for the thread to finish
        if (scheduler_thread.joinable()) {
            scheduler_thread.join();
        }

        // Clean up any remaining requests
        cleanupPendingRequests();
    }

    // Enqueue a method request
    void enqueue(MethodRequest* request) {
        activation_q.enqueue(request);
    }

private:
    // Main scheduler loop - runs in a separate thread
    void dispatch() {
        while (running) {
            // Dequeue the next request (blocking call)
            MethodRequest* request = activation_q.dequeue();

            // Skip if we're shutting down
            if (!running) {
                delete request;
                break;
            }

            // Check if the request can be executed
            if (request->guard()) {
                // Execute the request
                try {
                    request->call();
                } catch (const std::exception& e) {
                    // Log exception (in a real system)
                    std::cerr << "Exception in method request: "
                              << e.what() << std::endl;
                }
            } else {
                // Re-queue requests that can't be executed yet
                activation_q.enqueue(request);

                // Sleep briefly to avoid tight loop when all requests
                // have guard conditions that aren't met
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Clean up
            delete request;
        }
    }

    // Clean up any requests still in the queue when shutting down
    void cleanupPendingRequests() {
        while (!activation_q.isEmpty()) {
            MethodRequest* request = activation_q.dequeue();
            delete request;
        }
    }

    // Dummy request used to unblock the dispatch thread when shutting down
    class DummyRequest : public MethodRequest {
    public:
        bool guard() const override { return true; }
        void call() override { }
    };

    ActivationQ activation_q;
    std::thread scheduler_thread;
    std::atomic<bool> running;
};
#endif //SCHEDULER_HPP
