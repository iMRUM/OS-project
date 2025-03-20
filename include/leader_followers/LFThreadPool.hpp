#ifndef LFTHREADPOOL_HPP
#define LFTHREADPOOL_HPP
#include <semaphore>

#include "Reactor.hpp"

//when new thread begins, it would store reactFunc[fd], remove that fd from reactor, handle the event, reactivate with same fd and func
class LFThreadPool {
private:
    reactor_t *reactor_p;
    pthread_t leader_thread;
    std::binary_semaphore followers_semaphore;
    std::mutex promotion_mutex;

public:
    LFThreadPool(reactor_t *reactor_ptr = (reactor_t *) startReactor()): reactor_p(reactor_ptr) {
        std::cout << "[LFThreadPool] Creating thread pool..." << std::endl;
    }
    int join;
    int promote_new_leader();
};
#endif //LFTHREADPOOL_HPP
