#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../../include/dsa/Graph.hpp"

TEST_CASE("Graph constructors") {
    SUBCASE("Default constructor") {
        Graph g;
        CHECK_EQ(g.getVertices(), 0);
        CHECK(g.isEmpty());
    }
    
    SUBCASE("Constructor with vertices and edges") {
        Graph g(5, 0);
        CHECK_EQ(g.getVertices(), 5);
        CHECK_FALSE(g.isEmpty());
    }
    
    SUBCASE("Constructor with existing adjacency list") {
        adjList list(3);
        list[0].push_back({1, 5});  // Edge from 0 to 1 with weight 5
        list[0].push_back({2, 3});  // Edge from 0 to 2 with weight 3
        list[1].push_back({2, 2});  // Edge from 1 to 2 with weight 2
        
        Graph g(list);
        CHECK_EQ(g.getVertices(), 3);
        CHECK_FALSE(g.isEmpty());
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 2);
        CHECK_EQ(graph[1].size(), 1);
        CHECK_EQ(graph[2].size(), 0);
    }
}

TEST_CASE("Adding edges to graph") {
    SUBCASE("Add valid edges") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        g.addEdge(0, 2, 3);
        g.addEdge(1, 2, 2);
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 2);
        CHECK_EQ(graph[1].size(), 1);
        CHECK_EQ(graph[2].size(), 0);
        
        // Check weights
        CHECK_EQ(graph[0][0].second, 5);
        CHECK_EQ(graph[0][1].second, 3);
        CHECK_EQ(graph[1][0].second, 2);
    }
    
    SUBCASE("Update existing edge") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        
        // Update the edge with a new weight
        g.addEdge(0, 1, 10);
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 1);  // Still only one edge
        CHECK_EQ(graph[0][0].second, 10);  // Weight updated to 10
    }
    
    SUBCASE("Add edges with invalid vertices") {
        Graph g(3, 0);
        g.addEdge(-1, 1, 5);  // Invalid source
        g.addEdge(0, -1, 5);  // Invalid target
        g.addEdge(3, 1, 5);   // Invalid source (out of range)
        g.addEdge(0, 3, 5);   // Invalid target (out of range)
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 0);  // No edges should be added
    }
}

TEST_CASE("Removing edges from graph") {
    SUBCASE("Remove existing edge") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        g.addEdge(0, 2, 3);
        g.addEdge(1, 2, 2);
        
        g.removeEdge(0, 1);
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 1);
        CHECK_EQ(graph[0][0].first, 2);  // Only the edge to vertex 2 remains
    }
    
    SUBCASE("Remove non-existing edge") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        
        g.removeEdge(1, 0);  // This edge doesn't exist
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 1);  // Original edge still exists
        CHECK_EQ(graph[1].size(), 0);  // No edges from vertex 1
    }
    
    SUBCASE("Remove edge with invalid vertices") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        
        g.removeEdge(-1, 1);  // Invalid source
        g.removeEdge(0, -1);  // Invalid target
        g.removeEdge(3, 1);   // Invalid source (out of range)
        g.removeEdge(0, 3);   // Invalid target (out of range)
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 1);  // Original edge should still exist
    }
}

TEST_CASE("Utility methods") {
    SUBCASE("isEmpty method") {
        Graph g1;
        CHECK(g1.isEmpty());
        
        Graph g2(3, 0);
        CHECK_FALSE(g2.isEmpty());
        
        Graph g3(0, 0);
        CHECK(g3.isEmpty());
    }
    
    SUBCASE("getVertices method") {
        Graph g1;
        CHECK_EQ(g1.getVertices(), 0);
        
        Graph g2(5, 0);
        CHECK_EQ(g2.getVertices(), 5);
    }
    
    SUBCASE("getGraph method") {
        Graph g(3, 0);
        g.addEdge(0, 1, 5);
        g.addEdge(0, 2, 3);
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph.size(), 3);
        CHECK_EQ(graph[0].size(), 2);
        CHECK_EQ(graph[1].size(), 0);
        CHECK_EQ(graph[2].size(), 0);
    }
}

TEST_CASE("Complex graph operations") {
    SUBCASE("Build and modify a complex graph") {
        Graph g(5, 0);
        
        // Add a series of edges
        g.addEdge(0, 1, 5);
        g.addEdge(0, 2, 3);
        g.addEdge(1, 2, 2);
        g.addEdge(1, 3, 6);
        g.addEdge(2, 3, 7);
        g.addEdge(2, 4, 4);
        g.addEdge(3, 4, 1);
        
        const auto& graph = g.getGraph();
        CHECK_EQ(graph[0].size(), 2);
        CHECK_EQ(graph[1].size(), 2);
        CHECK_EQ(graph[2].size(), 2);
        CHECK_EQ(graph[3].size(), 1);
        CHECK_EQ(graph[4].size(), 0);
        
        // Update some edges
        g.addEdge(0, 1, 10);  // Update weight
        g.addEdge(3, 4, 8);   // Update weight
        
        CHECK_EQ(graph[0][0].second, 10);  // Updated weight
        CHECK_EQ(graph[3][0].second, 8);   // Updated weight
        
        // Remove some edges
        g.removeEdge(0, 2);
        g.removeEdge(2, 4);
        
        CHECK_EQ(graph[0].size(), 1);
        CHECK_EQ(graph[2].size(), 1);
    }
}