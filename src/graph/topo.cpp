#include <algorithm>
#include <vector>

#include "../common/macros.hpp"
#include "graph.hpp"

std::optional<vi> graph::topological_sort(const vvi &adj) {
    int n = SZ(adj), i = 0;
    vi in(n), topo(n, -1);
    for (const auto &row : adj)
        for (const auto &node : row)
            in[node]++;
    for (int j = 0; j < n; j++)
        if (!in[j])
            topo[i++] = j;
    for (const auto &node : topo) {
        if (node == -1)
            return std::nullopt;
        for (int next : adj[node])
            if (!--in[next])
                topo[i++] = next;
    }
    return topo;
}

template std::optional<vi> graph::topological_sort_matrix(const cmatrix &matrix);
template std::optional<vi> graph::topological_sort_matrix(const std::vector<std::vector<bool>> &matrix);

template <class T> std::optional<vi> graph::topological_sort_matrix(const std::vector<std::vector<T>> &adj) {
    int n = SZ(adj), i = 0;
    vi in(n), topo(n, -1);
    for (const auto &row : adj)
        for (int j = 0; j < n; j++)
            in[j] += !!row[j];
    for (int j = 0; j < n; j++)
        if (!in[j])
            topo[i++] = j;
    for (const auto &node : topo) {
        if (node == -1)
            return std::nullopt;
        for (int next = 0; next < n; next++)
            if (adj[node][next] && !--in[next])
                topo[i++] = next;
    }
    return topo;
}
