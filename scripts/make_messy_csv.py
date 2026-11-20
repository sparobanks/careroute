"""Turn a clean instance into deliberately messy raw CSVs, to demonstrate the
validator. It writes carers and visits as CSV, using mixed time formats and
injecting the kinds of errors real exports contain.

    python scripts/make_messy_csv.py --instance data/town_60.txt --out data/raw
"""
from __future__ import annotations

import argparse
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from careroute_data.instance import read_instance  # noqa: E402


def hhmm(minutes: int) -> str:
    return f"{minutes // 60:02d}:{minutes % 60:02d}"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--instance", default="data/town_60.txt")
    ap.add_argument("--out", default="data/raw")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    inst = read_instance(args.instance)

    # Carers: write shift times as HH:MM, and inject a few problems.
    with open(os.path.join(args.out, "carers.csv"), "w", newline="") as f:
        wr = csv.writer(f)
        wr.writerow(["id", "x", "y", "shift_start", "shift_end", "max_minutes", "skills"])
        for i, c in enumerate(inst["carers"]):
            skills = ";".join(str(s) for s in c["skills"])  # wrong separator on purpose
            maxm = "" if i == 1 else c["max_minutes"]        # one blank max_minutes
            wr.writerow([c["id"], c["x"], c["y"], hhmm(c["shift_start"]),
                         hhmm(c["shift_end"]), maxm, skills])
        # a duplicate carer id, and a carer with a broken shift
        wr.writerow([0, 1.0, 1.0, "08:00", "16:00", 480, "1"])        # duplicate id 0
        wr.writerow([99, 2.0, 2.0, "17:00", "09:00", 480, "1"])       # end before start

    # Visits: mixed time formats and injected errors.
    with open(os.path.join(args.out, "visits.csv"), "w", newline="") as f:
        wr = csv.writer(f)
        wr.writerow(["id", "x", "y", "skill", "duration", "tw_start", "tw_end",
                     "preferred_carer"])
        for i, v in enumerate(inst["visits"]):
            tw_s, tw_e = v["tw_start"], v["tw_end"]
            if i == 3:
                tw_s, tw_e = v["tw_end"], v["tw_start"]  # reversed window
            start_cell = tw_s if i % 2 == 0 else hhmm(tw_s)  # mix minutes and HH:MM
            pref = v["preferred_carer"]
            if i == 5:
                pref = 999                                  # non-existent carer
            dur = v["duration"] if i != 7 else 0            # bad duration
            wr.writerow([v["id"], v["x"], v["y"], v["skill"], dur,
                         start_cell, hhmm(tw_e), pref])
        # a visit needing a skill nobody has, a duplicate, and a blank-field row
        wr.writerow([1000, 5.0, 5.0, 9, 30, "10:00", "11:00", -1])   # unknown skill
        wr.writerow([0, 5.0, 5.0, 1, 30, "10:00", "11:00", -1])      # duplicate id 0
        wr.writerow([1001, "", 5.0, 1, 30, "10:00", "11:00", -1])    # missing x

    print(f"wrote messy CSVs to {args.out}/carers.csv and {args.out}/visits.csv")


if __name__ == "__main__":
    main()
