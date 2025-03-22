#include "../../include/leader_followers/LFThreadPool.hpp"
/*
    reactor_t *reactor_p;
    pthread_t leader_thread;
    std::binary_semaphore leader_semaphore;
    pthread_mutex promotion_mutex;
 */
int LFThreadPool::promote_new_leader() {
    pthread_mutex_lock(&promotion_mutex);
    leader_semaphore.release();
    pthread_mutex_unlock(&promotion_mutex);
    return 0;
}

// Thread joins the pool and becomes a follower (or leader if first)
int LFThreadPool::join() {
    while (running) {
        // Try to acquire the leader permit
        leader_semaphore.acquire();

        if (!running) {
            leader_semaphore.release(); // Let another thread exit too
            break;
        }

        // This thread is now the leader
        leader_thread = pthread_self();
        std::cout << "[LFThreadPool] Thread " << leader_thread << " became leader" << std::endl;

        // Leader monitors the reactor for events
        pthread_mutex_lock(&reactor_mutex);
        std::cout << "[LFThreadPool] Thread " << leader_thread << " locked reactor_mutex" << std::endl;
        fd_set read_fds = reactor_p->fds;
        int read_fds_size = reactor_p->max_fd;
        pthread_mutex_unlock(&reactor_mutex);
        std::cout << "[LFThreadPool] Thread " << leader_thread << " unlocked reactor_mutex" << std::endl;
        int ready = select(read_fds_size + 1, &read_fds, nullptr, nullptr, nullptr);
        std::cout << "[LFThreadPool] Thread " << leader_thread << " has update" << std::endl;
        if (ready > 0) {
            // Found activity - find which fd is ready
            for (int fd = 0; fd <= reactor_p->max_fd; fd++) {
                if (FD_ISSET(fd, &read_fds)) {
                    // Save the callback function
                    pthread_mutex_lock(&reactor_mutex);
                    std::cout << "[LFThreadPool] Thread " << leader_thread << " locked2 reactor_mutex" << std::endl;
                    reactorFunc callback = reactor_p->r_funcs[fd];
                    pthread_mutex_unlock(&reactor_mutex);
                    std::cout << "[LFThreadPool] Thread " << leader_thread << " unlocked2 reactor_mutex" << std::endl;

                    // Temporarily remove this fd from the reactor
                    removeFd(fd);

                    // Promote a new leader before processing
                    promote_new_leader();

                    // Process the event
                    callback(fd);

                    // Add the fd back to the reactor
                    addFd(fd, callback);

                    break;
                }
            }
        } else {
            // No events, release leadership and try again
            leader_semaphore.release();
        }
    }

    return 0;
}

void LFThreadPool::addFd(int fd, reactorFunc func) {
    pthread_mutex_lock(&reactor_mutex);
    addFdToReactor(reactor_p, fd, func);
    pthread_mutex_unlock(&reactor_mutex);
}

void LFThreadPool::removeFd(int fd) {
    pthread_mutex_lock(&reactor_mutex);
    if(removeFdFromReactor(reactor_p, fd) < 0) {
        perror("removeFdFromReactor");
    }
    pthread_mutex_unlock(&reactor_mutex);
}
