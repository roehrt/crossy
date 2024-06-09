#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <queue>

#include "../common/crossings.hpp"
#include "../common/instance.hpp"
#include "../common/macros.hpp"
#include "exact.hpp"

struct event {
    int y, x;
    bool is_leaf, is_deletion;

    [[nodiscard]] int partition() const { return is_leaf ? 1 : is_deletion ? 0 : 2; }

    bool operator<(const event &rhs) const {
        if (x != rhs.x)
            return x < rhs.x;
        if (partition() != rhs.partition())
            return partition() < rhs.partition();
        if (y != rhs.y)
            return y < rhs.y;
        return is_deletion < rhs.is_deletion;
    }
};

// Kobayashi, Y., & Tamaki, H. (2014). A Fast and Simple Subexponential Fixed Parameter Algorithm for One-Sided Crossing
// Minimization. Algorithmica, 72(3), 778–790. https://doi.org/10.1007/s00453-014-9872-x
vi exact::kobayashi_tamaki(const instance &inst) {
#ifdef LARGE_WEIGHTS
    using crossings_t = uint64_t;
#else
    using crossings_t = uint32_t;
#endif
    const cmatrix C = crossings::matrix(inst);
    vi solution;

    std::vector<event> events;
    REP(i, 0, inst.n1) {
        const auto &adj = inst.neighbors[i];
        if (adj.empty()) {
            solution.emplace_back(i);
            continue;
        }
        std::vector<int> adj_ant(ALL(adj));
        bool is_leaf = std::adjacent_find(ALL(adj_ant), std::not_equal_to()) == adj_ant.end();
        events.emplace_back(i, *std::min_element(ALL(adj)), is_leaf, false);
        events.emplace_back(i, *std::max_element(ALL(adj)), is_leaf, true);
    }
    sort(ALL(events));

    const int T = std::ssize(events);
    std::vector<int> depth(T);
    std::vector<bool> is_introduce(T);
    std::vector<std::vector<int>> interface(T);
    for (int t = 0; t < T; t++) {
        auto [y, x, is_leaf, is_deletion] = events[t];
        is_introduce[t] = !is_deletion;
        if (!is_introduce[t])
            continue;
        for (int tt = t; events[tt].y != y || tt == t; tt++)
            interface[tt].push_back(y);
    }
    for (int t = 0; t < T; t++)
        depth[t] = std::ssize(interface[t]);

    // Can be optimized to use only O(T * pathwidth) memory
    std::vector cL(T, std::vector<crossings_t>(inst.n1));
    for (int t = 1; t < T; t++) {
        int y = events[t].y;
        if (is_introduce[t]) {
            for (int v : interface[t])
                cL[t][v] = cL[t - 1][v];
            cL[t][y] = 0;
        } else
            for (int v : interface[t])
                cL[t][v] = cL[t - 1][v] + C[y][v];
    }

    auto insert_bit = [](int S, int i) {
        int pS = (S >> i) << (i + 1);
        pS |= (S & ((1 << i) - 1));
        pS |= 1 << i;
        return pS;
    };

    auto subset_crossings = [&](int t) {
        std::vector cS(depth[t], std::vector<crossings_t>(1 << depth[t]));
        for (int j = 0; j < depth[t]; j++) {
            for (int S = 0; S < 1 << depth[t]; S++) {
                crossings_t sum = 0;
                for (int i = 0; i < depth[t]; i++)
                    if (S & (1 << i))
                        sum += C[interface[t][i]][interface[t][j]];
                cS[j][S] = sum;
            }
        }
        return cS;
    };

    auto solve_interval = [&](int l, int r, int &state) {
        std::vector<std::vector<crossings_t>> opt(T);
        for (int t = 0; t < r; t++) {
            int y = events[t].y;
            if (!is_introduce[t]) {
                opt[t].resize(1 << depth[t]);
                int Y = int(std::find(ALL(interface[t - 1]), y) - interface[t - 1].begin());
                for (int S = 0; S < 1 << depth[t]; S++)
                    opt[t][S] = opt[t - 1][insert_bit(S, Y)];
                if (t - 1 < l)
                    opt[t - 1].clear();
                continue;
            }

            if (t != 0) {
                opt[t] = opt[t - 1];
                if (t - 1 < l)
                    opt[t - 1].clear();
            }
            opt[t].resize(1 << depth[t]);
            std::vector cS = subset_crossings(t);
            for (int S = 1 << (depth[t] - 1); S < 1 << depth[t]; S++) {
                crossings_t mn = std::numeric_limits<crossings_t>::max();
                for (int i = 0; i < depth[t]; i++) {
                    int I = 1 << i;
                    if (!(S & I))
                        continue;
                    crossings_t crossings = opt[t][S ^ I] + cL[t][interface[t][i]] + cS[i][S ^ I];
                    mn = std::min(mn, crossings);
                }
                opt[t][S] = mn;
            }
        }
        vi solution;
        for (int t = r - 1; t >= l; t--) {
            if (!is_introduce[t]) {
                int y = int(std::find(ALL(interface[t - 1]), int(events[t].y)) - interface[t - 1].begin());
                state = insert_bit(state, y);
                continue;
            }
            int y = int(std::find(ALL(interface[t]), int(events[t].y)) - interface[t].begin());
            std::vector cS = subset_crossings(t);
            while (state & (1 << y))
                for (int i = 0; i < depth[t]; i++) {
                    int I = 1 << i;
                    if (!(state & I))
                        continue;
                    crossings_t crossings = opt[t][state ^ I] + cL[t][interface[t][i]] + cS[i][state ^ I];
                    if (crossings == opt[t][state]) {
                        solution.emplace_back(interface[t][i]);
                        state ^= I;
                        break;
                    }
                }
        }
        std::reverse(ALL(solution));
        return solution;
    };

    int pathwidth = *std::max_element(ALL(depth));
    if (pathwidth > 30)
        return {};

    const long long MEMLIMIT = 7ll << 30; // 7 GiB
    std::vector<std::pair<int, int>> partitions;
    for (int l = 0; l < T;) {
        int r = l;
        long long memory_partition = 0;
        while (r < T && memory_partition + (2ll << depth[r]) <= MEMLIMIT)
            memory_partition += static_cast<long long>(sizeof(crossings_t)) << depth[r++];
        partitions.emplace_back(l, r);
        if (r <= l)
            return {};
        l = r;
    }

    std::ranges::reverse(partitions);
    for (int S = 0; auto [l, r] : partitions) {
        auto ans = solve_interval(l, r, S);
        solution.insert(solution.begin(), ALL(ans));
    }
    return solution;
#undef all
}
