# First create a library target for your core code
add_library(dsa_lib
        src/dsa/Graph.cpp
        src/dsa/MST.cpp
        include/dsa/AbstractProductAlgo.hpp
        include/dsa/ConcreteAlgoPrim.hpp
        include/dsa/ConcreteAlgoKruskal.hpp
        include/dsa/ConcreteAlgoFactory.hpp
        include/dsa/UnionFind.hpp
        src/dsa/UnionFind.cpp
        src/dsa/UnionFind.cpp
        include/dsa/UnionFind.hpp
        include/server/server.hpp
        src/server/server.cpp
        src/dsa/ConcreteAlgoFactory.cpp
        src/dsa/ConcreteAlgoKruskal.cpp
        src/dsa/ConcreteAlgoPrim.cpp
)

# Set include directories for the library
target_include_directories(dsa_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add header files for IDE organization (doesn't affect compilation)
target_sources(dsa_lib
        PRIVATE
        include/dsa/Graph.hpp
        include/dsa/MST.hpp
        include/Factory/AbstractProduct.hpp
        include/Factory/AbstractFactory.hpp
)

# Create a test executable for Graph tests
add_executable(graph_tests
        tests/dsa/Graph_test.cpp
)

# Link the test executable with your library and test framework
target_link_libraries(graph_tests
        PRIVATE
        dsa_lib
)

# Create a test executable for MST tests
add_executable(mst_tests
        tests/dsa/MST_test.cpp
)

# Link the test executable with your library and test framework
target_link_libraries(mst_tests
        PRIVATE
        dsa_lib
)

# Register the tests with CTest
include(CTest)
add_test(NAME GraphTests COMMAND graph_tests)
add_test(NAME MSTTests COMMAND mst_tests)

# Optionally add a "run all tests" convenience target
add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS graph_tests mst_tests
)