#include "../common/instance.hpp"
#include "reduction.hpp"
#include <ranges>
#include <vector>

vi reduction::subgraph(const instance &inst, const vi &vertices, const slvr &solve) {
    if (SZ(vertices) <= 1)
        return vertices;
    std::vector<bool> mask(inst.n1, false);
    for (int i : vertices)
        mask[i] = true;
    vi new_left(inst.n0, -1);
    int n0 = 0;
    int prev = -1;
    REP(i, 0, inst.n0) {
        const auto &back_neighbors = inst.back_neighbors[i];
        auto filtered = back_neighbors | std::views::filter([&](int v) { return mask[v]; });
        bool all_equal = std::ranges::adjacent_find(filtered, std::ranges::not_equal_to()) == filtered.end();
        if (filtered.empty())
            continue;
        if (all_equal) {
            if (prev != filtered.front())
                new_left[i] = n0++;
            else
                new_left[i] = n0 - 1;
            prev = filtered.front();
        } else {
            prev = -1;
            new_left[i] = n0++;
        }
    }
    instance reduced(n0, SZ(vertices));
    std::vector new_right(inst.n1, -1);
    vi old_right;
    for (int i : vertices) {
        new_right[i] = SZ(old_right);
        old_right.push_back(i);
    }
    for (int i : vertices) {
        for (int j : inst.neighbors[i]) {
            reduced.neighbors[new_right[i]].push_back(new_left[j]);
            reduced.back_neighbors[new_left[j]].push_back(new_right[i]);
        }
    }
    REP(i, 0, reduced.n0) std::sort(ALL(reduced.back_neighbors[i]));
    REP(i, 0, reduced.n1) std::sort(ALL(reduced.neighbors[i]));
    vi p = solve(reduced);
    std::transform(ALL(p), p.begin(), [&](int u) { return old_right[u]; });
    return p;
}
