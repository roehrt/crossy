#include <algorithm>
#include <numeric>
#include <vector>

#include "../common/instance.hpp"
#include "graph.hpp"

template vvi graph::matrix_to_list(const cmatrix &matrix);
template vvi graph::matrix_to_list(const std::vector<std::vector<bool>> &matrix);

template <class T> vvi graph::matrix_to_list(const std::vector<std::vector<T>> &matrix) {
    int n = SZ(matrix);
    vvi adj(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (matrix[i][j])
                adj[i].push_back(j);
    return adj;
}

int graph::pathwidth(const instance &inst) {
    vi depth(inst.n0);
    for (const auto &adj : inst.neighbors) {
        if (adj.empty())
            continue;
        auto [left, right] = std::ranges::minmax_element(adj);
        depth[*left]++;
        depth[*right]--;
    }
    std::partial_sum(ALL(depth), depth.begin());
    return *std::ranges::max_element(depth);
}
