// rng.hpp  -  one small random-number helper, so results are reproducible.
#pragma once

#include <random>

namespace careroute {

class Rng {
public:
    explicit Rng(unsigned seed = 0) : gen_(seed) {}

    // Integer in [lo, hi] inclusive.
    int range(int lo, int hi) {
        std::uniform_int_distribution<int> d(lo, hi);
        return d(gen_);
    }

    // Real in [0, 1).
    double unit() {
        std::uniform_real_distribution<double> d(0.0, 1.0);
        return d(gen_);
    }

    std::mt19937& engine() { return gen_; }

private:
    std::mt19937 gen_;
};

}  // namespace careroute
