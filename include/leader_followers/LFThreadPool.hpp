#ifndef LFTHREADPOOL_HPP
#define LFTHREADPOOL_HPP
#include <atomic>
#include <semaphore>

#include "Reactor.hpp"

//when new thread begins, it would store reactFunc[fd], remove that fd from reactor, handle the event, reactivate with same fd and func
class LFThreadPool {
private:
    reactor_t *reactor_p;
    pthread_t leader_thread;
    std::binary_semaphore leader_semaphore;
    pthread_mutex_t promotion_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t reactor_mutex = PTHREAD_MUTEX_INITIALIZER;
    std::atomic<bool> running{true};

public:
    LFThreadPool(reactor_t *reactor_ptr = (reactor_t *) startReactor()): reactor_p(reactor_ptr), leader_semaphore(1) {
        std::cout << "[LFThreadPool] Creating thread pool..." << std::endl;
    }

    ~LFThreadPool() {
        stopReactor(reactor_p);
        std::cout << "[LFThreadPool] Destroying thread pool..." << std::endl;
        free(reactor_p);
    }

    void addFd(int fd, reactorFunc func);

    void removeFd(int fd);

    int join();

    int promote_new_leader();
};
#endif //LFTHREADPOOL_HPP
