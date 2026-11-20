// lns.cpp  -  implementations for lns.hpp
#include "careroute/lns.hpp"

#include "careroute/construct.hpp"
#include "careroute/localsearch.hpp"
#include "careroute/rng.hpp"

#include <cmath>
#include <vector>

namespace careroute {

// Pull `q` randomly chosen assigned visits out of the plan and onto the
// unassigned list. This is the "ruin" step.
static void ruin(const Instance& inst, Solution& sol, int q, Rng& rng) {
    // Collect all (carer, position) pairs currently holding a visit.
    std::vector<std::pair<int, int>> placed;
    for (int c = 0; c < inst.n_carers(); ++c)
        for (int i = 0; i < static_cast<int>(sol.routes[c].visits.size()); ++i)
            placed.emplace_back(c, i);

    q = std::min<int>(q, static_cast<int>(placed.size()));
    // Pick q of them without repeats (partial Fisher-Yates shuffle).
    for (int k = 0; k < q; ++k) {
        const int r = rng.range(k, static_cast<int>(placed.size()) - 1);
        std::swap(placed[k], placed[r]);
    }

    // Record which visit ids to remove, then strip them out (back to front per
    // route so positions stay valid).
    std::vector<int> remove_ids;
    for (int k = 0; k < q; ++k)
        remove_ids.push_back(sol.routes[placed[k].first].visits[placed[k].second]);

    for (int c = 0; c < inst.n_carers(); ++c) {
        auto& vis = sol.routes[c].visits;
        for (int i = static_cast<int>(vis.size()) - 1; i >= 0; --i) {
            const int vid = vis[i];
            for (int rid : remove_ids) {
                if (vid == rid) {
                    vis.erase(vis.begin() + i);
                    sol.unassigned.push_back(vid);
                    break;
                }
            }
        }
    }
}

Solution solve(const Instance& inst, const Weights& w, const LnsParams& p) {
    Rng rng(p.seed);

    Solution cur = greedy_construct(inst, w);
    local_search(inst, w, cur);
    double cur_cost = cur.metrics.cost;

    Solution best = cur;
    double best_cost = cur_cost;

    // Geometric cooling from start_temp to end_temp over the run.
    const double cooling =
        (p.iterations > 1)
            ? std::pow(p.end_temp / p.start_temp, 1.0 / (p.iterations - 1))
            : 1.0;
    double temp = p.start_temp;

    for (int it = 0; it < p.iterations; ++it) {
        Solution cand = cur;
        const int q = rng.range(p.min_remove, p.max_remove);
        ruin(inst, cand, q, rng);
        insert_unassigned_greedy(inst, w, cand);
        if (p.ls_every > 0 && (it % p.ls_every == 0))
            local_search(inst, w, cand, 20);
        const double cand_cost = evaluate(inst, cand, w);

        // Accept better plans always; accept worse ones with a cooling chance.
        const double delta = cand_cost - cur_cost;
        if (delta < 0.0 || rng.unit() < std::exp(-delta / std::max(1e-9, temp))) {
            cur = std::move(cand);
            cur_cost = cand_cost;
        }
        if (cur_cost < best_cost) {
            best = cur;
            best_cost = cur_cost;
        }
        temp *= cooling;
    }

    // Polish the best plan once more.
    local_search(inst, w, best);
    evaluate(inst, best, w);
    return best;
}

}  // namespace careroute
