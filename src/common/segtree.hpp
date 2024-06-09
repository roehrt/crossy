#pragma once

#include "macros.hpp"

struct Tree {
    typedef crint T;
    static constexpr T unit = 0;
    static T f(T a, T b) { return a + b; }
    std::vector<T> s;
    int n;
    explicit Tree(int n = 0, T def = unit) : s(2 * n, def), n(n) {}
    void update(int pos) {
        for (++s[pos += n]; pos /= 2;)
            s[pos] = f(s[pos * 2], s[pos * 2 + 1]);
    }
    T query(int b, int e) {
        T ra = unit, rb = unit;
        for (b += n, e += n; b < e; b /= 2, e /= 2) {
            if (b % 2)
                ra = f(ra, s[b++]);
            if (e % 2)
                rb = f(s[--e], rb);
        }
        return f(ra, rb);
    }
};
