#ifndef MSTPIPELINESERVER_HPP
#define MSTPIPELINESERVER_HPP
#include "MSTServer.hpp"
#include "../factory/ConcreteAlgoFactory.hpp"
#include "../active_object/MSTPipeline.hpp"

class MSTPipelineServer : public MSTServer {
private:
    std::vector<std::thread> client_threads;
    std::unique_ptr<MSTPipeline> mstPipeline;

    void stop() override;

    void handleClient(ClientData *client_data) override;

    void setupSocket() override;

    void processCommand(int socket_fd, std::string &command) override;

public:
    MSTPipelineServer(): MSTServer() {
    }

    void start() override;
};
#endif //MSTPIPELINESERVER_HPP
