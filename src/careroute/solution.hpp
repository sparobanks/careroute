// solution.hpp
//
// A plan, and how good it is.
//
// A Solution gives each carer an ordered list of visits (a Route), and lists any
// visits we could not place. evaluate() walks each route minute by minute to
// work out travel, lateness and overtime, then turns those into a single cost
// using the Weights. Lower cost is better.
//
// Most rules here are "soft": breaking them is allowed but adds to the cost.
// That is how real planners work, because a perfect plan rarely exists, and it
// lets us trade one goal off against another by changing the weights. The one
// "hard" rule is skills: a carer is never given a visit they are not trained for.
#pragma once

#include "careroute/model.hpp"

#include <vector>

namespace careroute {

// How much each thing matters. Change these to change what "good" means
// (for example raise `continuity` to keep clients with the same carer).
struct Weights {
    double travel = 1.0;          // per minute of travel
    double unassigned = 1000.0;   // per visit left uncovered (kept high)
    double lateness = 10.0;       // per minute a visit starts after its window
    double overtime = 5.0;        // per minute a carer works beyond allowed
    double continuity = 50.0;     // per visit not done by its preferred carer
    double balance = 0.5;         // per minute of spread between carer workloads
};

// One carer's day: the visits they do, in order. Indices point into
// Instance.visits; `carer` is an index into Instance.carers.
struct Route {
    int carer = -1;
    std::vector<int> visits;
};

// The numbers behind a solution's cost, kept so we can report and check them.
struct Metrics {
    double travel = 0.0;
    int unassigned = 0;
    double lateness = 0.0;
    double overtime = 0.0;
    int continuity_violations = 0;
    double balance = 0.0;
    double cost = 0.0;
    bool skills_ok = true;
};

struct Solution {
    std::vector<Route> routes;    // one per carer, same index as Instance.carers
    std::vector<int> unassigned;  // visit indices not placed
    Metrics metrics;              // filled by evaluate()
};

// Start from nothing: an empty route per carer, every visit unassigned.
Solution empty_solution(const Instance& inst);

// The timing of a single route.
struct RouteEval {
    double travel = 0.0;    // minutes driving (including the trip back to base)
    double lateness = 0.0;  // minutes started after the time window
    double overtime = 0.0;  // minutes worked beyond the carer's allowed time
    double work = 0.0;      // total minutes from shift start to getting home
    bool skills_ok = true;  // false if a visit needs a skill the carer lacks
};
RouteEval evaluate_route(const Instance& inst, const Route& r);

// Score the whole solution, fill sol.metrics, and return the cost.
double evaluate(const Instance& inst, Solution& sol, const Weights& w);

}  // namespace careroute
