// test_careroute.cpp  -  unit tests for the solver.
#include "check.hpp"

#include "careroute/construct.hpp"
#include "careroute/io.hpp"
#include "careroute/lns.hpp"
#include "careroute/localsearch.hpp"
#include "careroute/model.hpp"
#include "careroute/solution.hpp"

#include <cstdio>

using namespace careroute;

// A small instance built in code: 2 carers, a few visits.
static Instance make_instance() {
    Instance inst;
    inst.speed_kmh = 30;
    Carer a;
    a.id = 0; a.x = 0; a.y = 0; a.skills = {1, 2};
    a.shift_start = 480; a.shift_end = 1020; a.max_minutes = 540;
    Carer b;
    b.id = 1; b.x = 10; b.y = 0; b.skills = {1};
    b.shift_start = 480; b.shift_end = 1020; b.max_minutes = 540;
    inst.carers = {a, b};

    auto mk = [](int id, double x, double y, int skill, int dur, int s, int e, int pref) {
        Visit v; v.id = id; v.x = x; v.y = y; v.skill = skill; v.duration = dur;
        v.tw_start = s; v.tw_end = e; v.preferred_carer = pref; return v;
    };
    inst.visits = {
        mk(0, 3, 4, 1, 30, 500, 600, -1),  // 5 km from carer 0
        mk(1, 6, 8, 2, 20, 560, 700, -1),  // needs skill 2 -> only carer 0
        mk(2, 9, 0, 1, 30, 540, 700, 1),   // prefers carer 1
    };
    return inst;
}

static void test_travel() {
    Instance inst = make_instance();
    // 3-4-5 triangle: 5 km at 30 km/h = 10 minutes.
    CHECK_NEAR(inst.travel_min(0, 0, 3, 4), 10.0, 1e-6);
}

static void test_route_timing() {
    Instance inst = make_instance();
    Route r; r.carer = 0; r.visits = {0};  // one visit, 5 km away
    const RouteEval e = evaluate_route(inst, r);
    // out 10 min + back 10 min = 20 min travel; starts at 500 (waited), no late.
    CHECK_NEAR(e.travel, 20.0, 1e-6);
    CHECK_NEAR(e.lateness, 0.0, 1e-6);
    CHECK(e.skills_ok);
}

static void test_skill_hard_rule() {
    Instance inst = make_instance();
    // carer 1 lacks skill 2, so visit 1 must never be inserted there.
    Solution sol = empty_solution(inst);
    const Insertion ins = best_insertion(inst, Weights{}, sol, 1);
    CHECK(ins.found());
    CHECK(ins.carer == 0);  // only carer 0 has skill 2
}

static void test_construct_covers_all() {
    Instance inst = make_instance();
    const Weights w;
    Solution sol = greedy_construct(inst, w);
    CHECK(sol.metrics.unassigned == 0);
    CHECK(sol.metrics.skills_ok);
}

static void test_local_search_never_worsens() {
    Instance inst = make_instance();
    const Weights w;
    Solution sol = greedy_construct(inst, w);
    const double before = sol.metrics.cost;
    local_search(inst, w, sol);
    CHECK(sol.metrics.cost <= before + 1e-6);
}

static void test_lns_improves_or_matches() {
    Instance inst = make_instance();
    const Weights w;
    Solution greedy = greedy_construct(inst, w);
    LnsParams p; p.iterations = 500; p.seed = 7;
    Solution lns = solve(inst, w, p);
    CHECK(lns.metrics.cost <= greedy.metrics.cost + 1e-6);
    CHECK(lns.metrics.unassigned == 0);
}

static void test_continuity_counts() {
    Instance inst = make_instance();
    const Weights w;
    // Force visit 2 (prefers carer 1) onto carer 0 -> one continuity violation.
    Solution sol = empty_solution(inst);
    sol.routes[0].visits = {2};
    sol.unassigned = {0, 1};
    evaluate(inst, sol, w);
    CHECK(sol.metrics.continuity_violations == 1);
}

int main() {
    std::printf("running CareRoute tests\n");
    test_travel();
    test_route_timing();
    test_skill_hard_rule();
    test_construct_covers_all();
    test_local_search_never_worsens();
    test_lns_improves_or_matches();
    test_continuity_counts();

    if (check::failures() == 0) {
        std::printf("all tests passed\n");
        return 0;
    }
    std::printf("%d check(s) failed\n", check::failures());
    return 1;
}
