// localsearch.hpp
//
// Tidying a plan with small changes.
//
// Local search keeps trying little edits and accepts any that lower the cost,
// until no edit helps. Three kinds of edit are used:
//   relocate  - take one visit out and put it in the best spot anywhere
//   swap      - exchange two visits between two carers
//   reverse   - flip a run of visits inside one carer's day (an "or-opt"/2-opt)
// This is "first improvement": we take the first edit that helps and start
// again, which is simple and fast. It gets us to a local best; the LNS on top
// is what escapes a local best to find something better.
#pragma once

#include "careroute/model.hpp"
#include "careroute/solution.hpp"

namespace careroute {

// Improve `sol` in place until no single edit helps (or max_passes is hit).
// Returns true if it changed anything. Assumes sol.metrics is current.
bool local_search(const Instance& inst, const Weights& w, Solution& sol,
                  int max_passes = 100);

}  // namespace careroute
