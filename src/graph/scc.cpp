#include <vector>

#include "../common/macros.hpp"
#include "graph.hpp"

int graph::scc(const vvi &adj, vi &comp) {
    vi val, z;
    int Time, ncomps;
    std::function<int(int)> dfs = [&](int j) {
        int low = val[j] = ++Time, x;
        z.push_back(j);
        for (auto e : adj[j])
            if (comp[e] < 0)
                low = std::min(low, val[e] ?: dfs(e));

        if (low == val[j]) {
            do {
                x = z.back();
                z.pop_back();
                comp[x] = ncomps;
            } while (x != j);
            ncomps++;
        }
        return val[j] = low;
    };
    int n = SZ(adj);
    val.assign(n, 0);
    comp.assign(n, -1);
    Time = ncomps = 0;
    for (int i = 0; i < n; i++)
        if (comp[i] < 0)
            dfs(i);
    return ncomps;
}

int graph::sorted_scc(const std::vector<std::vector<int>> &adj, vi &comp) {
    int n = SZ(adj);
    vi res;
    int ncomps = scc(adj, res);
    std::vector<std::vector<int>> condensation(ncomps);
    for (int i = 0; i < n; i++)
        for (auto e : adj[i])
            if (res[i] != res[e])
                condensation[res[i]].push_back(res[e]);
    auto topo = *graph::topological_sort(condensation);
    vi inv(ncomps);
    for (int i = 0; i < ncomps; i++)
        inv[topo[i]] = i;
    comp.resize(n);
    for (int i = 0; i < n; i++)
        comp[i] = inv[res[i]];
    return ncomps;
}
