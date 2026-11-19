"""The mods' archive\\ and text\\ folders as one more update of the game (port/mods.h).

    modpack.py <input.txt> <out folder>

The game reads its archives, then its updates TPPS.PSARC and TPPS1.PSARC to TPPS<n>.PSARC, n from PATCHES; an
entry of a newer one stands for the older one's. So the files the mods have for entries of the archives, and the
lines of the game's text they change, go into one more update: <out>\\files\\TPPS<n+1>.PSARC and PATCHES, which the
game reads before any mod's own files (docs/notes/2026-11-18-mods-archive-and-text.md).

- An entry the game changes through a .PATCH in an update (its WADs, .TS and .AS files) goes in as a .PATCH of one
  step that takes all of its bytes from the patch's own data: the game puts the newest .PATCH over the base, and a
  whole file in the base's place gets the older update's .PATCH put over it (the EU disc then hangs at its load).
  Any other entry goes in whole, as the game's own updates bring the DCB files. A mod's own .PATCH goes in as it is.
- Text: lines in the form `gowa.py text export` writes (<id>.<k><TAB><text>), in .tsv files named for the language
  (en.tsv, en-gb-story.tsv, or text\\fr\\menus.tsv). English, en, is the game's EN and also its EN-GB (the EU disc's
  English) where the mod has no en-gb file of its own. Each line is checked as `text build` checks it, and one that
  fails is left out. Both of the language's patches get the lines (R_LANG_<lang>.WAD.PATCH and its _MP), made from
  the game's newest ones.
- The first mod in the list wins: an entry or a line a mod above has is left out.

input.txt, UTF-8, a tab between the fields:
    usrdir  <folder>              the game's folder of its updates: PATCHES, TPPS.PSARC, TPPS1.PSARC...
    updates <n>                   the updates the game reads before this one (PATCHES, or a mod's own PATCHES)
    mod     <id> <archive folder or -> <text folder or ->   in the list's order

<out>\\result.txt, what each mod gave, a tab between the fields:
    update  <n+1, or 0 for none> <entries>
    archive <id> <entries> <of them as a .PATCH> <left out> <the first one left out, or ->
    text    <id> <lang> <lines> <left out> <why the first one was, or ->
    error   <id or -> <message>
"""
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gametext as gt  # noqa: E402

# The game's languages as its R_LANG_<lang>.WAD files name them; a file's name is matched with the longest first.
LANGS = ("EN-GB", "ES-LA", "PT-BR", "EN", "FR", "ES", "DE", "IT", "NL", "PT", "RU", "PL", "TR", "EL", "JA", "KO", "ZH")
PATCH_VERSION = 4
# What Windows and other systems leave in folders, which no mod means as a file of the game's.
LITTER = {"desktop.ini", "thumbs.db", ".ds_store"}


def read_input(path):
    cfg = {"usrdir": None, "updates": 0, "mods": []}
    with open(path, encoding="utf-8-sig") as f:
        for ln in f.read().splitlines():
            c = ln.split("\t")
            if c[0] == "usrdir" and len(c) > 1:
                cfg["usrdir"] = c[1]
            elif c[0] == "updates" and len(c) > 1:
                cfg["updates"] = int(c[1])
            elif c[0] == "mod" and len(c) > 3:
                cfg["mods"].append({"id": c[1], "archive": None if c[2] == "-" else c[2],
                                    "text": None if c[3] == "-" else c[3]})
    return cfg


def official_names(usrdir):
    """The entries the game's updates have: TPPS.PSARC and TPPS1.PSARC to TPPS<PATCHES>.PSARC."""
    names = set()
    n = gt.patch_count(usrdir) if os.path.isfile(os.path.join(usrdir, "PATCHES")) else 0
    for k in [""] + [str(i) for i in range(1, n + 1)]:
        p = os.path.join(usrdir, "TPPS%s.PSARC" % k)
        if os.path.isfile(p):
            names.update(gt.psarc_names(p))
    return names


def files_under(folder):
    """(the entry's name as the archives have it, /A/B.WAD in upper case, and the host path) of each file under it."""
    out = []
    for d, _, fs in os.walk(folder):
        for f in fs:
            if f.lower() in LITTER or f.startswith("."):
                continue
            p = os.path.join(d, f)
            out.append(("/" + os.path.relpath(p, folder).replace(os.sep, "/").upper(), p))
    return sorted(out)


def lang_of(name):
    """The language a text file's name, or its folder's, is for; None when it names none."""
    n = name.upper()
    for code in LANGS:
        if n == code or (n.startswith(code) and n[len(code)] in "-_. "):
            return code
    return None


def text_files(folder):
    """{lang: [paths]} of a mod's text folder."""
    out = {}
    for d, _, fs in os.walk(folder):
        for f in sorted(fs):
            if f.lower().endswith(".tsv"):
                lang = lang_of(os.path.splitext(f)[0]) or lang_of(os.path.basename(d))
                if lang:
                    out.setdefault(lang, []).append(os.path.join(d, f))
    return out


def one_step(data):
    """A .PATCH that takes all of its bytes from its own data: the whole file, whatever the base."""
    return struct.pack(">HHII", PATCH_VERSION, 1, 0x80000000, len(data)) + data


def pack_archive(mods, official, files, results):
    """The archive\\ folders into files; {entry: the mod that gave it}."""
    owner = {}
    for m in mods:
        if not m["archive"]:
            continue
        got = patched = 0
        left = []
        for name, path in files_under(m["archive"]):
            entry = name if name.endswith(".PATCH") or name + ".PATCH" not in official else name + ".PATCH"
            if entry in owner:
                left.append(name)
                continue
            with open(path, "rb") as f:
                data = f.read()
            if entry != name:
                data = one_step(data)
                patched += 1
            files[entry] = data
            owner[entry] = m["id"]
            got += 1
        results.append(["archive", m["id"], got, patched, len(left), left[0] if left else "-"])
    return owner


def pack_text(mods, usrdir, owner, files, results):
    by_lang = {}
    for m in mods:
        if not m["text"]:
            continue
        tf = text_files(m["text"])
        for lang, paths in sorted(tf.items()):
            by_lang.setdefault(lang, []).append((m["id"], paths, False))
        if "EN" in tf and "EN-GB" not in tf:
            by_lang.setdefault("EN-GB", []).append((m["id"], tf["EN"], True))
    for lang in sorted(by_lang):
        here = by_lang[lang]
        names = [gt.WAD_DIR + "R_LANG_%s.WAD" % lang, gt.WAD_DIR + "R_LANG_%s_MP.WAD" % lang]
        brought = next((owner.get(n) or owner.get(n + ".PATCH") for n in names
                        if n in owner or n + ".PATCH" in owner), None)
        if brought:   # a mod's archive\ has the language's whole text: its lines are not put over it
            for mid, paths, _ in here:
                tr, _ = gt.read_translation(paths)
                results.append(["text", mid, lang, 0, len(tr), "%s brings the %s text whole" % (brought, lang)])
            continue
        try:
            found, _ = gt.newest_patches(usrdir, lang)
        except gt.TextError:
            for mid, _, implicit in here:
                if not implicit:   # en for EN-GB, where the release has no EN-GB: nothing to say
                    results.append(["error", mid, "the game has no %s text" % lang])
            continue
        tables = {nm: gt.Table(gt.patch_msgs(p)) for nm, (_, p) in found.items()}
        given = set()
        for mid, paths, implicit in here:
            tr, errors = gt.read_translation(paths)
            above = sorted(k for k in tr if k in given)
            tr = {k: v for k, v in tr.items() if k not in given}
            bad, first = set(), errors[0] if errors else None
            for t in tables.values():
                for e in gt.apply(t, dict(tr))[1]:
                    key = e.split(":", 1)[0]
                    if implicit and e.endswith("no such line in the game's text"):
                        tr.pop(key, None)   # a line EN has and EN-GB has not
                        continue
                    bad.add(key)
                    first = first or e
            given.update(k for k in tr if k not in bad)
            if above and not first:
                first = "%s: a mod above has it" % above[0]
            results.append(["text", mid, lang, len(tr) - len(bad), len(bad) + len(errors) + len(above), first or "-"])
        for nm, (_, p) in found.items():
            files[nm] = gt.patch_with_msgs(p, tables[nm].raw())


def pack(cfg, out_dir, log=print):
    results, files = [], {}
    usrdir = cfg["usrdir"]
    official = official_names(usrdir) if usrdir and os.path.isdir(usrdir) else set()
    owner = pack_archive(cfg["mods"], official, files, results)
    pack_text(cfg["mods"], usrdir, owner, files, results)
    n = cfg["updates"]
    dst = os.path.join(out_dir, "files")
    os.makedirs(dst, exist_ok=True)
    for f in os.listdir(dst):
        os.remove(os.path.join(dst, f))
    if files:
        t0 = time.time()
        arc = os.path.join(dst, "TPPS%d.PSARC" % (n + 1))
        with open(arc, "wb") as f:
            f.write(gt.psarc_pack(files, level=1))
        with open(os.path.join(dst, "PATCHES"), "wb") as f:
            f.write(b"%d" % (n + 1))
        log("TPPS%d.PSARC: %d entries, %.1f MB, in %.1f s" % (n + 1, len(files), os.path.getsize(arc) / 1048576.0,
                                                              time.time() - t0))
    results.insert(0, ["update", n + 1 if files else 0, len(files)])
    return results


def main(argv):
    if len(argv) != 3:
        print(__doc__)
        return 2
    out_dir = argv[2]
    os.makedirs(out_dir, exist_ok=True)
    try:
        results, code = pack(read_input(argv[1]), out_dir), 0
    except (OSError, ValueError, gt.TextError) as e:
        results, code = [["update", 0, 0], ["error", "-", str(e)]], 1
    with open(os.path.join(out_dir, "result.txt"), "w", encoding="utf-8", newline="\n") as f:
        for r in results:
            f.write("\t".join(str(x).replace("\t", " ").replace("\n", " ") for x in r) + "\n")
    return code


if __name__ == "__main__":
    sys.exit(main(sys.argv))
