#ifndef THREADSYNCHRONIZER_HPP
#define THREADSYNCHRONIZER_HPP

#include <mutex>
#include <condition_variable>
#include <thread>

/**
 * The ThreadSynchronizer class provides synchronization mechanisms
 * for the Leader-Followers pattern. It manages the state transitions
 * between leader and follower threads.
 */
class ThreadSynchronizer {
public:
    /**
     * Constructor.
     */
    ThreadSynchronizer() : leaderThread(std::thread::id()), hasLeader(false) {}
    
    /**
     * Destructor.
     */
    ~ThreadSynchronizer() = default;
    
    /**
     * Wait to become the leader thread.
     * @return True if successful, false if timeout or error.
     */
    bool waitToBeLeader();
    
    /**
     * Promote a follower thread to become the new leader.
     * The current leader calls this method before processing an event.
     * @return True if a follower was promoted, false if no followers available.
     */
    bool promoteFollower();
    
    /**
     * Register the current thread as the leader.
     * @return True if successful, false if there's already a leader.
     */
    bool becomeLeader();
    
    /**
     * Check if the current thread is the leader.
     * @return True if the current thread is the leader, false otherwise.
     */
    bool isLeader() const;
    
    /**
     * Release leadership status.
     * @return True if successful, false if the calling thread wasn't the leader.
     */
    bool releaseLeader();

private:
    // The ID of the current leader thread
    std::thread::id leaderThread;
    
    // Flag indicating if a leader exists
    bool hasLeader;
    
    // Mutex for synchronization
    mutable std::mutex mutex;
    
    // Condition variable for followers waiting to become leader
    std::condition_variable leaderCondition;
};

#endif // THREADSYNCHRONIZER_HPP