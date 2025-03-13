#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <iostream>
#include "ThreadSynchronizer.hpp"
#include "Handle.hpp"
#include "HandleSet.hpp"
#include "EventHandler.hpp"

/**
 * The ThreadPool class implements the Leader-Followers pattern.
 * In this pattern, one thread (the leader) waits for events to occur,
 * while other threads (followers) wait until they are promoted to become
 * the leader. When an event occurs, the leader promotes a follower to
 * become the new leader, then processes the event as a worker.
 */
class ThreadPool {
public:
    /**
     * Constructor to create a thread pool with a specified number of threads.
     * @param numThreads The number of threads to create in the pool.
     * @param handleSet Optional pointer to a HandleSet for event handling.
     */
    ThreadPool(size_t numThreads, HandleSet* handleSet = nullptr);

    /**
     * Destructor to clean up resources.
     */
    ~ThreadPool();

    /**
     * Start the thread pool.
     */
    void start();

    /**
     * Stop the thread pool.
     */
    void stop();

    /**
     * Add a task to be executed by the thread pool.
     * @param task The task to be executed.
     */
    void addTask(std::function<void()> task);

    /**
     * Join the thread pool to wait for events.
     * This is called by a thread when it wants to become a follower.
     */
    void join();

    /**
     * Promote a follower thread to become the new leader.
     */
    void promoteNewLeader();

    /**
     * Check if the calling thread is the current leader.
     * @return True if the calling thread is the leader, false otherwise.
     */
    bool isLeader() const;

    /**
     * Set the HandleSet for event handling.
     * @param handleSet Pointer to the HandleSet to use.
     */
    void setHandleSet(HandleSet* handleSet);

private:
    /**
     * Worker function for threads in the pool.
     */
    void workerFunction();

    /**
     * Process events and tasks.
     * This is called by the leader thread to wait for events or process tasks.
     */
    void processEventsAndTasks();

    // Thread pool state
    std::vector<std::thread> threads;
    std::atomic<bool> running;

    // Task queue
    std::queue<std::function<void()> > taskQueue;

    // Leader-Followers synchronization
    ThreadSynchronizer synchronizer;

    // Currently active leader thread ID
    std::thread::id leaderThreadId;

    // Mutex to protect shared data
    mutable std::mutex mutex;

    // Condition variables
    std::condition_variable taskCondition; // For waiting on tasks
    std::condition_variable followerCondition; // For followers waiting to become leader

    // Number of threads waiting as followers
    size_t numFollowers;

    // Handle set for event handling
    HandleSet* handleSet_;
};

#endif // THREADPOOL_HPP