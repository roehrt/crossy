#pragma once

#include "../common/instance.hpp"
#include "../common/macros.hpp"

struct reduction {
    static vi isolated(const instance &inst, const slvr &solve);
    static vi merge_twins(const instance &inst, const slvr &solve);
    static vi components(const instance &inst, const slvr &solve);
    static vi subgraph(const instance &inst, const vi &vertices, const slvr &solve);
};
