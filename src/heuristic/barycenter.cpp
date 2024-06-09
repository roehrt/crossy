#include <numeric>
#include <vector>

#include "../common/crossings.hpp"
#include "heuristic.hpp"

vi heuristic::barycenter(const instance &inst) {
    std::vector<double> barycenter(inst.n1);
    REP(u, 0, inst.n1) {
        if (inst.neighbors[u].empty()) {
            barycenter[u] = 0;
            continue;
        }
        for (int v : inst.neighbors[u])
            barycenter[u] += v;
        barycenter[u] /= static_cast<double>(inst.neighbors[u].size());
    }

    vi p(inst.n1);
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(), [&](int u, int v) { return barycenter[u] < barycenter[v]; });
    return p;
}
