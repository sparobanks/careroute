// construct.hpp
//
// Building a first plan: greedy insertion.
//
// We take the visits that are not yet placed, deal with the most awkward ones
// first (the tightest time windows), and drop each into the single cheapest
// legal spot across all carers. "Cheapest" means the smallest increase in the
// overall cost. A carer is only legal for a visit if they hold the right skill.
// If no carer has the skill, the visit is left unassigned (and the cost carries
// a heavy penalty for that).
//
// The same insertion step is reused by the LNS later to rebuild a plan after
// part of it has been torn out, so it lives in its own file.
#pragma once

#include "careroute/model.hpp"
#include "careroute/solution.hpp"

namespace careroute {

// Where to put a visit, and what it would cost us.
struct Insertion {
    int carer = -1;                 // which carer's route
    int pos = -1;                   // index in that route to insert before
    double delta = 1e18;            // change in total cost (smaller is better)
    bool found() const { return carer >= 0; }
};

// Best legal place to insert visit `vi` into the current solution.
Insertion best_insertion(const Instance& inst, const Weights& w,
                         const Solution& sol, int vi);

// Place every currently-unassigned visit, greedily, tightest window first.
void insert_unassigned_greedy(const Instance& inst, const Weights& w, Solution& sol);

// Build a whole plan from scratch.
Solution greedy_construct(const Instance& inst, const Weights& w);

// A simple baseline: send each visit to the skilled carer whose last stop is
// nearest, in visit-id order, with no attempt to optimise. This is roughly what
// a basic "assign to the closest free worker" rule does, and it is what the
// smarter methods are measured against.
Solution naive_construct(const Instance& inst, const Weights& w);

}  // namespace careroute
