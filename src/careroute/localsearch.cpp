// localsearch.cpp  -  implementations for localsearch.hpp
#include "careroute/localsearch.hpp"

#include "careroute/construct.hpp"

#include <algorithm>

namespace careroute {

// Try to relocate one visit to a better place. On the first improving move it
// applies it and returns true.
static bool try_relocate(const Instance& inst, const Weights& w, Solution& sol,
                         double& cur) {
    for (int c1 = 0; c1 < inst.n_carers(); ++c1) {
        const int len = static_cast<int>(sol.routes[c1].visits.size());
        for (int i = 0; i < len; ++i) {
            Solution trial = sol;
            const int vi = trial.routes[c1].visits[i];
            trial.routes[c1].visits.erase(trial.routes[c1].visits.begin() + i);

            const Insertion ins = best_insertion(inst, w, trial, vi);
            if (!ins.found())
                continue;
            trial.routes[ins.carer].visits.insert(
                trial.routes[ins.carer].visits.begin() + ins.pos, vi);

            const double cost = evaluate(inst, trial, w);
            if (cost + 1e-9 < cur) {
                sol = std::move(trial);
                cur = cost;
                return true;
            }
        }
    }
    return false;
}

// Swap two visits between (or within) routes, keeping skills legal.
static bool try_swap(const Instance& inst, const Weights& w, Solution& sol,
                     double& cur) {
    for (int c1 = 0; c1 < inst.n_carers(); ++c1) {
        for (int i = 0; i < static_cast<int>(sol.routes[c1].visits.size()); ++i) {
            for (int c2 = c1; c2 < inst.n_carers(); ++c2) {
                const int jstart = (c2 == c1) ? i + 1 : 0;
                for (int j = jstart; j < static_cast<int>(sol.routes[c2].visits.size()); ++j) {
                    const int va = sol.routes[c1].visits[i];
                    const int vb = sol.routes[c2].visits[j];
                    // Skills must still hold after the swap.
                    if (!inst.carers[c1].has_skill(inst.visits[vb].skill)) continue;
                    if (!inst.carers[c2].has_skill(inst.visits[va].skill)) continue;

                    Solution trial = sol;
                    std::swap(trial.routes[c1].visits[i], trial.routes[c2].visits[j]);
                    const double cost = evaluate(inst, trial, w);
                    if (cost + 1e-9 < cur) {
                        sol = std::move(trial);
                        cur = cost;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Reverse a run of visits inside one carer's day (an intra-route 2-opt).
static bool try_reverse(const Instance& inst, const Weights& w, Solution& sol,
                        double& cur) {
    for (int c = 0; c < inst.n_carers(); ++c) {
        const int len = static_cast<int>(sol.routes[c].visits.size());
        for (int i = 0; i < len - 1; ++i) {
            for (int j = i + 1; j < len; ++j) {
                Solution trial = sol;
                std::reverse(trial.routes[c].visits.begin() + i,
                             trial.routes[c].visits.begin() + j + 1);
                const double cost = evaluate(inst, trial, w);
                if (cost + 1e-9 < cur) {
                    sol = std::move(trial);
                    cur = cost;
                    return true;
                }
            }
        }
    }
    return false;
}

bool local_search(const Instance& inst, const Weights& w, Solution& sol,
                  int max_passes) {
    double cur = evaluate(inst, sol, w);
    bool improved_any = false;

    for (int pass = 0; pass < max_passes; ++pass) {
        const bool improved =
            try_relocate(inst, w, sol, cur) ||
            try_swap(inst, w, sol, cur) ||
            try_reverse(inst, w, sol, cur);
        if (!improved)
            break;
        improved_any = true;
    }
    evaluate(inst, sol, w);  // make sure metrics match the final solution
    return improved_any;
}

}  // namespace careroute
