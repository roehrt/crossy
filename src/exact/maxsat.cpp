#include "../../external/uwrmaxsat/ipamir.h"
#include "../common/crossings.hpp"
#include "../common/penalty_graph.hpp"
#include "../graph/graph.hpp"
#include "exact.hpp"

namespace {
int should_terminate(void *data) {
    auto timeout = *static_cast<std::chrono::time_point<std::chrono::steady_clock> *>(data);
    return std::chrono::steady_clock::now() > timeout;
}
} // namespace

vi exact::maxsat(const instance &inst, std::optional<std::chrono::time_point<std::chrono::steady_clock>> timeout) {
    const cmatrix c = crossings::matrix(inst);
    auto K = penalty_graph(inst, c, true);
    std::string UWRFLAGS = "-mem-lim=7950 -no-par -lex-opt";
#if defined(SCIP_CPU) && SCIP_CPU > 0
    UWRFLAGS += " -scip-cpu=" + std::to_string(SCIP_CPU);
#ifndef SCIP_CPU_ADD
#define SCIP_CPU_ADD 0
#endif
    UWRFLAGS += " -scip-cpu-add=" + std::to_string(SCIP_CPU_ADD);
#else
    UWRFLAGS += " -no-scip";
#endif
    std::cerr << "UWRFLAGS: " << UWRFLAGS << std::endl;
    setenv("UWRFLAGS", UWRFLAGS.c_str(), 1);
    const int n = SZ(K);
    // TODO: Use std::vector<char> instead of bool*.
    bool *is_edge = new bool[n * n];
    bool *is_fixed = new bool[n * n];
    REP(i, 0, n) REP(j, 0, n) {
        is_edge[i * n + j] = 0 < K[i][j];
        is_fixed[i * n + j] = K[i][j] >= oo;
    }
    std::vector e2i(n, std::vector(n, -1));
    std::vector adj(n, std::vector<bool>(n));
    int32_t variable = 1;
    auto solver = ipamir_init({n, is_edge, is_fixed});
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (0 < K[i][j]) {
                adj[i][j] = true;
                if (K[i][j] < oo) {
                    e2i[i][j] = variable;
                    ipamir_add_soft_lit(solver, variable++, K[i][j]);
                }
            }
        }
    }
    if (timeout)
        ipamir_set_terminate(solver, &timeout.value(), should_terminate);

    bool first_run = true;
    auto adj_prime = adj;
    while (true) {
        vvi cycles;
        if (first_run) {
            cycles = graph::base_cycles(adj_prime, 50000);
            first_run = false;
        } else
            graph::chordless_cycles(adj_prime, 10);
        if (cycles.empty())
            cycles = graph::chordless_cycles(adj_prime);
        for (const auto &cycle : cycles) {
            for (int i = 0; i < std::ssize(cycle); i++) {
                int u = cycle[i], v = cycle[(i + 1) % std::ssize(cycle)];
                if (int x = e2i[u][v]; x != -1)
                    ipamir_add_hard(solver, x);
            }
            ipamir_add_hard(solver, 0);
        }
        auto ret = ipamir_solve(solver);
        if (ret == 0) {
            ipamir_release(solver);
            delete[] is_edge;
            delete[] is_fixed;
            return {};
        }
        if (!(ret == 30 || ret == 10))
            return {};

        adj_prime = adj;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (adj[i][j] && e2i[i][j] != -1 && ipamir_val_lit(solver, e2i[i][j]) > 0)
                    adj_prime[i][j] = false;
        if (auto topo = graph::topological_sort_matrix(adj_prime); topo) {
            ipamir_release(solver);
            delete[] is_edge;
            delete[] is_fixed;
            return *topo;
        }
    }
}
