#ifndef MSTLEADERFOLLOWERSSERVER_HPP
#define MSTLEADERFOLLOWERSSERVER_HPP

#include "../server/MSTServer.hpp"
#include "../leader_followers/ThreadPool.hpp"
#include "../leader_followers/HandleSet.hpp"
#include "../leader_followers/AcceptHandler.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"

/**
 * MSTLeaderFollowersServer implements the MSTServer using the Leader-Followers pattern.
 * It manages a thread pool where threads take turns being the leader to handle events
 * on socket handles. The server handles client connections and MST commands.
 */
class MSTLeaderFollowersServer : public MSTServer {
private:
    // Number of threads in the pool
    size_t numThreads_;

    // Thread pool for the Leader-Followers pattern
    std::unique_ptr<ThreadPool> threadPool_;

    // Handle set for managing socket handles
    std::unique_ptr<HandleSet> handleSet_;

    // Factory for creating MST algorithms
    ConcreteAlgoFactory algoFactory_;

    // Accept handler for handling new connections
    std::unique_ptr<AcceptHandler> acceptHandler_;

    // Map of client sockets to event handlers
    std::map<int, EventHandler *> clientHandlers_;

    // Mutex for protecting shared data
    std::mutex mutex_;

    // Close a client connection
    void closeClient(int socketFd);

    /**
     * Handle a new client connection.
     * In the Leader-Followers implementation, this is primarily handled
     * by the AcceptHandler, but we implement this method to satisfy
     * the base class interface.
     * @param clientData Data for the new client.
     */
    void handleClient(ClientData *clientData) override;

    /**
     * Process a command from a client.
     * In the Leader-Followers implementation, this is handled by
     * the CommandHandler, but we implement this method to satisfy
     * the base class interface.
     * @param socketFd The client socket file descriptor.
     * @param command The command string.
     */
    void processCommand(int socketFd, std::string &command) override;

public:
    /**
     * Constructor.
     * @param numThreads The number of threads to create in the pool.
     * The default is 4 (which is suitable for most systems) providing:
     * - A dedicated leader thread
     * - Multiple worker threads for concurrent request processing
     */
    MSTLeaderFollowersServer(size_t numThreads = 4);

    /**
     * Destructor.
     */
    ~MSTLeaderFollowersServer() override;

    /**
     * Start the server.
     * Initializes the socket, creates the thread pool, and begins
     * processing client connections using the Leader-Followers pattern.
     */
    void start() override;

    void stop() override;

protected:
};

#endif // MSTLEADERFOLLOWERSSERVER_HPP
