CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
INCLUDES = -I.

# Source directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests

# Source files for implementation
GRAPH_SRC = $(SRC_DIR)/dsa/Graph.cpp
MST_SRC = $(SRC_DIR)/dsa/MST.cpp
UNIONFIND_SRC = $(SRC_DIR)/dsa/UnionFind.cpp
ALGO_PRIM_SRC = $(SRC_DIR)/dsa/ConcreteAlgoPrim.cpp
ALGO_KRUSKAL_SRC = $(SRC_DIR)/dsa/ConcreteAlgoKruskal.cpp
ALGO_FACTORY_SRC = $(SRC_DIR)/dsa/ConcreteAlgoFactory.cpp
ABSTRACT_PRODUCT_ALGO_SRC = src/dsa/AbstractProductAlgo.cpp
ABSTRACT_PRODUCT_SRC = src/Factory/AbstractProduct.cpp

# Test files
GRAPH_TEST = $(TEST_DIR)/dsa/Graph_test.cpp
MST_TEST = $(TEST_DIR)/dsa/MST_test.cpp
FACTORY_ALGO_TEST = $(TEST_DIR)/dsa/Factory_Algo_test.cpp

# Target executables
GRAPH_TEST_EXE = graph_test
MST_TEST_EXE = mst_test
FACTORY_ALGO_TEST_EXE = factory_algo_test

# Default rule
all: $(GRAPH_TEST_EXE) $(MST_TEST_EXE) $(FACTORY_ALGO_TEST_EXE)

# Create necessary directories
$(ABSTRACT_PRODUCT_SRC):
	@mkdir -p src/Factory
	@echo "#include \"../../include/Factory/AbstractProduct.hpp\"\n\nAbstractProduct::~AbstractProduct() {}" > $@

$(ABSTRACT_PRODUCT_ALGO_SRC):
	@mkdir -p src/dsa
	@echo "#include \"../../include/dsa/AbstractProductAlgo.hpp\"\n\nMST* AbstractProductAlgo::execute(Graph &graph) {\n    return nullptr; // Default implementation\n}\n\nAbstractProductAlgo::~AbstractProductAlgo() {}" > $@

# Rule for Graph test
$(GRAPH_TEST_EXE): $(GRAPH_TEST) $(GRAPH_SRC) | $(ABSTRACT_PRODUCT_SRC) $(ABSTRACT_PRODUCT_ALGO_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Rule for MST test
$(MST_TEST_EXE): $(MST_TEST) $(MST_SRC) $(GRAPH_SRC) | $(ABSTRACT_PRODUCT_SRC) $(ABSTRACT_PRODUCT_ALGO_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Rule for Factory and Algorithms test
$(FACTORY_ALGO_TEST_EXE): $(FACTORY_ALGO_TEST) $(GRAPH_SRC) $(MST_SRC) $(UNIONFIND_SRC) $(ALGO_PRIM_SRC) $(ALGO_KRUSKAL_SRC) $(ALGO_FACTORY_SRC) $(ABSTRACT_PRODUCT_ALGO_SRC) $(ABSTRACT_PRODUCT_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Create the Factory_Algo_test.cpp file
$(FACTORY_ALGO_TEST):
	@mkdir -p $(TEST_DIR)/dsa
	@cp Factory_Algo_test.cpp $@

clean:
	rm -f $(GRAPH_TEST_EXE) $(MST_TEST_EXE) $(FACTORY_ALGO_TEST_EXE)
	rm -f $(FACTORY_ALGO_TEST)
	rm -f $(ABSTRACT_PRODUCT_SRC) $(ABSTRACT_PRODUCT_ALGO_SRC)

run-all: all
	@echo "\n=== Running Graph Tests ===\n"
	./$(GRAPH_TEST_EXE)
	@echo "\n=== Running MST Tests ===\n"
	./$(MST_TEST_EXE)
	@echo "\n=== Running Factory and Algorithm Tests ===\n"
	./$(FACTORY_ALGO_TEST_EXE)

.PHONY: all clean run-all