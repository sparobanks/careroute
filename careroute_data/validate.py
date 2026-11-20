"""Ingest, clean and validate raw home-care data.

Real scheduling exports are messy: times written as "09:30" in one row and as
minutes in another, blank cells, duplicate ids, time windows the wrong way
round, visits that need a skill no carer has. Before any optimisation can run,
the data has to be made correct and consistent, and problems have to be flagged
rather than silently passed on.

This module does that. It reads carers and visits from CSV, fixes what it safely
can, drops what it cannot use, records every decision as an issue, and writes a
clean instance plus a human-readable report.

An issue has a severity:
  error  - the row could not be used and was dropped
  fixed  - something was wrong but we corrected it safely
  warn   - the row is kept but may need a human to look at it
"""
from __future__ import annotations

import csv
from dataclasses import dataclass, field


@dataclass
class Issue:
    severity: str   # "error", "fixed" or "warn"
    where: str      # e.g. "visit 12"
    message: str


@dataclass
class Report:
    issues: list[Issue] = field(default_factory=list)
    carers_in: int = 0
    carers_out: int = 0
    visits_in: int = 0
    visits_out: int = 0

    def add(self, severity: str, where: str, message: str) -> None:
        self.issues.append(Issue(severity, where, message))

    def counts(self) -> dict[str, int]:
        c = {"error": 0, "fixed": 0, "warn": 0}
        for i in self.issues:
            c[i.severity] = c.get(i.severity, 0) + 1
        return c


def parse_time(value: str) -> int | None:
    """Accept '09:30', '570', '570.0' and return minutes from midnight."""
    if value is None:
        return None
    s = str(value).strip()
    if not s:
        return None
    if ":" in s:
        try:
            h, m = s.split(":")
            return int(h) * 60 + int(m)
        except ValueError:
            return None
    try:
        return int(round(float(s)))
    except ValueError:
        return None


def _num(value: str) -> float | None:
    try:
        return float(str(value).strip())
    except (ValueError, AttributeError):
        return None


def _skills(value: str) -> list[int]:
    out: list[int] = []
    if value is None:
        return out
    for part in str(value).replace(";", ",").split(","):
        part = part.strip()
        if part:
            try:
                out.append(int(float(part)))
            except ValueError:
                pass
    return out


def load_csv(path: str) -> list[dict]:
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def clean(carer_rows: list[dict], visit_rows: list[dict]) -> tuple[list[dict], list[dict], Report]:
    rep = Report(carers_in=len(carer_rows), visits_in=len(visit_rows))

    # --- carers ---------------------------------------------------------
    carers: dict[int, dict] = {}
    for row in carer_rows:
        cid = row.get("id")
        try:
            cid = int(float(cid))
        except (TypeError, ValueError):
            rep.add("error", "carer ?", "missing or non-numeric id, dropped")
            continue
        where = f"carer {cid}"
        if cid in carers:
            rep.add("error", where, "duplicate id, later row dropped")
            continue
        x, y = _num(row.get("x")), _num(row.get("y"))
        start = parse_time(row.get("shift_start"))
        end = parse_time(row.get("shift_end"))
        if None in (x, y, start, end):
            rep.add("error", where, "missing location or shift time, dropped")
            continue
        if end <= start:
            rep.add("error", where, "shift ends before it starts, dropped")
            continue
        max_minutes = parse_time(row.get("max_minutes"))
        if max_minutes is None or max_minutes <= 0:
            max_minutes = end - start
            rep.add("fixed", where, "missing max_minutes, set to shift length")
        skills = _skills(row.get("skills"))
        if not skills:
            skills = [1]
            rep.add("fixed", where, "no skills listed, defaulted to skill 1")
        carers[cid] = dict(id=cid, x=x, y=y, shift_start=start, shift_end=end,
                           max_minutes=max_minutes, skills=skills)

    all_skills = set()
    for c in carers.values():
        all_skills.update(c["skills"])

    # --- visits ---------------------------------------------------------
    visits: dict[int, dict] = {}
    for row in visit_rows:
        vid = row.get("id")
        try:
            vid = int(float(vid))
        except (TypeError, ValueError):
            rep.add("error", "visit ?", "missing or non-numeric id, dropped")
            continue
        where = f"visit {vid}"
        if vid in visits:
            rep.add("error", where, "duplicate id, later row dropped")
            continue
        x, y = _num(row.get("x")), _num(row.get("y"))
        skill = _num(row.get("skill"))
        duration = _num(row.get("duration"))
        s = parse_time(row.get("tw_start"))
        e = parse_time(row.get("tw_end"))
        if None in (x, y, skill, duration, s, e):
            rep.add("error", where, "missing required field, dropped")
            continue
        skill = int(skill)
        if duration <= 0:
            rep.add("error", where, "non-positive duration, dropped")
            continue
        if e < s:
            s, e = e, s
            rep.add("fixed", where, "time window was reversed, swapped")
        pref = row.get("preferred_carer")
        pref_i = -1
        if pref not in (None, "", "-1"):
            try:
                pref_i = int(float(pref))
            except ValueError:
                pref_i = -1
            if pref_i not in carers:
                rep.add("fixed", where,
                        f"preferred carer {pref} does not exist, cleared")
                pref_i = -1
        if skill not in all_skills:
            rep.add("warn", where,
                    f"needs skill {skill} that no carer holds (will be left unassigned)")
        visits[vid] = dict(id=vid, x=x, y=y, skill=skill, duration=int(duration),
                           tw_start=s, tw_end=e, preferred_carer=pref_i)

    rep.carers_out = len(carers)
    rep.visits_out = len(visits)
    return list(carers.values()), list(visits.values()), rep


def to_instance_lines(carers: list[dict], visits: list[dict], speed_kmh: float = 30.0) -> list[str]:
    lines = ["# CareRoute instance (cleaned from raw data)", f"speed {speed_kmh:g}"]
    for c in sorted(carers, key=lambda r: r["id"]):
        skills = ",".join(str(s) for s in sorted(c["skills"]))
        lines.append(f"carer {c['id']} {c['x']:g} {c['y']:g} {c['shift_start']} "
                     f"{c['shift_end']} {c['max_minutes']} {skills}")
    for v in sorted(visits, key=lambda r: r["id"]):
        lines.append(f"visit {v['id']} {v['x']:g} {v['y']:g} {v['skill']} "
                     f"{v['duration']} {v['tw_start']} {v['tw_end']} {v['preferred_carer']}")
    return lines


def report_lines(rep: Report) -> list[str]:
    c = rep.counts()
    out = [
        "CareRoute data validation report",
        "=" * 34,
        f"carers: {rep.carers_in} read, {rep.carers_out} kept",
        f"visits: {rep.visits_in} read, {rep.visits_out} kept",
        f"issues: {c['error']} errors, {c['fixed']} fixed, {c['warn']} warnings",
        "",
    ]
    for i in rep.issues:
        out.append(f"[{i.severity:5}] {i.where}: {i.message}")
    return out
