#include <numeric>
#include <vector>

#include "../common/instance.hpp"
#include "heuristic.hpp"

vi heuristic::median(const instance &inst) {
    vi medians(inst.n1);
    REP(v, 0, inst.n1) {
        auto neighbors = inst.neighbors[v];
        if (neighbors.empty()) {
            medians[v] = 0;
            continue;
        }
        auto median_iterator = neighbors.begin() + (neighbors.size() - 1) / 2;
        std::nth_element(neighbors.begin(), median_iterator, neighbors.end());
        medians[v] = *median_iterator;
    }

    vi p(inst.n1);
    std::iota(ALL(p), 0);
    std::sort(ALL(p), [&](int u, int v) { return medians[u] < medians[v]; });

    return p;
}
