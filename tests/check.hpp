// check.hpp  -  a tiny test helper, so we need no external test library.
#pragma once

#include <cmath>
#include <cstdio>
#include <string>

namespace check {

inline int& failures() {
    static int f = 0;
    return f;
}

inline void expect(bool ok, const char* expr, const char* file, int line) {
    if (!ok) {
        std::printf("  FAIL: %s  (%s:%d)\n", expr, file, line);
        ++failures();
    }
}

inline void expect_near(double a, double b, double eps, const char* expr,
                        const char* file, int line) {
    if (std::fabs(a - b) > eps) {
        std::printf("  FAIL: %s  (%.4f vs %.4f)  (%s:%d)\n", expr, a, b, file, line);
        ++failures();
    }
}

}  // namespace check

#define CHECK(cond) ::check::expect((cond), #cond, __FILE__, __LINE__)
#define CHECK_NEAR(a, b, eps) \
    ::check::expect_near((a), (b), (eps), #a " ~= " #b, __FILE__, __LINE__)
