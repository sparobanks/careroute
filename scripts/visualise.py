"""Draw a plan: a map of the routes and a Gantt chart of each carer's day.

    python scripts/visualise.py --instance data/town_60.txt \
        --solution reports/town_60_solution.txt --out reports

Writes routes.png (who goes where) and gantt.png (who does what, when).
"""
from __future__ import annotations

import argparse
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from careroute_data.instance import read_instance, read_solution  # noqa: E402

PALETTE = ["#2f7d52", "#1f5fa8", "#c2571a", "#8a3fb0", "#b23b3b", "#2b8c8c",
           "#9a7b2f", "#6a6f76", "#d0567f", "#4a7d2f", "#7a4a2f", "#3f5fb0"]


def hhmm(m: float) -> str:
    m = int(round(m))
    return f"{m // 60:02d}:{m % 60:02d}"


def draw_routes(inst: dict, sol: dict, path: str) -> None:
    fig, ax = plt.subplots(figsize=(7.2, 6.4))
    carers = {c["id"]: c for c in inst["carers"]}
    visits = {v["id"]: v for v in inst["visits"]}

    for idx, (cid, stops) in enumerate(sorted(sol["routes"].items())):
        col = PALETTE[idx % len(PALETTE)]
        c = carers[cid]
        xs = [c["x"]] + [visits[s["visit"]]["x"] for s in stops] + [c["x"]]
        ys = [c["y"]] + [visits[s["visit"]]["y"] for s in stops] + [c["y"]]
        ax.plot(xs, ys, "-", color=col, lw=1.3, alpha=0.8, zorder=1)
        ax.scatter([visits[s["visit"]]["x"] for s in stops],
                   [visits[s["visit"]]["y"] for s in stops],
                   s=22, color=col, zorder=2)
        ax.scatter([c["x"]], [c["y"]], marker="s", s=70, color=col,
                   edgecolor="white", linewidth=1.2, zorder=3)

    if sol["unassigned"]:
        ux = [visits[v]["x"] for v in sol["unassigned"]]
        uy = [visits[v]["y"] for v in sol["unassigned"]]
        ax.scatter(ux, uy, marker="x", s=55, color="#b23b3b", zorder=4,
                   label="unassigned")
        ax.legend(loc="upper right", frameon=False, fontsize=9)

    ax.set_title("Carer routes (squares are bases, dots are visits)",
                 loc="left", fontsize=11)
    ax.set_xlabel("km")
    ax.set_ylabel("km")
    ax.set_aspect("equal")
    ax.spines[["top", "right"]].set_visible(False)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)


def draw_gantt(inst: dict, sol: dict, path: str) -> None:
    carer_ids = sorted(sol["routes"].keys())
    visits = {v["id"]: v for v in inst["visits"]}
    fig, ax = plt.subplots(figsize=(8.4, 0.5 * max(4, len(carer_ids)) + 1))

    for row, cid in enumerate(carer_ids):
        col = PALETTE[row % len(PALETTE)]
        for s in sol["routes"][cid]:
            dur = s["finish"] - s["start"]
            ax.barh(row, dur, left=s["start"], height=0.6, color=col,
                    edgecolor="white")
    ax.set_yticks(range(len(carer_ids)))
    ax.set_yticklabels([f"carer {c}" for c in carer_ids], fontsize=9)
    ax.invert_yaxis()
    # x axis as clock times
    lo, hi = 480, 1020
    ticks = list(range(lo, hi + 1, 60))
    ax.set_xticks(ticks)
    ax.set_xticklabels([hhmm(t) for t in ticks], fontsize=8)
    ax.set_xlim(lo, hi)
    ax.set_title("Each carer's day (bars are visits)", loc="left", fontsize=11)
    ax.spines[["top", "right"]].set_visible(False)
    ax.grid(axis="x", color="#e2e7e2")
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--instance", default="data/town_60.txt")
    ap.add_argument("--solution", default="reports/town_60_solution.txt")
    ap.add_argument("--out", default="reports")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    inst = read_instance(args.instance)
    sol = read_solution(args.solution)
    draw_routes(inst, sol, os.path.join(args.out, "routes.png"))
    draw_gantt(inst, sol, os.path.join(args.out, "gantt.png"))
    print(f"wrote {args.out}/routes.png and {args.out}/gantt.png")


if __name__ == "__main__":
    main()
