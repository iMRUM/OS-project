#include "../../include/leader_followers/ThreadSynchronizer.hpp"
#include <iostream>

bool ThreadSynchronizer::waitToBeLeader() {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Wait until there is no leader
    leaderCondition.wait(lock, [this]() { return !hasLeader; });
    
    // Become the leader
    leaderThread = std::this_thread::get_id();
    hasLeader = true;
    
    return true;
}

bool ThreadSynchronizer::promoteFollower() {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if the calling thread is the current leader
    if (leaderThread != std::this_thread::get_id()) {
        std::cerr << "Error: Only the leader thread can promote a follower" << std::endl;
        return false;
    }
    
    // Reset leader status
    leaderThread = std::thread::id();
    hasLeader = false;
    
    // Notify one waiting follower to become the leader
    leaderCondition.notify_one();
    
    return true;
}

bool ThreadSynchronizer::becomeLeader() {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if there's already a leader
    if (hasLeader) {
        return false;
    }
    
    // Set the current thread as the leader
    leaderThread = std::this_thread::get_id();
    hasLeader = true;
    
    return true;
}

bool ThreadSynchronizer::isLeader() const {
    std::lock_guard<std::mutex> lock(mutex);
    return hasLeader && leaderThread == std::this_thread::get_id();
}

bool ThreadSynchronizer::releaseLeader() {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if the calling thread is the current leader
    if (!hasLeader || leaderThread != std::this_thread::get_id()) {
        return false;
    }
    
    // Reset leader status
    leaderThread = std::thread::id();
    hasLeader = false;
    
    // Notify any waiting follower
    leaderCondition.notify_one();
    
    return true;
}