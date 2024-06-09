#pragma once

#include <chrono>

#include "../common/instance.hpp"
#include "../common/macros.hpp"

struct exact {
    static vi kobayashi_tamaki(const instance &inst);
    static vi maxsat(const instance &inst,
                     std::optional<std::chrono::time_point<std::chrono::steady_clock>> timeout = std::nullopt);
    static vi solve(const instance &inst);
};
