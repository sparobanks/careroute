// main.cpp  -  the command-line runner.
//
//   careroute <instance> [--method greedy|ls|lns] [--seed N] [--iters N]
//             [--out solution.txt] [--continuity W] [--balance W] [--travel W]
//
// It loads an instance, solves it with the chosen method, prints one summary
// line of metrics (easy for the Python scripts to read), and optionally writes
// the full plan to a file.
#include "careroute/construct.hpp"
#include "careroute/io.hpp"
#include "careroute/lns.hpp"
#include "careroute/localsearch.hpp"
#include "careroute/model.hpp"
#include "careroute/solution.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace careroute;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr,
                     "usage: careroute <instance> [--method greedy|ls|lns] "
                     "[--seed N] [--iters N] [--out file] "
                     "[--continuity W] [--balance W] [--travel W]\n");
        return 1;
    }

    std::string instance_path = argv[1];
    std::string method = "lns";
    std::string out_path;
    LnsParams p;
    Weights w;

    for (int i = 2; i < argc; ++i) {
        auto next = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "missing value for %s\n", name);
                std::exit(1);
            }
            return argv[++i];
        };
        if (!std::strcmp(argv[i], "--method")) method = next("--method");
        else if (!std::strcmp(argv[i], "--seed")) p.seed = std::stoul(next("--seed"));
        else if (!std::strcmp(argv[i], "--iters")) p.iterations = std::stoi(next("--iters"));
        else if (!std::strcmp(argv[i], "--out")) out_path = next("--out");
        else if (!std::strcmp(argv[i], "--continuity")) w.continuity = std::stod(next("--continuity"));
        else if (!std::strcmp(argv[i], "--balance")) w.balance = std::stod(next("--balance"));
        else if (!std::strcmp(argv[i], "--travel")) w.travel = std::stod(next("--travel"));
        else {
            std::fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    Instance inst = load_instance(instance_path);

    const auto t0 = std::chrono::steady_clock::now();
    Solution sol;
    if (method == "naive") {
        sol = naive_construct(inst, w);
    } else if (method == "greedy") {
        sol = greedy_construct(inst, w);
    } else if (method == "ls") {
        sol = greedy_construct(inst, w);
        local_search(inst, w, sol);
    } else if (method == "lns") {
        sol = solve(inst, w, p);
    } else {
        std::fprintf(stderr, "unknown method: %s\n", method.c_str());
        return 1;
    }
    const auto t1 = std::chrono::steady_clock::now();
    const long ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    const Metrics& m = sol.metrics;
    const int covered = inst.n_visits() - m.unassigned;
    std::printf(
        "RESULT method=%s visits=%d covered=%d cost=%.1f travel=%.1f "
        "lateness=%.1f overtime=%.1f continuity=%d balance=%.1f time_ms=%ld\n",
        method.c_str(), inst.n_visits(), covered, m.cost, m.travel, m.lateness,
        m.overtime, m.continuity_violations, m.balance, ms);

    if (!out_path.empty()) {
        write_solution(out_path, inst, sol);
        std::printf("wrote %s\n", out_path.c_str());
    }
    return 0;
}
