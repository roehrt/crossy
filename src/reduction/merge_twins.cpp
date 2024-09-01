#include "reduction.hpp"

#include <algorithm>
#include <map>

vi reduction::merge_twins(const instance &inst, const slvr &solve) {
    std::map<std::vector<int>, std::vector<int>> twins;
    REP(v, 0, inst.n1)
    twins[inst.neighbors[v]].push_back(v);
    instance reduced(inst.n0, SZ(twins));
    vvi partitions;
    partitions.reserve(SZ(twins));
    for (auto &[neighbors, vertices] : twins)
        partitions.push_back(vertices);
    REP(u, 0, reduced.n1) {
        for (int v : partitions[u])
            reduced.neighbors[u].insert(reduced.neighbors[u].end(), ALL(inst.neighbors[v]));
        for (int v : reduced.neighbors[u])
            reduced.back_neighbors[v].push_back(u);
    }
    REP(i, 0, reduced.n0) std::sort(ALL(reduced.back_neighbors[i]));
    REP(i, 0, reduced.n1) std::sort(ALL(reduced.neighbors[i]));
    vi p = solve(reduced);
    vi sol;
    sol.reserve(inst.n1);
    for (int u : p)
        sol.insert(sol.end(), ALL(partitions[u]));
    return sol;
}
