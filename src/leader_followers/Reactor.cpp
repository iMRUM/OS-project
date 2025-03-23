#include "../../include/leader_followers/Reactor.hpp"



/*
* struct reactor {
    fd_set fds;       / Master set of file descriptors /
    int max_fd;              / Highest file descriptor value /
    int running;             / Flag to control reactor loop /
    reactorFunc r_funcs[MAX_FDS]; / Array of callback functions /
    pthread_mutex_t r_mtx = PTHREAD_MUTEX_INITIALIZER; / reactor mutex /
};
 */
void * startReactor() {
    reactor_t* reactor = (reactor_t*)malloc(sizeof(reactor_t));
    if (reactor == nullptr) {
        perror("Failed to allocate memory for reactor");
        return nullptr;
    }

    // Initialize the reactor structure
    FD_ZERO(&reactor->fds);
    reactor->max_fd = -1;
    reactor->running = 1;
    memset(reactor->r_funcs, 0, sizeof(reactor->r_funcs));
    pthread_mutex_init(&(reactor->r_mtx), NULL);
    return reactor;
}

int addFdToReactor(void *reactor, int fd, reactorFunc func) {
    reactor_t* r = (reactor_t*)reactor;

    if (r == nullptr || fd < 0 || fd >= MAX_FDS || func == nullptr) {
        errno = EINVAL;
        return -1;
    }
    // Add the file descriptor to the set
    FD_SET(fd, &r->fds);

    // Update max_fd if necessary
    if (fd > r->max_fd) {
        r->max_fd = fd;
    }

    // Store the callback function
    r->r_funcs[fd] = func;
    return 0;
}

int removeFdFromReactor(void *reactor, int fd) {
    reactor_t* r = (reactor_t*)reactor;
    if (r == nullptr || fd < 0 || fd >= MAX_FDS) {
        errno = EINVAL;
        return -1;
    }

    // Clear the file descriptor from the set
    FD_CLR(fd, &r->fds);

    // Clear the callback
    r->r_funcs[fd] = nullptr;
    // Recalculate max_fd if necessary
    if (fd == r->max_fd) {
        // Start from the previous max_fd and search downward
        r->max_fd = -1;
        for (int i = fd - 1; i >= 0; i--) {
            if (FD_ISSET(i, &r->fds)) {// Found the new highest fd
                r->max_fd = i;
                break;
            }
        }
    }
    //std::cout<<"removed socket "<<fd<<". max socket is "<<r->max_fd<<"\n";
    return r->max_fd;
}

int stopReactor(void *reactor) {
    reactor_t* r = (reactor_t*)reactor;
    if (r == nullptr) {
        errno = EINVAL;
        return -1;
    }
    r->running = 0;
    FD_ZERO(&r->fds);
    memset(r->r_funcs, 0, sizeof(r->r_funcs));
    free(r);
    return 0;
}

