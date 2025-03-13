#include "../../include/leader_followers/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads, HandleSet* handleSet)
    : running(false),
      numFollowers(0),
      handleSet_(handleSet) {

    // Reserve space for threads
    threads.reserve(numThreads);
}

ThreadPool::~ThreadPool() {
    if (running) {
        stop();
    }
}

void ThreadPool::start() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (running) {
            return; // Already running
        }
        running = true;
    }

    // Create the worker threads
    for (size_t i = 0; i < threads.capacity(); ++i) {
        threads.emplace_back(&ThreadPool::workerFunction, this);
    }

    std::cout << "Thread pool started with " << threads.size() << " threads" << std::endl;
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!running) {
            return; // Already stopped
        }
        running = false;
    }

    // Notify all threads to wake up and exit
    taskCondition.notify_all();
    followerCondition.notify_all();

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Clear the thread container
    threads.clear();

    std::cout << "Thread pool stopped" << std::endl;
}

void ThreadPool::addTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        taskQueue.push(task);
    }

    // Notify one thread that a task is available
    taskCondition.notify_one();
}

void ThreadPool::join() {
    std::unique_lock<std::mutex> lock(mutex);

    while (running) {
        // Check if there's no leader - if so, become the leader using ThreadSynchronizer
        if (synchronizer.becomeLeader()) {
            std::cout << "Thread " << std::this_thread::get_id() << " is now the leader" << std::endl;

            // Release the lock to allow other threads to join as followers
            lock.unlock();

            // Process events and tasks
            processEventsAndTasks();

            // Reacquire the lock to check if we need to become a follower
            lock.lock();

            // If we're still the leader (no follower was promoted), continue as leader
            if (synchronizer.isLeader()) {
                continue;
            }
        }

        // We're not the leader, so wait as a follower
        ++numFollowers;
        // Wait until promoted to leader or until the pool is stopped
        followerCondition.wait(lock, [this]() {
            return !running || synchronizer.isLeader();
        });

        --numFollowers;
    }
}

void ThreadPool::processEventsAndTasks() {
    // If we have a HandleSet, wait for events on it
    if (handleSet_ != nullptr && !handleSet_->isEmpty()) {
        // First check for any tasks
        bool hasTask = false;
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> taskLock(mutex);
            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
                hasTask = true;
            }
        }

        // If there's a task, process it first
        if (hasTask) {
            // Promote a new leader before processing the task
            promoteNewLeader();

            // Process the task
            if (task) {
                task();
            }

            return;
        }

        // No task, so wait for events on handles (with a short timeout to check for tasks periodically)
        Handle handle = handleSet_->waitForEvents(1000);

        // If we got a valid handle, dispatch the event
        if (handle.isValid()) {
            // Promote a new leader before processing the event
            promoteNewLeader();

            // Dispatch the event to its handler
            handleSet_->dispatchEvent(handle);

            return;
        }

        // If we reach here, there was no event, so we'll check for tasks again on the next iteration
    } else {
        // No HandleSet or empty HandleSet, just wait for tasks
        bool hasTask = false;
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> taskLock(mutex);
            // Wait for a task or stop signal
            taskCondition.wait(taskLock, [this, &hasTask, &task]() {
                if (!running) {
                    return true; // Stop signal received
                }

                if (!taskQueue.empty()) {
                    task = taskQueue.front();
                    taskQueue.pop();
                    hasTask = true;
                    return true; // Task available
                }

                return false; // Keep waiting
            });
        }

        // If we're stopped, exit
        if (!running) {
            // Release leadership status before exiting
            synchronizer.releaseLeader();
            return;
        }

        // If we have a task, process it after promoting a new leader
        if (hasTask) {
            // Promote a new leader before processing the task
            promoteNewLeader();

            // Process the task
            if (task) {
                task();
            }
        }
    }
}

void ThreadPool::promoteNewLeader() {
    // Check if the calling thread is the current leader using ThreadSynchronizer
    if (!synchronizer.isLeader()) {
        std::cerr << "Error: Only the leader thread can promote a follower" << std::endl;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        // If there are no followers, there's no thread to promote
        if (numFollowers == 0) {
            std::cout << "No followers to promote" << std::endl;
            return;
        }
    }

    // Use ThreadSynchronizer to promote a follower
    synchronizer.promoteFollower();

    // Notify a follower thread
    followerCondition.notify_one();

    std::cout << "Leader promoted a new follower to leader" << std::endl;
}

bool ThreadPool::isLeader() const {
    // Delegate to ThreadSynchronizer
    return synchronizer.isLeader();
}

void ThreadPool::setHandleSet(HandleSet* handleSet) {
    handleSet_ = handleSet;
}

void ThreadPool::workerFunction() {
    while (running) {
        join(); // Participate in the leader-followers pattern
        
        // If the pool is stopped, exit
        if (!running) {
            break;
        }
    }
}