#include "reduction.hpp"

vi reduction::isolated(const instance &inst, const slvr &solve) {
    std::vector<int> isolated, remaining;
    REP(v, 0, inst.n1) {
        if (inst.neighbors[v].empty())
            isolated.push_back(v);
        else
            remaining.push_back(v);
    }
    vi p = subgraph(inst, remaining, solve);
    p.insert(p.end(), ALL(isolated));
    return p;
}
