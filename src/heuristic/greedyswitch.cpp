#include "heuristic.hpp"

// TODO: swap argument order ?
void heuristic::greedy_switch(vi &p, const cmatrix &C) {
    for (int i = 0; i < SZ(p) - 1; i++)
        if (C[p[i + 1]][p[i]] < C[p[i]][p[i + 1]])
            std::swap(p[i + 1], p[i]), i = std::max(i - 2, -1);
}
