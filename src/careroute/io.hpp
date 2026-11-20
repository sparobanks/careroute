// io.hpp
//
// Reading an instance from a file and writing a plan back out.
//
// The file format is deliberately plain text, one record per line, so both this
// program and the Python tools can read and write it without any library:
//
//   speed 30
//   carer <id> <x> <y> <shift_start> <shift_end> <max_minutes> <skills>
//   visit <id> <x> <y> <skill> <duration> <tw_start> <tw_end> <preferred_carer>
//
// skills is a comma list like 1,2 or - for none. Times are minutes from midnight.
#pragma once

#include "careroute/model.hpp"
#include "careroute/solution.hpp"

#include <string>
#include <vector>

namespace careroute {

Instance load_instance(const std::string& path);

// One scheduled stop: when the carer arrives, starts and finishes a visit.
struct Stop {
    int visit = -1;
    double arrive = 0.0;
    double start = 0.0;
    double finish = 0.0;
};
std::vector<Stop> schedule_route(const Instance& inst, const Route& r);

// Write the plan in a plain format the Python visualiser reads.
void write_solution(const std::string& path, const Instance& inst,
                    const Solution& sol);

}  // namespace careroute
