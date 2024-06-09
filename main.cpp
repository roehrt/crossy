#include <iostream>

#include "src/common/instance.hpp"
#include "src/exact/kobayashi_tamaki.hpp"
#include "src/heuristic/heuristic.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    instance inst;
    std::cin >> inst;
    auto res = exact::solve(inst);

    auto heuristic = crossings::count(inst, heuristic::quick(inst));
    auto exact = crossings::count(inst, res);
    if (!(exact <= heuristic)) return 0xBAD;

    REP(i, 0, inst.n1)
        std::cout << res[i]+inst.n0+1 << "\n";
    return 0;
}
