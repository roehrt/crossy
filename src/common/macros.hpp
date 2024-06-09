#pragma once

#include <functional>
#include <vector>

#define ALL(x) std::begin(x), std::end(x)
#define SZ(x) static_cast<int>(std::size(x))
#define REP(i, a, b) for (int i = (a); i < (b); i++)

#ifdef LARGE_WEIGHTS
using crint = long long;
constexpr crint oo = 1e18;
#else
using crint = int;
constexpr crint oo = 1e9;
#endif
using vi = std::vector<int>;
using vvi = std::vector<std::vector<int>>;
using cmatrix = std::vector<std::vector<crint>>;
