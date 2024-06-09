#pragma once

#include "../common/instance.hpp"
#include "../common/macros.hpp"
#include <cassert>
#include <optional>

struct graph {
    static vvi chordless_cycles(const std::vector<std::vector<bool>> &graph, int max_length = 1e9);
    static vvi base_cycles(const std::vector<std::vector<bool>> &graph, int max_cnt = 1e9);
    static int scc(const vvi &graph, vi &comp);
    static int sorted_scc(const vvi &graph, vi &comp);
    static std::optional<vi> topological_sort(const vvi &graph);
    template <class T> static std::optional<vi> topological_sort_matrix(const std::vector<std::vector<T>> &graph);
    static int pathwidth(const instance &inst);
    template <class T> static vvi matrix_to_list(const std::vector<std::vector<T>> &matrix);
};
