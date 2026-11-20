"""Make a realistic home-care instance and write it in CareRoute's text format.

This stands in for real operational data while we develop and test. It places
carers and clients across a town, gives carers skills and shifts, and gives each
visit a required skill, a duration and a time window. It makes sure every visit's
skill is held by at least one carer, so a full plan is always possible.

    python scripts/generate_instance.py --carers 8 --visits 60 --seed 1 \
        --out data/town_60.txt
"""
from __future__ import annotations

import argparse
import random

SKILLS = [1, 2, 3]  # 1 = personal care (all carers), 2 = medication, 3 = complex


def generate(n_carers: int, n_visits: int, area_km: float, seed: int) -> list[str]:
    rng = random.Random(seed)
    lines = [
        "# CareRoute instance (synthetic)",
        f"# carers={n_carers} visits={n_visits} seed={seed}",
        "speed 30",
    ]

    # Carers: everyone has skill 1; roughly half also have 2, a third also 3.
    carer_skills = []
    for c in range(n_carers):
        skills = {1}
        if rng.random() < 0.55:
            skills.add(2)
        if rng.random() < 0.35:
            skills.add(3)
        carer_skills.append(skills)
    # Guarantee at least one carer holds each skill.
    for s in (2, 3):
        if not any(s in sk for sk in carer_skills):
            carer_skills[rng.randrange(n_carers)].add(s)

    for c in range(n_carers):
        x = round(rng.uniform(0, area_km), 2)
        y = round(rng.uniform(0, area_km), 2)
        start = 480  # 08:00
        end = rng.choice([960, 1020])  # 16:00 or 17:00
        max_minutes = end - start  # a full shift; overtime is penalised softly
        skills = ",".join(str(s) for s in sorted(carer_skills[c]))
        lines.append(f"carer {c} {x} {y} {start} {end} {max_minutes} {skills}")

    # Visits spread across the working day with time windows.
    skilled_carers = {s: [c for c in range(n_carers) if s in carer_skills[c]]
                      for s in SKILLS}
    for v in range(n_visits):
        x = round(rng.uniform(0, area_km), 2)
        y = round(rng.uniform(0, area_km), 2)
        # skill 1 most common, then 2, then 3
        skill = rng.choices(SKILLS, weights=[0.6, 0.28, 0.12])[0]
        duration = rng.choice([20, 30, 30, 45, 60])
        # window start within the working day, so the visit fits inside a shift
        # (08:00 to 15:00); width 60-120 minutes
        tw_start = rng.randint(480, 900)
        tw_end = tw_start + rng.choice([60, 90, 120])
        # 40% of clients prefer a specific carer who can do the visit
        preferred = -1
        if rng.random() < 0.4 and skilled_carers[skill]:
            preferred = rng.choice(skilled_carers[skill])
        lines.append(
            f"visit {v} {x} {y} {skill} {duration} {tw_start} {tw_end} {preferred}")
    return lines


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--carers", type=int, default=8)
    ap.add_argument("--visits", type=int, default=60)
    ap.add_argument("--area", type=float, default=15.0)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--out", default="data/town_60.txt")
    args = ap.parse_args()

    lines = generate(args.carers, args.visits, args.area, args.seed)
    with open(args.out, "w") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {args.out}  ({args.carers} carers, {args.visits} visits)")


if __name__ == "__main__":
    main()
