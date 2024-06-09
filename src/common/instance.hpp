#pragma once

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <vector>

#include "macros.hpp"

struct instance {
    int n0 = 0, n1 = 0;
    vvi neighbors, back_neighbors;
    instance() = default;
    instance(int n0, int n1);
};

using slvr = std::function<vi(const instance &)>;

std::istream &operator>>(std::istream &is, instance &inst);
