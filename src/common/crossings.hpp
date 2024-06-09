#pragma once

#include "instance.hpp"
#include "macros.hpp"

struct crossings {
    static cmatrix matrix(const instance &inst);
    static crint count(const cmatrix &C, const vi &p);
    static crint count(const instance &inst, const vi &p);
    static crint lower(const cmatrix &C);
};
