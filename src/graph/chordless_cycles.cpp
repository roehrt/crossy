#include "graph.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>

// Ported from
// https://networkx.org/documentation/stable/_modules/networkx/algorithms/cycles.html#chordless_cycles
struct nx_graph {
    std::unordered_map<int, std::unordered_set<int>> pred, succ;
    std::unordered_set<int> nodes;
    void add_edge(int u, int v) {
        pred[v].insert(u);
        succ[u].insert(v);
        nodes.insert(u);
        nodes.insert(v);
    }
    bool has_edge(int u, int v) const { return succ.at(u).contains(v); }
    auto build_nei() const {
        std::unordered_map<int, std::unordered_set<int>> nei;
        for (int u : nodes) {
            for (int v : succ.at(u))
                nei[u].insert(v);
            for (int v : pred.at(u))
                nei[u].insert(v);
        }
        return nei;
    }
    void remove_node(int v) {
        for (int u : pred[v])
            succ[u].erase(v);
        for (int w : succ[v])
            pred[w].erase(v);
        pred.erase(v);
        succ.erase(v);
        nodes.erase(v);
    }
    nx_graph() = default;
    explicit nx_graph(const std::vector<std::vector<bool>> &graph) {
        int n = SZ(graph);
        for (int i = 0; i < n; i++)
            nodes.insert(i);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (graph[i][j])
                    add_edge(i, j);
    }
    auto operator[](int v) const {
        if (!succ.contains(v))
            return std::unordered_set<int>();
        return succ.at(v);
    }

    std::vector<std::tuple<int, int, int, bool>> stem(int v) const {
        std::vector<std::tuple<int, int, int, bool>> res;
        if (!pred.contains(v) || !succ.contains(v))
            return res;
        for (auto u : pred.at(v))
            for (auto w : succ.at(v))
                if (!has_edge(u, w))
                    res.emplace_back(u, v, w, has_edge(w, u));
        return res;
    }
    nx_graph subgraph(const std::unordered_set<int> &nodes) const {
        nx_graph res;
        res.nodes = nodes;
        for (int u : nodes)
            for (int v : succ.at(u))
                if (nodes.contains(v))
                    res.add_edge(u, v);
        return res;
    }
};

std::vector<std::unordered_set<int>> nx_scc(const nx_graph &graph) {
    std::vector<int> z;
    std::unordered_map<int, int> comp, val;
    int Time, ncomps;
    std::function<int(int)> dfs = [&](int j) {
        int low = val[j] = ++Time, x;
        z.push_back(j);
        for (auto e : graph[j])
            if (!comp.contains(e))
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
    Time = ncomps = 0;
    for (int i : graph.nodes)
        if (!comp.contains(i))
            dfs(i);
    std::vector<std::unordered_set<int>> res(ncomps);
    for (int i : graph.nodes)
        res[comp[i]].insert(i);
    return res;
}

void chordless_search(const nx_graph &graph, std::vector<int> path, int max_length,
                      std::vector<std::vector<int>> &cycles,
                      const std::unordered_map<int, std::unordered_set<int>> &nei) {
    std::unordered_map<int, int> blocked;
    int target = path.front();
    blocked[path[1]] = 1;
    for (int i = 1; i < std::ssize(path); i++) {
        int w = path[i];
        for (int v : nei.at(w))
            blocked[v]++;
    }

    std::function<void(int)> dfs = [&](int v) {
        for (int w : graph.succ.at(v)) {
            if (blocked[w] == 1 && ssize(path) < max_length) {
                if (graph.succ.at(w).contains(target)) {
                    path.push_back(w);
                    cycles.push_back(path);
                    path.pop_back();
                } else {
                    if (nei.at(w).contains(target))
                        continue;
                    for (int u : nei.at(w))
                        blocked[u]++;
                    path.push_back(w);
                    dfs(w);
                    for (int u : nei.at(path.back()))
                        blocked[u]--;
                    path.pop_back();
                }
            }
        }
    };

    dfs(path.back());
}
vvi triangles(const std::vector<std::vector<bool>> &graph) {
    int n = SZ(graph);
    vvi res;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            for (int k = j + 1; k < n; k++)
                if (graph[i][j] && graph[j][k] && graph[k][i])
                    res.push_back({i, j, k});
    return res;
}

vvi graph::chordless_cycles(const std::vector<std::vector<bool>> &graph, int max_length) {
    if (max_length < 3)
        return {};
    const int n = SZ(graph);
    if (max_length == 3)
        return triangles(graph);
    if (max_length <= 4) {
        vvi cycles;
        std::vector stems(n, vvi(n));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j || !graph[i][j])
                    continue;
                for (int k = 0; k < n; k++) {
                    if (i == k || j == k || !graph[j][k])
                        continue;
                    if (graph[k][i] && i < j && j < k)
                        cycles.push_back({i, j, k});
                    if (!graph[i][k])
                        stems[i][k].push_back(j);
                }
            }
        }
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                for (auto u : stems[i][j])
                    for (auto v : stems[j][i])
                        if (!graph[u][v] && !graph[v][u] && i < u && u < j && j < v)
                            cycles.push_back({i, u, j, v});
        return cycles;
    }
    std::vector<std::vector<int>> cycles;
    nx_graph G(graph);
    auto sccs = nx_scc(G);
    while (!sccs.empty()) {
        auto scc = sccs.back();
        sccs.pop_back();
        if (scc.size() < 3)
            continue;
        nx_graph H = G.subgraph(scc);
        auto vv = *scc.begin();
        auto nei = H.build_nei();
        for (auto [u, v, w, is_triangle] : H.stem(vv)) {
            if (is_triangle) {
                cycles.push_back({u, v, w});
                continue;
            }

            chordless_search(H, {u, v, w}, max_length, cycles, nei);
        }
        H.remove_node(vv);
        auto new_sccs = nx_scc(H);
        for (const auto &new_scc : new_sccs)
            if (new_scc.size() > 2)
                sccs.push_back(new_scc);
    }
    return cycles;
}
