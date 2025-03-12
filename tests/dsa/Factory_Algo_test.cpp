#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../../include/dsa/Graph.hpp"
#include "../../include/dsa/MST.hpp"
#include "../../include/dsa/ConcreteAlgoFactory.hpp"
#include "../../include/dsa/ConcreteAlgoKruskal.hpp"
#include "../../include/dsa/ConcreteAlgoPrim.hpp"

// Helper function to create a test graph
Graph createTestGraph() {
    Graph g(6, 0);
    
    // Add edges (undirected graph)
    g.addEdge(0, 1, 7);
    g.addEdge(1, 0, 7);
    
    g.addEdge(0, 2, 9);
    g.addEdge(2, 0, 9);
    
    g.addEdge(0, 5, 14);
    g.addEdge(5, 0, 14);
    
    g.addEdge(1, 2, 10);
    g.addEdge(2, 1, 10);
    
    g.addEdge(1, 3, 15);
    g.addEdge(3, 1, 15);
    
    g.addEdge(2, 3, 11);
    g.addEdge(3, 2, 11);
    
    g.addEdge(2, 5, 2);
    g.addEdge(5, 2, 2);
    
    g.addEdge(3, 4, 6);
    g.addEdge(4, 3, 6);
    
    g.addEdge(4, 5, 9);
    g.addEdge(5, 4, 9);
    
    return g;
}

TEST_CASE("Factory Pattern") {
    SUBCASE("Factory creates correct algorithm instances") {
        ConcreteAlgoFactory factory;
        
        // Create Prim's algorithm (ID: 0)
        AbstractProductAlgo* primAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(0));
        CHECK(primAlgo != nullptr);
        CHECK(dynamic_cast<ConcreteAlgoPrim*>(primAlgo) != nullptr);
        
        // Create Kruskal's algorithm (ID: 1)
        AbstractProductAlgo* kruskalAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(1));
        CHECK(kruskalAlgo != nullptr);
        CHECK(dynamic_cast<ConcreteAlgoKruskal*>(kruskalAlgo) != nullptr);
    }
    
    SUBCASE("Factory handles invalid IDs") {
        ConcreteAlgoFactory factory;
        
        // Check behavior with invalid ID
        CHECK_THROWS_AS(factory.createProduct(2), std::out_of_range);
    }
}

TEST_CASE("MST Algorithms") {
    Graph g = createTestGraph();
    ConcreteAlgoFactory factory;
    
    SUBCASE("Prim's Algorithm") {
        AbstractProductAlgo* primAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(0));
        MST* primMST = primAlgo->execute(g);
        
        CHECK(primMST != nullptr);
        CHECK_EQ(primMST->getTotalWeight(), 33);  // Expected MST weight
        
        // The MST should have exactly (V-1) edges
        CHECK_EQ(primMST->getEdges().size(), g.getVertices() - 1);
        
        delete primMST;
    }
    
    SUBCASE("Kruskal's Algorithm") {
        AbstractProductAlgo* kruskalAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(1));
        MST* kruskalMST = kruskalAlgo->execute(g);
        
        CHECK(kruskalMST != nullptr);
        CHECK_EQ(kruskalMST->getTotalWeight(), 33);  // Expected MST weight
        
        // The MST should have exactly (V-1) edges
        CHECK_EQ(kruskalMST->getEdges().size(), g.getVertices() - 1);
        
        delete kruskalMST;
    }
    
    SUBCASE("Both algorithms produce identical MST weight") {
        AbstractProductAlgo* primAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(0));
        MST* primMST = primAlgo->execute(g);
        
        AbstractProductAlgo* kruskalAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(1));
        MST* kruskalMST = kruskalAlgo->execute(g);
        
        CHECK_EQ(primMST->getTotalWeight(), kruskalMST->getTotalWeight());
        
        delete primMST;
        delete kruskalMST;
    }
    
    SUBCASE("Algorithms on empty graph") {
        Graph emptyGraph;
        
        AbstractProductAlgo* primAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(0));
        MST* primMST = primAlgo->execute(emptyGraph);
        
        AbstractProductAlgo* kruskalAlgo = static_cast<AbstractProductAlgo*>(factory.createProduct(1));
        MST* kruskalMST = kruskalAlgo->execute(emptyGraph);
        
        CHECK_EQ(primMST->getTotalWeight(), 0);
        CHECK_EQ(kruskalMST->getTotalWeight(), 0);
        
        delete primMST;
        delete kruskalMST;
    }
}