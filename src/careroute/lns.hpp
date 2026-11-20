// lns.hpp
//
// Large Neighbourhood Search: the method that actually finds good plans.
//
// Local search gets stuck in the first decent plan it reaches. LNS gets past
// that by repeatedly "ruining" part of the plan (pulling a handful of visits
// out) and then "recreating" it (greedily putting them back). Because the
// visits usually go back in different places, this explores big jumps that
// small edits cannot reach.
//
// A new plan is kept if it is better. Sometimes a slightly worse plan is kept
// too, with a probability that shrinks as the search cools down (simulated
// annealing). That keeps the search from getting trapped early. The best plan
// ever seen is remembered and returned.
#pragma once

#include "careroute/model.hpp"
#include "careroute/solution.hpp"

namespace careroute {

struct LnsParams {
    int iterations = 3000;     // how many ruin-and-recreate rounds
    int min_remove = 2;        // fewest visits pulled out per round
    int max_remove = 12;       // most visits pulled out per round
    int ls_every = 20;         // run local search every N rounds (it is costly)
    double start_temp = 30.0;  // starting "temperature" for acceptance
    double end_temp = 0.05;    // ending temperature
    unsigned seed = 0;
    bool verbose = false;
};

// Solve an instance. Returns the best plan found, with metrics filled in.
Solution solve(const Instance& inst, const Weights& w, const LnsParams& p);

}  // namespace careroute
