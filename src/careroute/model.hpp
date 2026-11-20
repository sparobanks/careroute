// model.hpp
//
// The problem, written down as plain data.
//
// CareRoute solves a Home Health Care Routing and Scheduling Problem (HHCRSP):
// we have a set of carers (the workers) and a set of visits (one per client
// appointment). We must decide which carer does each visit and in what order,
// so that skills match, time windows are respected, nobody works too long, and
// total travel is small. This header just defines the data; the rules for
// scoring a plan live in solution.hpp.
//
// Times are integer minutes from midnight, so 8:00 is 480 and 17:00 is 1020.
// Locations are (x, y) in kilometres, and travel time is worked out from the
// straight-line distance and a travel speed. A real system would use road
// travel times instead; swapping that in only changes this one function.
#pragma once

#include <vector>

namespace careroute {

// A carer: where they start, what they can do, and when they are available.
struct Carer {
    int id = 0;                 // stable id (we keep ids equal to the index)
    double x = 0.0, y = 0.0;    // base/home location in km
    std::vector<int> skills;    // skills this carer holds
    int shift_start = 0;        // earliest they can start (minutes from midnight)
    int shift_end = 0;          // latest they can finish
    int max_minutes = 0;        // most minutes they may work in the shift

    bool has_skill(int s) const;
};

// A visit: where it is, what it needs, and when it should happen.
struct Visit {
    int id = 0;
    double x = 0.0, y = 0.0;    // location in km
    int skill = 0;              // skill the carer must hold
    int duration = 0;          // how long the visit takes (minutes)
    int tw_start = 0;          // earliest good start time (soft)
    int tw_end = 0;            // latest good start time (soft)
    int preferred_carer = -1;  // carer id the client prefers, or -1 for none
};

// A whole instance: the carers, the visits, and the travel speed.
struct Instance {
    double speed_kmh = 30.0;
    std::vector<Carer> carers;
    std::vector<Visit> visits;

    int n_carers() const { return static_cast<int>(carers.size()); }
    int n_visits() const { return static_cast<int>(visits.size()); }

    // Travel time in minutes between two points, from distance and speed.
    double travel_min(double x1, double y1, double x2, double y2) const;
    double travel_min(const Carer& c, const Visit& v) const;
    double travel_min(const Visit& a, const Visit& b) const;
};

}  // namespace careroute
