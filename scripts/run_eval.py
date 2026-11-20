"""Compare the methods across several instance sizes, and chart the result.

Runs the naive baseline, greedy construction, greedy plus local search, and the
full LNS on a range of instances (several random seeds each), averages the
results, writes reports/metrics.json and draws reports/comparison.png.

    python scripts/run_eval.py
"""
from __future__ import annotations

import json
import os
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "build", "careroute")
GEN = os.path.join(ROOT, "scripts", "generate_instance.py")

SIZES = [40, 60, 80]
SEEDS = [1, 2, 3]
CARERS = {40: 6, 60: 8, 80: 10}
METHODS = ["naive", "greedy", "ls", "lns"]
LABELS = {"naive": "Naive", "greedy": "Greedy", "ls": "Greedy + local search",
          "lns": "LNS (full)"}


def run(instance: str, method: str, seed: int) -> dict:
    cmd = [BIN, instance, "--method", method, "--seed", str(seed)]
    if method == "lns":
        cmd += ["--iters", "3000"]
    out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
    line = next(l for l in out.splitlines() if l.startswith("RESULT"))
    d = {}
    for tok in line.split()[1:]:
        k, v = tok.split("=")
        if k == "method":
            d[k] = v
        else:
            d[k] = float(v) if "." in v else int(v)
    return d


def main() -> None:
    os.makedirs(os.path.join(ROOT, "reports"), exist_ok=True)
    results: dict = {}

    for size in SIZES:
        results[size] = {m: {"cost": [], "travel": [], "covered": []} for m in METHODS}
        for seed in SEEDS:
            inst = os.path.join(ROOT, "data", f"eval_{size}_{seed}.txt")
            subprocess.run([sys.executable, GEN, "--carers", str(CARERS[size]),
                            "--visits", str(size), "--seed", str(seed),
                            "--out", inst], check=True,
                           capture_output=True, text=True)
            for m in METHODS:
                r = run(inst, m, seed)
                results[size][m]["cost"].append(r["cost"])
                results[size][m]["travel"].append(r["travel"])
                results[size][m]["covered"].append(r["covered"] / r["visits"])

    def mean(xs):
        return sum(xs) / len(xs)

    summary = {}
    for size in SIZES:
        summary[size] = {}
        for m in METHODS:
            summary[size][m] = {
                "cost": round(mean(results[size][m]["cost"]), 1),
                "travel": round(mean(results[size][m]["travel"]), 1),
                "covered_pct": round(100 * mean(results[size][m]["covered"]), 1),
            }

    with open(os.path.join(ROOT, "reports", "metrics.json"), "w") as f:
        json.dump(summary, f, indent=2)
    print(json.dumps(summary, indent=2))

    # charts: mean cost and mean travel by method, grouped by size
    colours = {"naive": "#999999", "greedy": "#9a7b2f",
               "ls": "#1f5fa8", "lns": "#2f7d52"}
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.4))
    x = range(len(SIZES))
    width = 0.2
    for ax, key, title in [(axes[0], "cost", "Total cost (lower is better)"),
                           (axes[1], "travel", "Travel minutes (lower is better)")]:
        for mi, m in enumerate(METHODS):
            vals = [summary[s][m][key] for s in SIZES]
            ax.bar([xi + (mi - 1.5) * width for xi in x], vals, width,
                   label=LABELS[m], color=colours[m])
        ax.set_xticks(list(x))
        ax.set_xticklabels([f"{s} visits" for s in SIZES])
        ax.set_title(title, loc="left", fontsize=11)
        ax.spines[["top", "right"]].set_visible(False)
    axes[0].set_yscale("log")  # naive dwarfs the rest, so use a log scale
    axes[0].set_ylabel("cost (log scale)")
    axes[1].set_ylabel("minutes")
    axes[0].legend(frameon=False, fontsize=8)
    fig.tight_layout()
    fig.savefig(os.path.join(ROOT, "reports", "comparison.png"), dpi=150)
    plt.close(fig)
    print("wrote reports/comparison.png and reports/metrics.json")


if __name__ == "__main__":
    main()
