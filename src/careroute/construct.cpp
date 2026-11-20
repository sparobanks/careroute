// construct.cpp  -  implementations for construct.hpp
#include "careroute/construct.hpp"

#include <algorithm>

namespace careroute {

// The cost change from inserting one visit into one route counts only the parts
// that this move can change: that route's travel, lateness and overtime, the
// continuity penalty, and removing the visit from the unassigned list. Balance
// is left out of the decision (it is a small, whole-solution effect); the final
// evaluate() still scores it exactly.
static double route_delta(const Instance& inst, const Weights& w,
                          const Route& before, const Route& after,
                          int carer_id, const Visit& v) {
    const RouteEval a = evaluate_route(inst, before);
    const RouteEval b = evaluate_route(inst, after);
    double d = w.travel * (b.travel - a.travel)
             + w.lateness * (b.lateness - a.lateness)
             + w.overtime * (b.overtime - a.overtime);
    if (v.preferred_carer != -1 && carer_id != v.preferred_carer)
        d += w.continuity;
    d -= w.unassigned;  // this visit is no longer uncovered
    return d;
}

Insertion best_insertion(const Instance& inst, const Weights& w,
                         const Solution& sol, int vi) {
    Insertion best;
    const Visit& v = inst.visits[vi];

    for (int c = 0; c < inst.n_carers(); ++c) {
        const Carer& carer = inst.carers[c];
        if (!carer.has_skill(v.skill))
            continue;  // hard rule: skills must match

        const Route& cur = sol.routes[c];
        const int len = static_cast<int>(cur.visits.size());
        for (int pos = 0; pos <= len; ++pos) {
            Route trial = cur;
            trial.visits.insert(trial.visits.begin() + pos, vi);
            const double d = route_delta(inst, w, cur, trial, carer.id, v);
            if (d < best.delta) {
                best.delta = d;
                best.carer = c;
                best.pos = pos;
            }
        }
    }
    return best;
}

void insert_unassigned_greedy(const Instance& inst, const Weights& w, Solution& sol) {
    // Deal with the tightest time windows first: they are the hardest to fit.
    std::vector<int> order = sol.unassigned;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        const Visit& va = inst.visits[a];
        const Visit& vb = inst.visits[b];
        const int wa = va.tw_end - va.tw_start;
        const int wb = vb.tw_end - vb.tw_start;
        if (wa != wb) return wa < wb;
        return va.tw_start < vb.tw_start;
    });

    std::vector<int> still_unassigned;
    for (int vi : order) {
        const Insertion ins = best_insertion(inst, w, sol, vi);
        if (ins.found()) {
            Route& r = sol.routes[ins.carer];
            r.visits.insert(r.visits.begin() + ins.pos, vi);
        } else {
            still_unassigned.push_back(vi);  // no carer has the skill
        }
    }
    sol.unassigned = std::move(still_unassigned);
}

Solution greedy_construct(const Instance& inst, const Weights& w) {
    Solution sol = empty_solution(inst);
    insert_unassigned_greedy(inst, w, sol);
    evaluate(inst, sol, w);
    return sol;
}

Solution naive_construct(const Instance& inst, const Weights& w) {
    Solution sol = empty_solution(inst);
    std::vector<int> still_unassigned;

    for (int vi : sol.unassigned) {
        const Visit& v = inst.visits[vi];
        int best_c = -1;
        double best_d = 1e18;
        for (int c = 0; c < inst.n_carers(); ++c) {
            const Carer& carer = inst.carers[c];
            if (!carer.has_skill(v.skill))
                continue;
            // distance from the carer's last point to this visit
            double lx = carer.x, ly = carer.y;
            if (!sol.routes[c].visits.empty()) {
                const Visit& last = inst.visits[sol.routes[c].visits.back()];
                lx = last.x;
                ly = last.y;
            }
            const double d = inst.travel_min(lx, ly, v.x, v.y);
            if (d < best_d) {
                best_d = d;
                best_c = c;
            }
        }
        if (best_c >= 0)
            sol.routes[best_c].visits.push_back(vi);
        else
            still_unassigned.push_back(vi);
    }
    sol.unassigned = std::move(still_unassigned);
    evaluate(inst, sol, w);
    return sol;
}

}  // namespace careroute
