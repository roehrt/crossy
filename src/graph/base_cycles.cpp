#include "graph.hpp"

#include <chrono>
#include <random>
#include <ranges>

vvi graph::base_cycles(const std::vector<std::vector<bool>> &graph, int max_cnt) {
    const int n = SZ(graph);

    std::vector stems(n, vvi(n));

    vvi cycles;
    REP(i, 0, n) {
        REP(j, 0, n) {
            if (i == j || !graph[i][j])
                continue;
            for (int k = 0; k < n; k++) {
                if (i == k || j == k || !graph[j][k])
                    continue;
                if (graph[k][i] && i < j && j < k)
                    cycles.push_back({i, j, k});
                stems[i][k].push_back(j);
            }
        }
    }

    if (SZ(cycles) >= max_cnt) {
        auto rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
        std::shuffle(std::ranges::begin(cycles), std::ranges::end(cycles), rng);
        cycles.resize(max_cnt);
        return cycles;
    }

    REP(i, 0, n) {
        REP(j, i + 1, n) {
            for (auto u : stems[i][j])
                for (auto v : stems[j][i]) {
                    cycles.push_back({i, u, j, v});
                    if (SZ(cycles) == max_cnt)
                        return cycles;
                }
        }
    }
    return cycles;
}
