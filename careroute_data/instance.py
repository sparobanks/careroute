"""Read CareRoute instance and solution files in Python (for tooling and plots)."""
from __future__ import annotations


def read_instance(path: str) -> dict:
    speed = 30.0
    carers: list[dict] = []
    visits: list[dict] = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            t = line.split()
            if t[0] == "speed":
                speed = float(t[1])
            elif t[0] == "carer":
                skills = [] if t[7] == "-" else [int(s) for s in t[7].split(",")]
                carers.append(dict(id=int(t[1]), x=float(t[2]), y=float(t[3]),
                                   shift_start=int(t[4]), shift_end=int(t[5]),
                                   max_minutes=int(t[6]), skills=skills))
            elif t[0] == "visit":
                visits.append(dict(id=int(t[1]), x=float(t[2]), y=float(t[3]),
                                   skill=int(t[4]), duration=int(t[5]),
                                   tw_start=int(t[6]), tw_end=int(t[7]),
                                   preferred_carer=int(t[8])))
    return dict(speed=speed, carers=carers, visits=visits)


def read_solution(path: str) -> dict:
    metrics: dict[str, float] = {}
    routes: dict[int, list[dict]] = {}
    unassigned: list[int] = []
    with open(path) as f:
        for line in f:
            t = line.split()
            if not t or t[0].startswith("#"):
                continue
            if t[0] == "metric":
                metrics[t[1]] = float(t[2])
            elif t[0] == "route":
                carer = int(t[1])
                routes.setdefault(carer, []).append(
                    dict(visit=int(t[2]), start=float(t[3]), finish=float(t[4])))
            elif t[0] == "unassigned":
                unassigned.append(int(t[1]))
    return dict(metrics=metrics, routes=routes, unassigned=unassigned)
