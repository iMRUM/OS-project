//
// Created by imry on 3/11/25.
//

#ifndef CONCRETEALGOPRIM_HPP
#define CONCRETEALGOPRIM_HPP
#define INF 0x3f3f3f3f
#include "AbstractProductAlgo.hpp"
struct Edge
{
    int w = INF, to = -1, id;
    bool operator<(Edge const& other) const
    {
        return make_pair(w, to) < make_pair(other.w, other.to);
    }
    Edge()
    {
        w = INF;
        to = -1;
    }
    Edge(int _w, int _to, int _id) : w(_w), to(_to), id(_id) {}
};
class ConcreteAlgoPrim : public AbstractProductAlgo {
private:
    vector<tuple<int, int, int, int>> _prim(const vector<vector<Edge>>& adj, int n);
    vector<tuple<int, int, int, int>> prim(const vector<tuple<int, int, int, int>>& edges, int n);

protected:
    MST * execute(Graph &graph) override;

public:
    ~ConcreteAlgoPrim() override = default;
};
#endif //CONCRETEALGOPRIM_HPP
