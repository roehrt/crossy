#pragma once

#include <chrono>

#include "../common/instance.hpp"
#include "../common/macros.hpp"

struct exact {
    static vi kobayashi_tamaki(const instance &inst);
    static vi maxsat(const instance &inst);
    static vi solve(const instance &inst);
};
