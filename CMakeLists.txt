cmake_minimum_required(VERSION 3.29)
project(os_assignments_master)

set(CMAKE_CXX_STANDARD 14)

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
        include/server/server.hpp
        src/server/server.cpp
        src/dsa/ConcreteAlgoKruskal.cpp
        src/dsa/ConcreteAlgoPrim.cpp
        include/server/MSTServer.hpp
        src/server/MSTServer.cpp
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
)
