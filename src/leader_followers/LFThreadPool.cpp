#include "../../include/leader_followers/LFThreadPool.hpp"

int LFThreadPool::promote_new_leader() {
    pthread_mutex_lock(&promotion_mutex);
    leader_semaphore.release();
    pthread_mutex_unlock(&promotion_mutex);
    return 0;
}

int LFThreadPool::join() {
    try {
        while (running) {
            leader_semaphore.acquire();
            if (!running) {
                promote_new_leader();
                break;
            }

            pthread_mutex_lock(&promotion_mutex);
            leader_thread = pthread_self();
            pthread_mutex_unlock(&promotion_mutex);


            // Make a copy of fd_set without holding mutex during select()
            fd_set read_fds;
            int max_fd;

            // Critical section - get what we need quickly
            pthread_mutex_lock(&reactor_mutex);
            if (!reactor_p) {
                std::cerr << "[Thread " << pthread_self() << "] reactor_p is NULL!" << std::endl;
                pthread_mutex_unlock(&reactor_mutex);
                leader_semaphore.release();
                continue;
            }
            read_fds = reactor_p->fds;
            max_fd = reactor_p->max_fd;
            pthread_mutex_unlock(&reactor_mutex);
            int ready = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
            if (ready > 0) {
                // Process all ready file descriptors
                for (int fd = 0; fd <= max_fd; fd++) {
                    if (FD_ISSET(fd, &read_fds)) {
                        // Get callback safely
                        reactorFunc callback = nullptr;

                        pthread_mutex_lock(&reactor_mutex);
                        if (FD_ISSET(fd, &reactor_p->fds)) {
                            callback = reactor_p->r_funcs[fd];
                        }
                        pthread_mutex_unlock(&reactor_mutex);

                        if (callback) {
                            // Remove fd, promote leader, then handle the event
                            removeFd(fd);
                            promote_new_leader();

                            //std::cout << "[LFThreadPool] Thread " << pthread_self() << " handling event on fd " << fd << std::endl;

                            callback(fd);

                            // Add the fd back when done
                            addFd(fd, callback);
                            break;
                        }
                    }
                }
            } else {
                std::cerr << "[Thread " << pthread_self() << "] select() failed: " << strerror(errno) << std::endl;
                leader_semaphore.release();
                continue;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "[Thread " << pthread_self() << "] Exception in join(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[Thread " << pthread_self() << "] Unknown exception in join()" << std::endl;
    }
    return 0;
}

void LFThreadPool::addFd(int fd, reactorFunc func) {
    pthread_mutex_lock(&(reactor_p->r_mtx));
    addFdToReactor(reactor_p, fd, func);
    pthread_mutex_unlock(&(reactor_p->r_mtx));
}

void LFThreadPool::removeFd(int fd) {
    pthread_mutex_lock(&(reactor_p->r_mtx));
    removeFdFromReactor(reactor_p, fd);
    pthread_mutex_unlock(&(reactor_p->r_mtx));
}