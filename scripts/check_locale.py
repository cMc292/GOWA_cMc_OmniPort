#!/usr/bin/env python3
"""Validate that every launcher locale has the same keys as English."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1] / "locale"


def keys(path: Path):
    out = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.lstrip().startswith("#") or "=" not in line:
            continue
        k, _ = line.split("=", 1)
        out[k.strip()] = out.get(k.strip(), 0) + 1
    return out


english = keys(root / "en.lang")
bad = False
for p in sorted(root.glob("*.lang")):
    got = keys(p)
    missing = sorted(set(english) - set(got))
    extra = sorted(set(got) - set(english))
    dupes = sorted(k for k, n in got.items() if n > 1)
    print(f"{p.name}: keys={len(got)} missing={len(missing)} extra={len(extra)} duplicates={len(dupes)}")
    for k in missing[:10]:
        print(f"  missing: {k}")
        bad = True
    for k in extra[:10]:
        print(f"  extra:   {k}")
        bad = True
    for k in dupes[:10]:
        print(f"  duplicate: {k}")
        bad = True
sys.exit(1 if bad else 0)
