#include "../common/crossings.hpp"
#include "../graph/graph.hpp"
#include "../heuristic/heuristic.hpp"
#include "../reduction/reduction.hpp"
#include "exact.hpp"

vi exact::solve(const instance &inst) {
    return reduction::isolated(inst, [](const instance &inst) {
        return reduction::merge_twins(inst, [](const instance &inst) {
            return reduction::components(inst, [](const instance &inst) {
                return reduction::merge_twins(inst, [](const instance &inst) {
                    try {
                        if (int pw = graph::pathwidth(inst); pw < 40 && (1ll << pw) * inst.n1 <= crint(5e5))
                            if (auto ans = exact::kobayashi_tamaki(inst); !ans.empty())
                                return ans;
                    } catch (const std::exception &e) {
                    }
                    auto ans = exact::maxsat(inst);
                    if (ans.empty() || crossings::count(inst, ans) > crossings::count(inst, heuristic::quick(inst)))
                        exit(42);
                    return ans;
                });
            });
        });
    });
}
