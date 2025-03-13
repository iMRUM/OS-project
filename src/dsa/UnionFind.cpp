#include "../../include/dsa/UnionFind.hpp"

UnionFind::UnionFind(int _n) : n(_n), cc(_n) {
    parent.resize(n);
    rank.resize(n, 0);

    // Initialize each element as its own parent
    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }
}

int UnionFind::find_parent(int node) {
    // Path compression: make every node point directly to the root
    if (parent[node] != node) {
        parent[node] = find_parent(parent[node]);
    }
    return parent[node];
}

bool UnionFind::unite(int x, int y) {
    int root_x = find_parent(x);
    int root_y = find_parent(y);

    // Already in the same set
    if (root_x == root_y) {
        return false;
    }

    // Union by rank: attach smaller rank tree under root of higher rank tree
    if (rank[root_x] < rank[root_y]) {
        parent[root_x] = root_y;
    }
    else if (rank[root_x] > rank[root_y]) {
        parent[root_y] = root_x;
    }
    else {
        // Same rank, make one a child of the other and increment its rank
        parent[root_y] = root_x;
        rank[root_x]++;
    }

    // Decrease the count of connected components
    cc--;

    return true;
}