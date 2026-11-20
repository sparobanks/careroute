"""Clean and validate raw CSVs into a solver-ready instance, with a report.

    python scripts/clean_data.py --carers data/raw/carers.csv \
        --visits data/raw/visits.csv --out data/cleaned.txt \
        --report reports/validation.txt
"""
from __future__ import annotations

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from careroute_data.validate import (clean, load_csv, report_lines,  # noqa: E402
                                     to_instance_lines)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--carers", default="data/raw/carers.csv")
    ap.add_argument("--visits", default="data/raw/visits.csv")
    ap.add_argument("--out", default="data/cleaned.txt")
    ap.add_argument("--report", default="reports/validation.txt")
    args = ap.parse_args()

    carers_raw = load_csv(args.carers)
    visits_raw = load_csv(args.visits)
    carers, visits, rep = clean(carers_raw, visits_raw)

    with open(args.out, "w") as f:
        f.write("\n".join(to_instance_lines(carers, visits)) + "\n")

    os.makedirs(os.path.dirname(args.report) or ".", exist_ok=True)
    report_text = "\n".join(report_lines(rep))
    with open(args.report, "w") as f:
        f.write(report_text + "\n")

    print(report_text)
    print(f"\nclean instance -> {args.out}")


if __name__ == "__main__":
    main()
