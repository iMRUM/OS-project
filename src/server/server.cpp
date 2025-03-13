#include "../../include/server/MSTPipelineServer.hpp"
#include "../../include/server/MSTLeaderFollowersServer.hpp"
#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <csignal>
#include <thread>

// Global server pointer for signal handling
MSTServer* serverPtr = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    // Cleanup and close the server
    if (serverPtr) {
        serverPtr->stop();
    }

    // Terminate program
    exit(signum);
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -p, --pipeline         Use pipeline implementation (default)\n"
              << "  -l, --leader-followers Use leader/followers implementation\n"
              << "  -t, --threads NUM      Number of threads to use (default: CPU cores)\n"
              << "  -h, --help             Display this help and exit\n";
}

int main(int argc, char* argv[]) {
    // Register signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Default settings
    bool usePipeline = true; // Default to pipeline implementation

    // Default number of threads (typically matches the number of CPU cores)
    size_t numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        // If hardware_concurrency() fails, use 4 as a reasonable default
        numThreads = 4;
    }

    // Define the long options
    struct option long_options[] = {
        {"pipeline", no_argument, 0, 'p'},
        {"leader-followers", no_argument, 0, 'l'},
        {"threads", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command line options
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "plt:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                usePipeline = true;
                break;
            case 'l':
                usePipeline = false;
                break;
            case 't':
                {
                    int t = std::atoi(optarg);
                    if (t > 0) {
                        numThreads = t;
                    } else {
                        std::cerr << "Invalid thread count. Using default: " << numThreads << std::endl;
                    }
                }
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    // Create the appropriate server
    try {
        if (usePipeline) {
            std::cout << "Starting MST server with Pipeline implementation..." << std::endl;
            MSTPipelineServer server;
            serverPtr = &server;
            server.start();
        } else {
            std::cout << "Starting MST server with Leader/Followers implementation using "
                      << numThreads << " threads..." << std::endl;
            MSTLeaderFollowersServer server(numThreads);
            serverPtr = &server;
            server.start();
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}