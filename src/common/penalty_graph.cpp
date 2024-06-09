#include "macros.hpp"

#include "instance.hpp"
#include <algorithm>
#include <cassert>
#include <set>

namespace {
template <typename T> std::vector<T> operator-(const std::vector<T> &a, const std::vector<T> &b) {
    assert(SZ(a) == SZ(b));
    std::vector<T> res(SZ(a));
    REP(i, 0, SZ(a)) res[i] = a[i] - b[i];
    return res;
}

struct frac {
    long long num, den;

    frac(long long num, long long den) : num(num), den(den) {
        if (den < 0) {
            this->num = -this->num;
            this->den = -this->den;
        }
    }

    auto operator<=>(const frac &rhs) const { return num * rhs.den <=> rhs.num * den; }
};

/**
 * @brief Check if the LP Ax > 0 and x >= 0 is feasible
 *
 * @param A The matrix A
 * @return true if the LP is feasible, false otherwise
 *
 * @details Complexity: O(n)
 */
bool is_feasible(const std::array<std::vector<crint>, 2> &A) {
    bool feasible = false;
    auto [rowL, rowR] = A;
    frac a_lo(0, 1), a_hi(std::numeric_limits<int>::max(), 1);
    REP(i, 0, SZ(rowL)) {
        if (rowL[i] * rowR[i] >= 0) {
            if (rowL[i] > 0 || rowR[i] > 0) {
                feasible = true;
                break;
            }
            continue;
        }
        if (rowL[i] < 0 && rowR[i] > 0)
            a_lo = std::max(a_lo, frac(-rowR[i], rowL[i]));
        if (rowL[i] > 0 && rowR[i] < 0)
            a_hi = std::min(a_hi, frac(-rowR[i], rowL[i]));
    }
    return feasible || a_lo > a_hi;
}

/**
 * @brief Compute the center column of the tabular analysis of a < b
 *
 * @param a Adjacency list of a
 * @param b Adjacency list of b
 * @param vertices The set (not multiset) union of the adjacency lists of a and b
 * @return The center column of the tabular analysis
 *
 * @details Complexity: O(n)
 * For details regarding tabular analysis see: Fixed parameter algorithms for one-sided crossing minimization revisited,
 * https://doi.org/10.1016/j.jda.2006.12.008
 */
auto cost_vector(const std::vector<int> &a, const std::vector<int> &b, const std::vector<int> &vertices) {
    std::vector<crint> res;
    res.reserve(2 * SZ(vertices) + 1);
    int seenA = 0, seenB = 0;
    res.push_back(SZ(a));
    REP(i, 0, SZ(vertices)) {
        while (seenA < SZ(a) && a[seenA] <= vertices[i])
            ++seenA;
        while (seenB < SZ(b) && b[seenB] < vertices[i])
            ++seenB;
        res.push_back(seenB + SZ(a) - seenA);
        if (i == SZ(vertices) - 1)
            continue;
        int l = vertices[i], r = vertices[i + 1];
        while (seenA < SZ(a) && a[seenA] < r)
            ++seenA;
        while (seenB < SZ(b) && b[seenB] <= l)
            ++seenB;
        res.push_back(seenB + SZ(a) - seenA);
    }
    res.push_back(SZ(b));
    return res;
};
} // namespace


/**
 * @brief Compute the penalty graph of an instance
 *
 * @param inst The instance
 * @param C The crossing matrix
 * @param presolve Whether to apply presolve
 * @return cmatrix The penalty graph
 *
 * @details Complexity: O(nm) with presolve, O(n^2) without presolve
 */
cmatrix penalty_graph(const instance &inst, const cmatrix &C, bool presolve) {
    int n = SZ(C);
    cmatrix K(n, std::vector<crint>(n));
    REP(i, 0, n) {
        REP(j, 0, n) {
            if (C[i][j] == 0 && C[j][i] == 0)
                continue;
            if (C[i][j] == 0)
                K[i][j] = oo;
            else
                K[i][j] = std::max(crint(0), C[j][i] - C[i][j]);
        }
    }

    if (!presolve)
        return K;

    REP(a, 0, n) {
        REP(b, 0, n) {
            if (0 < C[a][b] && C[a][b] <= C[b][a]) {
                std::vector<int> nei_union;
                std::ranges::merge(inst.neighbors[a], inst.neighbors[b], std::back_inserter(nei_union));
                nei_union.erase(std::unique(ALL(nei_union)), nei_union.end());

                auto adj_a = inst.neighbors[a];
                auto adj_b = inst.neighbors[b];

                std::vector<crint> costsASB = cost_vector(adj_a, adj_b, nei_union);
                std::vector<crint> costsBSA = cost_vector(adj_b, adj_a, nei_union);

                std::vector<crint> costsSAB, costsABS;
                int seenA = 0, seenB = 0;
                costsSAB.push_back(0);
                REP(k, 0, SZ(nei_union)) {
                    while (seenA < SZ(adj_a) && adj_a[seenA] < nei_union[k])
                        ++seenA;
                    while (seenB < SZ(adj_b) && adj_b[seenB] < nei_union[k])
                        ++seenB;
                    costsSAB.push_back(seenA + seenB);
                    if (k == SZ(nei_union) - 1)
                        continue;
                    while (seenA < SZ(adj_a) && adj_a[seenA] <= nei_union[k])
                        ++seenA;
                    while (seenB < SZ(adj_b) && adj_b[seenB] <= nei_union[k])
                        ++seenB;
                    costsSAB.push_back(seenA + seenB);
                }
                costsSAB.push_back(SZ(adj_a) + SZ(adj_b));

                seenA = 0, seenB = 0;
                costsABS.push_back(SZ(adj_a) + SZ(adj_b));
                REP(k, 0, SZ(nei_union)) {
                    while (seenA < SZ(adj_a) && adj_a[seenA] <= nei_union[k])
                        ++seenA;
                    while (seenB < SZ(adj_b) && adj_b[seenB] <= nei_union[k])
                        ++seenB;
                    costsABS.push_back(SZ(adj_a) + SZ(adj_b) - seenA - seenB);
                    if (k == SZ(nei_union) - 1)
                        continue;
                    while (seenA < SZ(adj_a) && adj_a[seenA] < nei_union[k + 1])
                        ++seenA;
                    while (seenB < SZ(adj_b) && adj_b[seenB] < nei_union[k + 1])
                        ++seenB;
                    costsABS.push_back(SZ(adj_a) + SZ(adj_b) - seenA - seenB);
                }
                costsABS.push_back(0);
                if (std::ranges::any_of(costsASB - costsBSA, [](crint x) { return x > 0; }) &&
                    is_feasible({costsSAB - costsBSA, costsABS - costsBSA}))
                    continue;
                K[a][b] = oo;
                K[b][a] = 0;
            }
        }
    }

    std::vector reachability(n, std::vector<bool>(n));
    REP(i, 0, n) REP(j, 0, n) reachability[i][j] = K[i][j] >= oo;
    REP(k, 0, n)
    REP(i, 0, n) REP(j, 0, n) reachability[i][j] = reachability[i][j] || (reachability[i][k] && reachability[k][j]);

    REP(i, 0, n) {
        REP(j, 0, n) {
            if (!reachability[i][j])
                continue;
            if (K[j][i] >= oo) exit(100); // We commited a cycle
            assert(K[j][i] < oo);
            if (0 < K[i][j] && K[i][j] < oo) {
                K[i][j] = oo;
            } else if (K[j][i]) {
                K[i][j] = oo;
                K[j][i] = 0;
            }
        }
    }
    return K;
}
