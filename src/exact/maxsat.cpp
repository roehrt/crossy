#include "../common/crossings.hpp"
#include "../common/penalty_graph.hpp"
#include "../graph/graph.hpp"
#include "EvalMaxSAT.h"
#include "exact.hpp"

namespace {
using vvb = std::vector<std::vector<bool>>;

vvb transitive_closure(vvb g) {
    int n = std::ssize(g);
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                g[i][j] = g[i][j] || (g[i][k] && g[k][j]);
    return g;
}
vvb matmul(const vvb &a, const vvb &b) {
    int n = std::ssize(a);
    vvb c(n, std::vector<bool>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                c[i][j] = c[i][j] || (a[i][k] && b[k][j]);
    return c;
}
vvb transitive_reduction(const vvb &g) {
    auto closure = transitive_closure(g);
    auto prod = matmul(g, closure);
    vvb red(g);
    for (int i = 0; i < std::ssize(g); i++)
        for (int j = 0; j < std::ssize(g); j++)
            red[i][j] = g[i][j] && !prod[i][j];
    return red;
}

struct OnlineCycleDetection {
    int n;
    struct ring_buffer {
        unsigned N;
        std::vector<uint16_t> data;
        int head, tail;
        explicit ring_buffer(unsigned n) {
            n = std::max(n, 1u << 10);
            N = std::bit_ceil(n);
            data = std::vector<uint16_t>(N);
            --N;
            head = tail = 0;
        }
        void clear() { head = tail = 0; }
        void push(uint16_t x) {
            data[tail++] = x;
            tail &= N;
        }
        uint16_t pop() {
            uint16_t x = data[head++];
            head &= N;
            return x;
        }
        [[nodiscard]] inline bool empty() const { return head == tail; }
    };
    ring_buffer hq;
    std::vector<std::vector<uint16_t>> g;
    std::vector<std::pair<uint16_t, uint16_t>> history;
    std::vector<uint16_t> history_e;
    std::vector<std::pair<uint32_t, uint32_t>> commits;
    std::vector<uint16_t> L, par;
    std::vector<uint32_t> par_time;
    uint32_t mtime = 0;

    OnlineCycleDetection(const vvb &adj, const vvb &fixed)
        : n(std::ssize(adj)), hq(n * n), g(n), L(n), par(n), par_time(n) {
        history.reserve(n * n + 100);
        history_e.reserve(n * n + 100);
        vvb is_spine(n, std::vector<bool>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                is_spine[i][j] = fixed[i][j];

        auto red = transitive_reduction(is_spine);

        vvi gT(n, std::vector<int>());
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (red[i][j]) {
                    g[i].emplace_back(j);
                    gT[j].emplace_back(i);
                }

        std::vector<bool> visited(n);
        std::function<void(int)> dfs = [&](int u) {
            visited[u] = true;
            int depth = 0;
            for (int v : gT[u]) {
                if (!visited[v])
                    dfs(v);
                depth = std::max(depth, L[v] + 1);
            }
            L[u] = depth;
        };
        for (int i = 0; i < n; i++)
            if (!visited[i])
                dfs(i);
    }

    auto naive_bfs(int s, int t) {
        std::vector<bool> vis(n);
        std::vector path(n, -1);
        std::queue<int> q;
        q.push(s);
        vis[s] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : g[u]) {
                if (!vis[v]) {
                    vis[v] = true;
                    path[v] = u;
                    q.push(v);
                }
            }
        }
        std::vector<std::vector<int>> out(1);
        for (int u = t; u != -1; u = path[u])
            out.back().emplace_back(u);
        std::reverse(out.back().begin(), out.back().end());
        out.back().emplace_back(s);
        return out;
    }

    inline void set(uint16_t x, uint16_t y) {
        history.emplace_back(x, L[x]);
        L[x] = y;
    }
    bool prop(uint16_t s, uint16_t t) {
        mtime++;
        if (L[s] < L[t])
            return false;
        set(t, L[s] + 1);
        hq.clear();
        hq.push(t);
        while (!hq.empty()) {
            auto u = hq.pop();
            for (uint16_t v : g[u]) {
                if (L[u] < L[v])
                    continue;
                if (par_time[v] != mtime)
                    par[v] = u, par_time[v] = mtime;
                if (v == s)
                    return true;
                set(v, L[u] + 1);
                hq.push(v);
            }
        }
        return false;
    }

    std::optional<vvi> add_edge(int a, int b) {
        auto time = history.size();
        if (prop(a, b)) {
            uint16_t v = a;
            vi cycle{b};
            while (v != b) {
                cycle.emplace_back(v);
                if (par_time[v] != mtime)
                    exit(42);
                v = par[v];
            }
            cycle.emplace_back(b);
            std::ranges::reverse(cycle);

            while (history.size() > time)
                L[history.back().first] = history.back().second, history.pop_back();
            return vvi{cycle};
        }
        g[a].emplace_back(b);
        history_e.emplace_back(a);
        return std::nullopt;
    }

    void rollback(int t) {
        auto [ht, het] = commits[t];
        commits.resize(t);
        while (history.size() > ht)
            L[history.back().first] = history.back().second, history.pop_back();
        while (history_e.size() > het)
            g[history_e.back()].pop_back(), history_e.pop_back();
    }

    void commit() { commits.emplace_back(history.size(), history_e.size()); }
};

class DAGPropagator : public EvalMaxSAT::ExternalPropagator {
  public:
    OnlineCycleDetection ocd;
    vvi c, var;
    std::vector<std::pair<int, int>> lookup;

    DAGPropagator(const vvb &adj, const vvb &fixed) : ocd(adj, fixed), var(size(adj), vi(size(adj))) {
        int n = std::ssize(adj);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (adj[i][j] && !fixed[i][j]) {
                    lookup.emplace_back(i, j);
                    var[i][j] = lookup.size();
                }
            }
        }
    }

    void notify_assignment(const vi &lits) override {
        for (const auto &lit : lits) {
            if (lit > 0)
                continue;
            if (!c.empty()) return;
            auto [u, v] = lookup[-lit - 1];
            if (auto cycle = ocd.add_edge(u, v)) {
                c.reserve(c.size() + cycle->size());
                std::ranges::move(*cycle, std::back_inserter(c));
                cycle->clear();
                return;
            }
        }
    }
    void notify_new_decision_level() override {
        ocd.commit();
    }
    void notify_backtrack(size_t new_level) override {
        ocd.rollback(new_level);
    }
    bool cb_has_external_clause(bool & is_forgettable) override {
        is_forgettable = true;
        return !c.empty();
    }
    int cb_decide() override { return 0; }
    int cb_propagate() override { return 0; }
    int cb_add_reason_clause_lit(int) override { return 0; }

    bool cb_check_found_model(const std::vector<int> &model) override {
        return true;
    }

    int cb_add_external_clause_lit() override {
        if (c.empty())
            return 0;
        auto &cycle = c.back();
        int lit = 0;
        while (!lit) {
            if (cycle.size() == 1) {
                c.pop_back();
                return 0;
            }
            lit = var[end(cycle)[-2]][cycle.back()];
            cycle.pop_back();
        }
        return lit;
    }
};

} // namespace

vi exact::maxsat(const instance &inst) {
    const cmatrix c = crossings::matrix(inst);
    cmatrix K = penalty_graph(inst, c, true);

    const int n = SZ(K);

    std::vector e2i(n, std::vector(n, -1));
    vvb adj(n, std::vector<bool>(n));
    vvb fixed(n, std::vector<bool>(n));

    EvalMaxSAT solver;
    solver.setTargetComputationTime(1800);
    int vars = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (0 < K[i][j]) {
                adj[i][j] = true;
                fixed[i][j] = K[i][j] >= oo;
                if (K[i][j] < oo) {
                    e2i[i][j] = solver.newVar();
                    ++vars;
                    assert(e2i[i][j] == vars);
                    solver.addClause({-e2i[i][j]}, K[i][j]);
                }
            }
    DAGPropagator propagator(adj, fixed);
    solver.connect_external_propagator(&propagator);
    assert(vars == solver.nVars());
    for (int v = 1; v <= vars; v++)
        solver.add_observed_var(v);

    vvi cycles = graph::base_cycles(adj, 5000);
    for (const auto &cycle : cycles) {
        vi clause;
        for (int i = 0; i < std::ssize(cycle); i++) {
            int u = cycle[i], v = cycle[(i + 1) % std::ssize(cycle)];
            if (int x = e2i[u][v]; x != -1)
                clause.push_back(x);
        }
        solver.addClause(clause);
    }

    if (!solver.solve())
        return {};
    int64_t cost = 0, reduced_cost = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (adj[i][j] && e2i[i][j] != -1 && solver.getValue(e2i[i][j])) {
                adj[i][j] = false;
                cost += c[i][j];
                reduced_cost += K[i][j];
            }

    auto topo = graph::topological_sort_matrix(adj);
    assert(topo.has_value());
    return *topo;
}
