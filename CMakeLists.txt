cmake_minimum_required(VERSION 3.29)
project(os_project)

set(CMAKE_CXX_STANDARD 20)

# Create a test executable for Graph tests
add_executable(graph_tests
        tests/dsa/Graph_test.cpp
        tests/dsa/Factory_Algo_test.cpp
        src/dsa/Graph.cpp
        src/dsa/MST.cpp
        include/dsa/ConcreteAlgoPrim.hpp
        include/dsa/ConcreteAlgoKruskal.hpp
        include/dsa/UnionFind.hpp
        src/dsa/UnionFind.cpp
        src/dsa/UnionFind.cpp
        include/dsa/UnionFind.hpp
        include/server/Server.hpp
        src/dsa/ConcreteAlgoKruskal.cpp
        src/dsa/ConcreteAlgoPrim.cpp
        include/server/MSTServer.hpp
        include/active_object/MSTProxy.hpp
        include/active_object/MethodRequest.hpp
        include/active_object/ActivationQ.hpp
        include/active_object/Scheduler.hpp
        include/active_object/MSTServant.hpp
        include/active_object/Future.hpp
        include/factory/ConcreteAlgoFactory.hpp
        include/factory/AbstractProductAlgo.hpp
        src/active_object/MSTServant.cpp
        src/factory/AbstractProduct.cpp
        include/active_object/MSTPipeline.hpp
        include/commands.hpp
        src/active_object/ActivationQ.cpp
        src/active_object/MSTPipeline.cpp
        src/active_object/MSTProxy.cpp
        include/server/MSTPipelineServer.hpp
        include/server/MSTPipelineServer.hpp
        include/server/MSTLeaderFollowersServer.hpp
        include/leader_followers/LFThreadPool.hpp
        include/leader_followers/Reactor.hpp
        src/leader_followers/LFThreadPool.cpp
        src/leader_followers/Reactor.cpp
        src/server/MSTServerLF.cpp
        src/server/MSTServerPipeline.cpp
)
