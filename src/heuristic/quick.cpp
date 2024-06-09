#include "../common/crossings.hpp"
#include "heuristic.hpp"

vi heuristic::quick(const instance &inst) {
    auto barycenter = heuristic::barycenter(inst);
    auto median = heuristic::median(inst);
    std::vector solutions = {barycenter, median};
    return *std::ranges::min_element(solutions, {}, [&](const auto &p) { return crossings::count(inst, p); });
}
