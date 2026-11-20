#!/usr/bin/env python3
"""Build the publishable portable from the full tree.

The executable is pinned in scripts/release-exe.json because the current build
can contain a launcher regression. Replace that pin only after a clean-package
end-to-end test.

Usage:
    python scripts/make_portable.py --version v0.1.5
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import os
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
    "SDL3.dll",
    "avcodec-62.dll",
    "avutil-60.dll",
    "swresample-6.dll",
    "dxcompiler.dll",
    "dxil.dll",
]
VC_RUNTIME = ["msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll", "concrt140.dll"]
DOCS = ["LICENSE", "THIRD-PARTY.txt", "THIRD_PARTY_NOTICES.md", "SOURCE_OFFER.txt",
        "CREDITOS.md", "update_check.txt"]

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


def release_exe(args_exe: str | None) -> tuple[Path, str | None]:
    if args_exe:
        return Path(args_exe).resolve(), None
    manifest = FULL / "scripts" / "release-exe.json"
    if manifest.is_file():
        m = json.loads(manifest.read_text(encoding="utf-8-sig"))
        src = (FULL / m["source"]).resolve()
        return src, m.get("sha256")
    return BUILD / "GOWA-cMc.exe", None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="v0.1.5", help="release version, e.g. v0.1.5")
    ap.add_argument("--exe", default=None, help="override the pinned known-good exe")
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
    (out / "vfs").mkdir(parents=True, exist_ok=True)
    (out / "gamedata" / "dev_hdd0").mkdir(parents=True, exist_ok=True)
    (out / "locale").mkdir(exist_ok=True)
    for loc in sorted((FULL / "locale").glob("*.lang")):
        shutil.copy2(loc, out / "locale" / loc.name)
    (out / "mods").mkdir(exist_ok=True)
    shutil.copy2(need(FULL / "scripts" / "mods-LEEME.txt"), out / "mods" / "LEEME-MODS.txt")
    # The portable mod packer: mods_pack looks for kit\python and kit\scripts.
    kit_py = FULL / "tools" / "python"
    if (kit_py / "python.exe").is_file():
        shutil.copytree(kit_py, out / "kit" / "python", dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
    kit_scripts = out / "kit" / "scripts"
    kit_scripts.mkdir(parents=True, exist_ok=True)
    for name in ("modpack.py", "gametext.py"):
        src = FULL / "scripts" / name
        if src.is_file():
            shutil.copy2(src, kit_scripts / name)

    # A portable release carries a working mods folder and its first mod.
    src_mod = BUILD / "mods" / "skip-intro"
    if src_mod.is_dir():
        dst_mod = out / "mods" / "skip-intro"
        dst_mod.mkdir(parents=True, exist_ok=True)
        for name in ("mod.ini", "mod.json"):
            src = src_mod / name
            if src.is_file():
                shutil.copy2(src, dst_mod / name)
        src_arc = src_mod / "archive"
        if src_arc.is_dir():
            shutil.copytree(src_arc, dst_mod / "archive", dirs_exist_ok=True)
    # Portable mode: settings, logs and caches beside the executable.
    (out / "portable.txt").write_text("GOWA_cMc_OmniPort portable\n", encoding="utf-8", newline="\n")

    exe_src, expected = release_exe(args.exe)
    exe_src = need(exe_src)
    shutil.copy2(exe_src, out / "GOWA-cMc.exe")
    exe_hash = sha256(out / "GOWA-cMc.exe")
    if expected and exe_hash.lower() != expected.lower():
        raise SystemExit(f"known-good exe hash mismatch: got {exe_hash}, expected {expected}")

    for n in RUNTIME:
        src = need(BUILD / n)
        shutil.copy2(src, out / n)


    sys32 = Path(os.environ.get("WINDIR", r"C:\Windows")) / "System32"
    for n in VC_RUNTIME:
        src = sys32 / n
        if src.is_file():
            shutil.copy2(src, out / n)

    ffroot = FULL / "_src" / "ffmpeg"
    ffbin = out / "_src" / "ffmpeg" / "bin"
    ffbin.mkdir(parents=True, exist_ok=True)
    for n in ("avcodec-62.dll", "avutil-60.dll", "swresample-6.dll",
              "avformat-62.dll", "avfilter-11.dll", "swscale-9.dll", "avdevice-62.dll"):
        src = ffroot / "bin" / n
        if not src.is_file():
            src = BUILD / n
        src = need(src)
        shutil.copy2(src, ffbin / n)
        shutil.copy2(src, out / n)
    lic = ffroot / "LICENSE.txt"
    if lic.is_file():
        shutil.copy2(lic, out / "_src" / "ffmpeg" / "LICENSE.txt")

    (out / "ascension-r.ini").write_text(CONFIG, encoding="utf-8", newline="\n")


    pkg = PROJECT / "GOWA_cMc_OmniPort-0.1.0-alpha.10"
    for n in DOCS:
        src = pkg / n
        if not src.is_file():
            src = FULL / n
        if src.is_file():
            shutil.copy2(src, out / n)

    date = datetime.date.today().isoformat()
    values = {"{{VERSION}}": version, "{{EXE_SHA256}}": exe_hash, "{{DATE}}": date}
    for tmpl_name, out_name in (("README-portable.template.md", "README.md"),
                                ("LEEME-portable.template.txt", "LEEME-PRIMERO.txt")):
        text = need(FULL / "scripts" / tmpl_name).read_text(encoding="utf-8")
        for k, v in values.items():
            text = text.replace(k, v)
        (out / out_name).write_text(text, encoding="utf-8", newline="\n")

    try:
        commit = subprocess.run(["git", "-C", str(FULL), "rev-parse", "--short", "HEAD"],
                                text=True, capture_output=True, check=False).stdout.strip()
    except Exception:
        commit = ""
    (out / "BUILD-INFO.txt").write_text(
        f"Package: {name}\nVersion: {version}\nFull: work/0.2.0-full\n"
        f"Exe source: {exe_src.relative_to(PROJECT) if exe_src.is_relative_to(PROJECT) else exe_src.name}\nExe SHA-256: {exe_hash}\n"
        f"Exe bytes: {(out / 'GOWA-cMc.exe').stat().st_size}\n"
        f"Full commit: {commit}\nGame data: not included; selected by the player on first run.\n",
        encoding="utf-8", newline="\n")

    for bad in ("logs",):
        if (out / bad).exists():
            raise SystemExit(f"unexpected generated file in package: {bad}")
    for cfg in ("ascension-r.ini",):
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
    print(f"exe source: {exe_src}")
    print(f"files:   {len([p for p in out.rglob('*') if p.is_file()])}")
    print(f"size:    {total / (1024 * 1024):.1f} MB")
    print(f"exe:     {exe_hash}")
    if not args.no_zip:
        print(f"zip:     {zpath} ({zpath.stat().st_size / (1024 * 1024):.1f} MB)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
