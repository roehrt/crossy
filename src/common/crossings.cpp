#include "crossings.hpp"
#include "instance.hpp"
#include "segtree.hpp"
#include <algorithm>
#include <ranges>
#include <vector>

cmatrix crossings::matrix(const instance &inst) {
    cmatrix c(inst.n1, std::vector<crint>(inst.n1));
    std::vector<int> r(inst.n0);
    REP(i, 0, inst.n1) {
        const auto &adj = inst.neighbors[i];
        for (int u = 0, p = 0; u < inst.n0; ++u) {
            while (p < SZ(adj) && adj[p] <= u) ++p;
            r[u] = SZ(adj) - p;
        }
        REP(j, 0, inst.n1) {
            if (i == j)
                continue;
            for (int u : inst.neighbors[j])
                c[i][j] += r[u];
        }
    }
    return c;
}

crint crossings::count(const cmatrix &C, const vi &p) {
    crint cr = 0;
    REP(i, 0, SZ(p)) REP(j, i + 1, SZ(p)) cr += C[p[i]][p[j]];
    return cr;
}

crint crossings::lower(const cmatrix &C) {
    crint cr = 0;
    REP(i, 0, SZ(C)) REP(j, i + 1, SZ(C)) cr += std::min(C[i][j], C[j][i]);
    return cr;
}

crint crossings::count(const instance &inst, const vi &p) {
    crint cr = 0;
    Tree t(inst.n1);
    vi r(inst.n1);
    REP(i, 0, inst.n1) r[p[i]] = i;
    REP(i, 0, inst.n0) {
        auto nei = inst.back_neighbors[i] | std::views::transform([&](int v) { return r[v]; });
        auto nei_vec = std::vector<int>(ALL(nei));
        std::ranges::sort(nei_vec);
        for (int v : nei_vec) {
            cr += t.query(v + 1, inst.n1);
            t.update(v);
        }
    }
    return cr;
}
