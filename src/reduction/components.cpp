#include "../common/crossings.hpp"
#include "../common/penalty_graph.hpp"
#include "../graph/graph.hpp"
#include "reduction.hpp"

vi reduction::components(const instance &inst, const slvr &solve) {
    cmatrix c = crossings::matrix(inst);
    vvi adj = graph::matrix_to_list(penalty_graph(inst, c, false));
    vi comp;
    int ncomps = graph::sorted_scc(adj, comp);
    vvi components(ncomps);
    REP(i, 0, SZ(comp)) components[comp[i]].push_back(i);
    vi p;
    for (const auto &component : components) {
        vi q = subgraph(inst, component, solve);
        p.insert(p.end(), ALL(q));
    }
    return p;
}
