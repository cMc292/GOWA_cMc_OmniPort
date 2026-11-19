#!/usr/bin/env python3
"""Build the publishable portable from the current full tree.

Usage (from anywhere):
    python scripts/make_portable.py --version v0.1.5

The output is:
    <project>/work/dist/GOWA_cMc_OmniPort v0.1.5/
    <project>/work/dist/GOWA_cMc_OmniPort v0.1.5.zip

Only runtime files are copied. The game itself is never copied; the player
selects their own game folder on first run.
"""
from __future__ import annotations

import argparse
import hashlib
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

FULL = Path(__file__).resolve().parent.parent
PROJECT = FULL.parent.parent
BUILD = FULL / "build"
DIST = PROJECT / "work" / "dist"

RUNTIME = [
    "GOWA-cMc.exe",
    "GOWA.elf",
    "SDL3.dll",
    "avcodec-62.dll",
    "avutil-60.dll",
    "swresample-6.dll",
    "dxcompiler.dll",
    "dxil.dll",
]
VC_RUNTIME = ["msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll", "concrt140.dll"]
DOCS = ["LICENSE", "THIRD-PARTY.txt", "THIRD_PARTY_NOTICES.md", "SOURCE_OFFER.txt",
        "CREDITOS.md", "LEEME-PRIMERO.txt", "README.md", "update_check.txt"]

CONFIG = """[paths]
vfs =
hdd0 =
elf = GOWA.elf

[graphics]
shader_prebuild = on

[display]
show_fps = overlay
"""


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def need(path: Path) -> Path:
    if not path.is_file():
        raise SystemExit(f"missing required file: {path}")
    return path


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="v0.1.5", help="release version, e.g. v0.1.5")
    ap.add_argument("--no-zip", action="store_true")
    args = ap.parse_args()
    version = args.version.strip()
    if not version:
        raise SystemExit("empty version")

    name = f"GOWA_cMc_OmniPort {version}"
    out = DIST / name
    zpath = DIST / (name + ".zip")
    if out.exists():
        shutil.rmtree(out)
    if zpath.exists():
        zpath.unlink()
    out.mkdir(parents=True, exist_ok=True)

    # Required runtime, straight from the full build.
    copied = []
    for n in RUNTIME:
        src = need(BUILD / n)
        shutil.copy2(src, out / n)
        copied.append(n)

    # Visual C++ runtime is not part of the build output; take it from Windows.
    sys32 = Path(os.environ.get("WINDIR", r"C:\Windows")) / "System32"
    for n in VC_RUNTIME:
        src = sys32 / n
        if src.is_file():
            shutil.copy2(src, out / n)
            copied.append(n)

    # Only the FFmpeg runtime, never include/lib/doc.
    ffroot = FULL / "_src" / "ffmpeg"
    ffbin = out / "_src" / "ffmpeg" / "bin"
    ffbin.mkdir(parents=True, exist_ok=True)
    for n in ("avcodec-62.dll", "avutil-60.dll", "swresample-6.dll"):
        src = ffroot / "bin" / n
        if not src.is_file():
            # fall back to the build copy
            src = BUILD / n
        need(src)
        shutil.copy2(src, ffbin / n)
    lic = ffroot / "LICENSE.txt"
    if lic.is_file():
        shutil.copy2(lic, out / "_src" / "ffmpeg" / "LICENSE.txt")

    # Configuration: no absolute paths, shader prebuild enabled.
    (out / "ascension-r.ini").write_text(CONFIG, encoding="utf-8", newline="\n")
    (out / "gowa-cmc.ini").write_text(CONFIG, encoding="utf-8", newline="\n")

    # Legal and readme files from the project package.
    pkg = PROJECT / "GOWA_cMc_OmniPort-0.1.0-alpha.10"
    for n in DOCS:
        src = pkg / n
        if not src.is_file():
            src = FULL / n
        if src.is_file():
            shutil.copy2(src, out / n)

    # Provenance and an explicit hash check.
    exe_hash = sha256(out / "GOWA-cMc.exe")
    full_hash = sha256(BUILD / "GOWA-cMc.exe")
    if exe_hash != full_hash:
        raise SystemExit("packaged exe does not match the full build")
    try:
        commit = subprocess.run(["git", "-C", str(FULL), "rev-parse", "--short", "HEAD"],
                                text=True, capture_output=True, check=False).stdout.strip()
    except Exception:
        commit = ""
    (out / "BUILD-INFO.txt").write_text(
        f"Package: {name}\nVersion: {version}\nFull: {FULL}\n"
        f"Exe SHA-256: {exe_hash}\nExe bytes: {(out / 'GOWA-cMc.exe').stat().st_size}\n"
        f"Full commit: {commit}\nGame data: not included; selected by the player on first run.\n",
        encoding="utf-8", newline="\n")

    # Hard checks: no cache, logs, links, or absolute paths in config.
    for bad in ("logs", "d3d12_shader_cache.bin.dxil.prebuilt", "d3d12_shader_cache.bin.fprogs"):
        if (out / bad).exists():
            raise SystemExit(f"unexpected generated file in package: {bad}")
    for cfg in ("ascension-r.ini", "gowa-cmc.ini"):
        text = (out / cfg).read_text(encoding="utf-8")
        if "C:\\" in text or "D:\\" in text or "E:\\" in text:
            raise SystemExit(f"absolute path left in {cfg}")

    if not args.no_zip:
        with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as z:
            for p in sorted(out.rglob("*")):
                if p.is_file():
                    z.write(p, p.relative_to(out.parent))

    total = sum(p.stat().st_size for p in out.rglob("*") if p.is_file())
    print(f"package: {out}")
    print(f"files:   {len([p for p in out.rglob('*') if p.is_file()])}")
    print(f"size:    {total / (1024 * 1024):.1f} MB")
    print(f"exe:     {exe_hash}")
    if not args.no_zip:
        print(f"zip:     {zpath} ({zpath.stat().st_size / (1024 * 1024):.1f} MB)")
    return 0


if __name__ == "__main__":
    import os  # noqa: E402  (used above; kept local to avoid global import warning)
    raise SystemExit(main())
