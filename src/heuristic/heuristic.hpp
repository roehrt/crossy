#pragma once

#include <algorithm>
#include <chrono>

#include "../common/instance.hpp"
#include "../common/macros.hpp"

struct heuristic {
    static void greedy_switch(vi &p, const cmatrix &C);
    static vi barycenter(const instance &inst);
    static vi median(const instance &inst);
    static vi quick(const instance &inst);
};
