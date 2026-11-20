// solution.cpp  -  implementations for solution.hpp
#include "careroute/solution.hpp"

#include <algorithm>

namespace careroute {

Solution empty_solution(const Instance& inst) {
    Solution sol;
    sol.routes.resize(inst.n_carers());
    for (int c = 0; c < inst.n_carers(); ++c) {
        sol.routes[c].carer = c;
        sol.routes[c].visits.clear();
    }
    sol.unassigned.clear();
    for (int v = 0; v < inst.n_visits(); ++v)
        sol.unassigned.push_back(v);
    return sol;
}

RouteEval evaluate_route(const Instance& inst, const Route& r) {
    RouteEval e;
    if (r.carer < 0 || r.visits.empty())
        return e;

    const Carer& c = inst.carers[r.carer];
    double t = c.shift_start;      // current clock time
    double px = c.x, py = c.y;     // current location, starting at base

    for (int vi : r.visits) {
        const Visit& v = inst.visits[vi];
        if (!c.has_skill(v.skill))
            e.skills_ok = false;

        const double tr = inst.travel_min(px, py, v.x, v.y);
        e.travel += tr;
        const double arrive = t + tr;
        // We may arrive early and wait; starting after tw_end is lateness.
        const double start = std::max(arrive, static_cast<double>(v.tw_start));
        if (start > v.tw_end)
            e.lateness += start - v.tw_end;
        t = start + v.duration;    // finish this visit
        px = v.x;
        py = v.y;
    }

    // Trip back to base counts as travel and as working time.
    const double back = inst.travel_min(px, py, c.x, c.y);
    e.travel += back;
    t += back;

    e.work = t - c.shift_start;
    const double allowed = std::min(static_cast<double>(c.shift_end - c.shift_start),
                                    static_cast<double>(c.max_minutes));
    e.overtime = std::max(0.0, e.work - allowed);
    return e;
}

double evaluate(const Instance& inst, Solution& sol, const Weights& w) {
    Metrics m;
    std::vector<double> work(inst.n_carers(), 0.0);

    for (const Route& r : sol.routes) {
        const RouteEval e = evaluate_route(inst, r);
        m.travel += e.travel;
        m.lateness += e.lateness;
        m.overtime += e.overtime;
        if (!e.skills_ok) m.skills_ok = false;
        if (r.carer >= 0) work[r.carer] = e.work;

        // Continuity: did each visit get the carer the client asked for?
        for (int vi : r.visits) {
            const int pref = inst.visits[vi].preferred_carer;
            if (pref != -1 && inst.carers[r.carer].id != pref)
                ++m.continuity_violations;
        }
    }

    m.unassigned = static_cast<int>(sol.unassigned.size());

    // Balance: the gap between the busiest and the quietest carer.
    if (!work.empty()) {
        const double hi = *std::max_element(work.begin(), work.end());
        const double lo = *std::min_element(work.begin(), work.end());
        m.balance = hi - lo;
    }

    m.cost = w.travel * m.travel
           + w.unassigned * m.unassigned
           + w.lateness * m.lateness
           + w.overtime * m.overtime
           + w.continuity * m.continuity_violations
           + w.balance * m.balance;
    if (!m.skills_ok)
        m.cost += 1e9;  // should never happen; a guard against bad moves

    sol.metrics = m;
    return m.cost;
}

}  // namespace careroute
