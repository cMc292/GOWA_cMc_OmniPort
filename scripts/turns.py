"""What a switch gives, from a bench run that turned it on and off by turns (GOWA_TURNS).

    py scripts/turns.py logs/bench-<stamp> [tag ...] [-v] [--jobs-min=N] [--jobs-max=N]

A bench run with GOWA_TURNS=<seconds> in its environment (native/spu_edge_memo.c) logs every half second
    [turns] step S turn N on|off at T ms: F flips, H kept jobs
and a switch that asks (GOWA_EDGE_UNPACK, ...) is on in the odd turns. Each turn's frames are counted from its
second half second, when the frames in flight have changed over, to the next turn's start, and only turns inside the
bench's level window (load_s + from .. load_s + until, result.json) count.

The fight's load swings by a fifth every few seconds (1,300 to 2,600 kept jobs a flip, 25 to 45 ms a flip), so a turn's
frame time is fitted to its kept jobs a flip and the switch: ms = a + b * kept jobs a flip + c * on. c is the switch's
effect a flip (negative: faster), with its standard error; one run of 160 s in turns of 3 s gives about +-0.2-0.35 ms,
where two runs of one build differ by 1.3% (docs/notes/2026-11-16-s-j-review.md). --jobs-min and --jobs-max fit the
light or the heavy turns alone.
"""
import json
import os
import re
import statistics
import sys

STEP = re.compile(r"\[turns\] step (\d+) turn (\d+) (on|off) at (\d+) ms: (\d+) flips, (\d+) kept jobs")
HEAD = re.compile(r"\[turns\] turns of ([\d.]+) s")


def ols(xs, ys):
    """Least squares with an intercept: (coefficients, standard errors, residual sd)."""
    rows = [[1.0] + list(x) for x in xs]
    p, n = len(rows[0]), len(rows)
    a = [[sum(r[i] * r[j] for r in rows) for j in range(p)] for i in range(p)]
    inv = [[float(i == j) for j in range(p)] for i in range(p)]
    for c in range(p):
        piv = max(range(c, p), key=lambda r: abs(a[r][c]))
        a[c], a[piv], inv[c], inv[piv] = a[piv], a[c], inv[piv], inv[c]
        d = a[c][c]
        a[c] = [v / d for v in a[c]]
        inv[c] = [v / d for v in inv[c]]
        for r in range(p):
            if r != c and a[r][c]:
                f = a[r][c]
                a[r] = [v - f * w for v, w in zip(a[r], a[c])]
                inv[r] = [v - f * w for v, w in zip(inv[r], inv[c])]
    xty = [sum(r[i] * y for r, y in zip(rows, ys)) for i in range(p)]
    beta = [sum(inv[i][j] * xty[j] for j in range(p)) for i in range(p)]
    res = [y - sum(b * v for b, v in zip(beta, r)) for r, y in zip(rows, ys)]
    s2 = sum(e * e for e in res) / max(1, n - p)
    return beta, [(s2 * inv[i][i]) ** 0.5 for i in range(p)], s2 ** 0.5


def turns_of(path):
    """[(turn, on, t0 s, t1 s, flips, kept jobs)] of a run's log, each turn without its first half second."""
    steps, secs = {}, None
    with open(path, errors="replace") as f:
        for ln in f:
            m = HEAD.search(ln)
            if m:
                secs = float(m.group(1))
            m = STEP.search(ln)
            if m:
                steps[int(m.group(1))] = (int(m.group(2)), m.group(3) == "on", int(m.group(4)) / 1000.0,
                                          int(m.group(5)), int(m.group(6)))
    out = []
    if not steps or not secs:
        return out, secs
    halves = round(secs * 2)
    for turn in sorted({s[0] for s in steps.values()}):
        a, b = steps.get(turn * halves + 1), steps.get((turn + 1) * halves)
        if a and b:
            out.append((turn, a[1], a[2], b[2], b[3] - a[3], b[4] - a[4]))
    return out, secs


def arg(name, default):
    return float(next((a.split("=", 1)[1] for a in sys.argv if a.startswith(name + "=")), default))


def one(bench, tag, res):
    rec = next((r for r in res["runs"] if r.get("tag") == tag), None)
    if rec is None or "load_s" not in rec:
        print("%s: no run, or it reached no load" % tag)
        return
    lo, hi = rec["load_s"] + res["from"], rec["load_s"] + res["until"]
    jmin, jmax = arg("--jobs-min", 0), arg("--jobs-max", 1e9)
    turns, secs = turns_of(os.path.join(bench, tag + ".err"))
    rows = []
    for turn, on, t0, t1, fl, hits in turns:
        if t0 < lo or t1 > hi or fl <= 0 or not jmin <= hits / fl < jmax:
            continue
        rows.append({"turn": turn, "on": on, "t": t0 - rec["load_s"], "ms": 1000.0 * (t1 - t0) / fl, "jobs": hits / fl})
    print("== %s: turns of %s s, %d in %.0f-%.0f s after the load" % (tag, secs, len(rows), res["from"], res["until"]))
    on = [r for r in rows if r["on"]]
    off = [r for r in rows if not r["on"]]
    if len(on) < 3 or len(off) < 3:
        print("  too few turns (GOWA_TURNS unset, or a switch that never asked?)")
        return
    if "-v" in sys.argv:
        for r in rows:
            print("  turn %3d %-3s %6.1f s  %6.2f ms a flip  %6.0f kept jobs a flip" % (
                r["turn"], "on" if r["on"] else "off", r["t"], r["ms"], r["jobs"]))
    mean_ms = statistics.mean(r["ms"] for r in rows)
    print("  on: %d turns, %.2f ms a flip, %.0f kept jobs a flip; off: %d turns, %.2f ms, %.0f" % (
        len(on), statistics.mean(r["ms"] for r in on), statistics.mean(r["jobs"] for r in on), len(off),
        statistics.mean(r["ms"] for r in off), statistics.mean(r["jobs"] for r in off)))
    beta, se, sd = ols([(r["jobs"], 1.0 if r["on"] else 0.0) for r in rows], [r["ms"] for r in rows])
    print("  the switch: %+.3f +- %.3f ms a flip, %+.2f%% +- %.2f%% of the frame rate (ms = a + %.4f * jobs + c * on, "
          "residual sd %.3f ms)" % (beta[2], se[2], -100.0 * beta[2] / mean_ms, 100.0 * se[2] / mean_ms, beta[1], sd))


def main():
    bench = sys.argv[1]
    res = json.load(open(os.path.join(bench, "result.json")))
    tags = [a for a in sys.argv[2:] if not a.startswith("-")] or [r["tag"] for r in res["runs"]]
    for tag in tags:
        one(bench, tag, res)


if __name__ == "__main__":
    main()
