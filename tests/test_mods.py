#!/usr/bin/env python3
"""The mods folder (port/mods.h) on made-up folders, and the file system's side of it (vfs_mods.h).

Builds a small driver of port/mods.cpp with clang-cl and runs it on mods folders under a temporary directory: which
folders and zips are mods, in which order, which files each gives and which a mod above it has too, a zip unpacked
once and again when it changes, and the settings the Mods tab writes. Then builds ps3recomp's tests/test_vfs_mods.c,
which looks files up as the game does. No game file is used.

Usage:
    python tests/test_mods.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DRIVER = r'''
#include "mods.h"
#include "ps3emu/ps3_settings.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
static std::string u8(const std::wstring& w)
{
    char b[4096];
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, b, sizeof b, NULL, NULL);
    return n > 0 ? std::string(b) : std::string();
}
static std::string rel(const std::wstring& p, const std::wstring& base)
{
    if (p.size() > base.size() && !_wcsnicmp(p.c_str(), base.c_str(), base.size()) && p[base.size()] == L'\\')
        return u8(p.substr(base.size() + 1));
    return p.empty() ? std::string("-") : u8(p);
}
static void print(const ModList& l)
{
    for (const ModInfo& m : l.mods) {
        std::string later, dirs;
        for (const std::string& k : m.later) later += (later.empty() ? "" : ",") + k;
        for (const std::wstring& u : m.usrdirs) dirs += (dirs.empty() ? "" : ",") + rel(u, l.dir);
        printf("mod\t%s\ton=%d\tzip=%d\tname=%s\tversion=%s\tauthor=%s\tdesc=%s\tfiles=%d\tshadowed=%d\tby=%s\t"
               "later=%s\terror=%s\troot=%s\tusrdirs=%s\tarchive=%d\ttext=%d\tpacked=%s\twarn=%d\n", u8(m.id).c_str(),
               m.on, m.zip, m.name.c_str(), m.version.c_str(), m.author.c_str(), m.desc.c_str(), m.files, m.shadowed,
               u8(m.shadowed_by).c_str(), later.c_str(), m.error.c_str(), rel(m.root, l.dir).c_str(), dirs.c_str(),
               m.archive_files, m.text_files, m.packed.c_str(), m.pack_warn);
    }
    printf("built\t%s\n", l.built.empty() ? "-" : rel(l.built, l.dir).c_str());
    printf("packerror\t%s\n", l.pack_error.c_str());
    std::string d = mods_dirs(l), out;
    size_t at = 0;
    while (!d.empty() && at <= d.size()) {
        size_t e = d.find('|', at);
        if (e == std::string::npos) e = d.size();
        std::wstring w(d.begin() + (long long)at, d.begin() + (long long)e);
        wchar_t wb[2048];
        MultiByteToWideChar(CP_UTF8, 0, d.substr(at, e - at).c_str(), -1, wb, 2048);
        out += (out.empty() ? "" : "|") + rel(wb, l.dir);
        at = e + 1;
    }
    printf("dirs\t%s\n", out.c_str());
    printf("summary\t%s\n", mods_summary(l).c_str());
}
int wmain(int argc, wchar_t** argv)
{
    char ini[2048];
    WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, ini, sizeof ini, NULL, NULL);
    ps3_settings_load(ini);
    ModList l;
    if (!wcscmp(argv[2], L"scan") || !wcscmp(argv[2], L"look")) {
        mods_scan(&l, !wcscmp(argv[2], L"scan"));
        print(l);
    } else if (!wcscmp(argv[2], L"pack")) {   /* as the game starts: the game's update folder in argv[3] */
        mods_scan(&l, 1);
        mods_pack(&l, argv[3]);
        print(l);
    } else if (!wcscmp(argv[2], L"store")) {
        /* on:<id> off:<id> up:<id> down:<id>, as the tab does them, then written */
        mods_scan(&l, 1);
        for (int a = 3; a < argc; a++) {
            const wchar_t* c = wcschr(argv[a], L':');
            if (!c) continue;
            for (size_t i = 0; i < l.mods.size(); i++) {
                if (_wcsicmp(l.mods[i].id.c_str(), c + 1)) continue;
                if (!wcsncmp(argv[a], L"on", 2)) l.mods[i].on = 1;
                else if (!wcsncmp(argv[a], L"off", 3)) l.mods[i].on = 0;
                else if (!wcsncmp(argv[a], L"up", 2) && i > 0) std::swap(l.mods[i], l.mods[i - 1]);
                else if (!wcsncmp(argv[a], L"down", 4) && i + 1 < l.mods.size()) std::swap(l.mods[i], l.mods[i + 1]);
                break;
            }
        }
        mods_store(l);
        return ps3_settings_save(NULL) ? 1 : 0;
    }
    return 0;
}
'''


def put(path, data=b"x"):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data if isinstance(data, bytes) else data.encode("utf-8"))


def toolchain_env():
    sys.path.insert(0, str(ROOT / "scripts"))
    import gowa
    return gowa.toolchain_env()


class Mods(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="mods-")
        cls.exe = os.path.join(cls.tmp, "t.exe")
        src = os.path.join(cls.tmp, "t.cpp")
        Path(src).write_text(DRIVER)
        env = toolchain_env()
        clang = shutil.which("clang-cl", path=env.get("PATH"))
        if not clang:
            if os.environ.get("CI"):
                raise AssertionError("no clang-cl")
            raise unittest.SkipTest("no clang-cl")
        z = ROOT / "third_party" / "zlib"
        cmd = [clang, "/nologo", "/O1", "/EHsc", "/std:c++17", "-D_CRT_SECURE_NO_WARNINGS", "/I" + str(ROOT / "port"),
               "/I" + str(ROOT), "/I" + str(ROOT / "ps3recomp" / "include"), src, str(ROOT / "port" / "mods.cpp"),
               str(ROOT / "ps3recomp" / "runtime" / "settings" / "ps3_settings.c"),
               *[str(z / f) for f in ("adler32.c", "crc32.c", "inffast.c", "inflate.c", "inftrees.c", "zutil.c")],
               "/Fe" + cls.exe, "/Fo" + cls.tmp + "\\"]
        r = subprocess.run(cmd, capture_output=True, text=True, env=env, cwd=cls.tmp)
        if r.returncode:
            raise AssertionError(r.stdout + r.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="m-", dir=self.tmp)
        self.mods = os.path.join(self.d, "mods")
        os.makedirs(self.mods)
        self.ini = os.path.join(self.d, "ascension-r.ini")

    def settings(self, **mods):
        lines = ["[mods]", "folder = " + self.mods] + ["%s = %s" % kv for kv in mods.items()]
        Path(self.ini).write_text("\n".join(lines) + "\n")

    def run_t(self, *a, env=None):
        r = subprocess.run([self.exe, self.ini, *a], capture_output=True, text=True, encoding="utf-8", env=env)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        mods, out = {}, {}
        order = []
        for line in r.stdout.splitlines():
            f = line.split("\t")
            if f[0] == "mod":
                mods[f[1]] = dict(kv.split("=", 1) for kv in f[2:])
                order.append(f[1])
            elif f[0] in ("dirs", "summary", "built", "packerror"):
                out[f[0]] = f[1] if len(f) > 1 else ""
        return order, mods, out

    def zip(self, name, files, when=None):
        p = os.path.join(self.mods, name)
        with zipfile.ZipFile(p, "w", zipfile.ZIP_DEFLATED) as z:
            for n, data in files.items():
                z.writestr(n, data)
        if when:
            os.utime(p, (when, when))
        return p

    def test_a_folder_with_mod_ini_and_files(self):
        put(os.path.join(self.mods, "Icons", "mod.ini"),
            "\ufeff[mod]\nname = Xbox icons\nversion = 1.0\nauthor = \"Someone\"\ndescription = Icons, \x01changed.\n"
            "[other]\nname = not this\n")
        put(os.path.join(self.mods, "Icons", "files", "PATCHES"), b"6")
        put(os.path.join(self.mods, "Icons", "files", "WAD", "PS3_BE", "SHELL.WAD"))
        self.settings()
        order, m, out = self.run_t("scan")
        self.assertEqual(order, ["Icons"])
        i = m["Icons"]
        self.assertEqual((i["name"], i["version"], i["author"], i["desc"]), ("Xbox icons", "1.0", "Someone", "Icons,  changed."))
        self.assertEqual((i["on"], i["files"], i["error"], i["usrdirs"]), ("1", "2", "", "Icons\\files"))
        self.assertEqual(out["dirs"], "Icons\\files")
        self.assertEqual(out["summary"], "Xbox icons 1.0 (2 files)")

    def test_the_layouts_a_mod_comes_in(self):
        # As RPCS3's mods come: dev_hdd0\game\<id>\USRDIR, USRDIR, PS3_GAME\USRDIR; files right in the folder, with a
        # read-me beside them; a folder of the game's own (data) taken as USRDIR's, not gone into.
        put(os.path.join(self.mods, "Rpcs3", "dev_hdd0", "game", "NPUA80918", "USRDIR", "A.BIN"))
        put(os.path.join(self.mods, "Usr", "USRDIR", "B.BIN"))
        put(os.path.join(self.mods, "Disc", "PS3_GAME", "USRDIR", "C.BIN"))
        put(os.path.join(self.mods, "Loose", "TPPS6.PSARC"))
        put(os.path.join(self.mods, "Loose", "README.txt"))
        put(os.path.join(self.mods, "Font", "data", "fonts", "symbol3.fnt"))
        self.settings()
        _, m, _ = self.run_t("scan")
        self.assertEqual(m["Rpcs3"]["usrdirs"], "Rpcs3\\dev_hdd0\\game\\NPUA80918\\USRDIR")
        self.assertEqual(m["Usr"]["usrdirs"], "Usr\\USRDIR")
        self.assertEqual(m["Disc"]["usrdirs"], "Disc\\PS3_GAME\\USRDIR")
        self.assertEqual((m["Loose"]["usrdirs"], m["Loose"]["files"]), ("Loose", "1"))
        self.assertEqual((m["Font"]["root"], m["Font"]["usrdirs"], m["Font"]["files"]), ("Font", "Font", "1"))

    def test_a_folder_in_a_folder_and_the_old_text_tool_folder(self):
        # Extracted into a folder of its own: the one folder in it is the mod. The text tool's old output, a
        # dev_hdd0 folder right in the mods folder, is one mod.
        put(os.path.join(self.mods, "Pack-1.0", "Pack", "mod.ini"), "[mod]\nname = Pack\n")
        put(os.path.join(self.mods, "Pack-1.0", "Pack", "files", "X.BIN"))
        put(os.path.join(self.mods, "dev_hdd0", "game", "BCES01741", "USRDIR", "PATCHES"), b"6")
        self.settings()
        _, m, _ = self.run_t("scan")
        self.assertEqual((m["Pack-1.0"]["root"], m["Pack-1.0"]["name"]), ("Pack-1.0\\Pack", "Pack"))
        self.assertEqual(m["Pack-1.0"]["usrdirs"], "Pack-1.0\\Pack\\files")
        self.assertEqual((m["dev_hdd0"]["usrdirs"], m["dev_hdd0"]["files"]), ("dev_hdd0\\game\\BCES01741\\USRDIR", "1"))

    def test_order_off_and_the_file_a_mod_above_has(self):
        for name in ("A", "B", "C"):
            put(os.path.join(self.mods, name, "files", "SHARED.BIN"), name)
            put(os.path.join(self.mods, name, "files", name + ".BIN"))
        put(os.path.join(self.mods, "D", "files", "shared.bin"))   # the same name, in another case
        self.settings(order="C|A|gone", off="B")
        order, m, out = self.run_t("scan")
        self.assertEqual(order, ["C", "A", "B", "D"])   # the settings' order, then the new ones by name
        self.assertEqual((m["C"]["shadowed"], m["A"]["shadowed"], m["A"]["by"]), ("0", "1", "C"))
        self.assertEqual((m["B"]["on"], m["B"]["shadowed"]), ("0", "0"))   # off: no file of its is taken
        self.assertEqual((m["D"]["on"], m["D"]["shadowed"], m["D"]["by"]), ("1", "1", "C"))
        self.assertEqual(out["dirs"], "C\\files|A\\files|D\\files")
        self.assertEqual(out["summary"], "C (2 files), A (2 files, 1 from a mod above), D (1 file, 1 from a mod above); "
                                         "off: B (2 files)")
        self.settings(order="C|A", off="B", use="off")
        _, _, out = self.run_t("scan")
        self.assertEqual(out["dirs"], "")
        self.assertTrue(out["summary"].startswith("Use mods is off; in the folder: C (2 files)"), out["summary"])

    def test_the_tab_writes_the_order_and_the_switches(self):
        for name in ("A", "B", "C"):
            put(os.path.join(self.mods, name, "files", name + ".BIN"))
        self.settings()
        r = subprocess.run([self.exe, self.ini, "store", "down:A", "off:C", "up:C"], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        text = Path(self.ini).read_text()
        self.assertIn("order = B|C|A", text)
        self.assertIn("off = C", text)
        order, m, out = self.run_t("scan")
        self.assertEqual(order, ["B", "C", "A"])
        self.assertEqual(out["dirs"], "B\\files|A\\files")

    def test_a_zip_is_unpacked_once_and_again_when_it_changes(self):
        self.zip("Big.zip", {"Big/mod.ini": "[mod]\nname = Big\n", "Big/files/WAD/A.WAD": b"one" * 1000,
                             "Big/files/B.BIN": b"two"}, when=time.time() - 100)
        self.settings()
        _, m, out = self.run_t("look")   # the report's look: not unpacked
        self.assertEqual(m["Big.zip"]["error"], "not unpacked yet: the launcher unpacks it")
        self.assertEqual(out["dirs"], "")
        _, m, out = self.run_t("scan")
        b = m["Big.zip"]
        self.assertEqual((b["zip"], b["name"], b["files"], b["error"]), ("1", "Big", "2", ""))
        self.assertEqual(b["root"], ".unpacked\\Big.zip\\Big")
        self.assertEqual(out["dirs"], ".unpacked\\Big.zip\\Big\\files")
        unpacked = Path(self.mods, ".unpacked", "Big.zip", "Big", "files", "WAD", "A.WAD")
        self.assertEqual(unpacked.read_bytes(), b"one" * 1000)
        first = unpacked.stat().st_mtime_ns
        time.sleep(0.05)
        self.run_t("scan")
        self.assertEqual(unpacked.stat().st_mtime_ns, first)   # the same zip: not unpacked again
        self.zip("Big.zip", {"Big/files/WAD/A.WAD": b"three"})
        _, m, _ = self.run_t("scan")
        self.assertEqual(unpacked.read_bytes(), b"three")
        self.assertFalse(Path(self.mods, ".unpacked", "Big.zip", "Big", "files", "B.BIN").exists())   # the old copy gone
        self.assertEqual(m["Big.zip"]["name"], "Big")
        self.assertNotIn(".unpacked", self.run_t("scan")[1])   # not a mod itself

    def test_zips_that_are_not_read(self):
        self.zip("Evil.zip", {"../evil.txt": "x"})
        self.zip("Drive.zip", {"C:/x.txt": "x"})
        Path(self.mods, "Bad.zip").write_bytes(b"not a zip at all, but long enough to look at")
        put(os.path.join(self.mods, "Empty", "README.txt"))
        put(os.path.join(self.mods, "Later", "patches", "patch.yml"))
        self.settings()
        order, m, out = self.run_t("scan")
        self.assertTrue(m["Evil.zip"]["error"].startswith("the zip was not unpacked: it names a file outside its folder"))
        self.assertTrue(m["Drive.zip"]["error"].startswith("the zip was not unpacked: it names a file outside"))
        self.assertEqual(m["Bad.zip"]["error"], "the zip was not unpacked: it is not a zip file")
        self.assertEqual(m["Empty"]["error"], "it is empty")
        self.assertEqual((m["Later"]["later"], m["Later"]["error"]), ("patches", "this build does not read its patches yet"))
        # The new ones by name, a zip among the folders.
        self.assertEqual(order, ["Bad.zip", "Drive.zip", "Empty", "Evil.zip", "Later"])
        self.assertEqual(out["dirs"], "")
        self.assertFalse(Path(self.d, "evil.txt").exists())
        self.assertFalse(Path(self.mods, "evil.txt").exists())

    # --- the pack: archive\ and text\ as one more update (scripts/modpack.py), run as the game starts ---

    def pack_env(self, python=None):
        env = dict(os.environ)
        env["GOWA_PYTHON"] = python or sys.executable
        env["GOWA_MODPACK"] = str(ROOT / "scripts" / "modpack.py")
        return env

    def game(self):
        """test_gametext's made-up folder of the game's updates: PATCHES 5, a TPPS5.PSARC with the English text."""
        sys.path.insert(0, str(ROOT / "scripts"))
        sys.path.insert(0, str(ROOT / "tests"))
        import gametext as gt
        from test_gametext import MSGS, MP, SP, patch
        usr = os.path.join(self.d, "game", "USRDIR")
        os.makedirs(usr)
        Path(usr, "PATCHES").write_bytes(b"5")
        Path(usr, "TPPS5.PSARC").write_bytes(gt.psarc_pack({SP: patch(MSGS, 0xBBA20), MP: patch(MSGS, 0xCCE40)}))
        return usr

    def test_archive_and_text_packed_once_as_one_more_update(self):
        usr = self.game()
        put(os.path.join(self.mods, "Words", "text", "en.tsv"), "1.1\tJeda\n")
        put(os.path.join(self.mods, "Words", "archive", "DC", "PS3_BE", "X.DCB"), b"mine")
        put(os.path.join(self.mods, "Loose", "files", "PATCHES"), b"7")   # a mod with a seventh update of its own
        put(os.path.join(self.mods, "Loose", "files", "TPPS7.PSARC"), b"its own update")
        self.settings(order="Words|Loose")
        env = self.pack_env()
        _, m, out = self.run_t("pack", usr, env=env)
        self.assertEqual(out["packerror"], "")
        self.assertEqual(out["built"], ".built\\files")
        self.assertEqual(out["dirs"], ".built\\files|Loose\\files")   # the pack's PATCHES before any mod's
        self.assertEqual(m["Words"]["packed"], "archive: 1 entry; text EN: 1 line")
        built = Path(self.mods, ".built", "files")
        self.assertEqual(sorted(os.listdir(built)), ["PATCHES", "TPPS8.PSARC"])   # after Loose's seventh
        self.assertEqual((built / "PATCHES").read_bytes(), b"8")
        result = Path(self.mods, ".built", "result.txt")
        first = result.stat().st_mtime_ns
        time.sleep(0.05)
        _, m, _ = self.run_t("pack", usr, env=env)   # nothing has changed: not packed again
        self.assertEqual(result.stat().st_mtime_ns, first)
        self.assertEqual(m["Words"]["packed"], "archive: 1 entry; text EN: 1 line")
        _, m, _ = self.run_t("scan")   # the launcher's look: what the last start packed, as the mods are the same
        self.assertEqual(m["Words"]["packed"], "archive: 1 entry; text EN: 1 line")
        put(os.path.join(self.mods, "Words", "text", "en.tsv"), "1.1\tJeda\n3.1\tTidak ada\n")
        _, m, out = self.run_t("scan")   # changed since: the launcher says it is packed at the next start
        self.assertEqual((m["Words"]["packed"], m["Words"]["archive"], m["Words"]["text"]), ("", "1", "1"))
        _, m, out = self.run_t("pack", usr, env=env)   # a file of it changed: packed again
        self.assertNotEqual(result.stat().st_mtime_ns, first)
        self.assertEqual((m["Words"]["packed"], m["Words"]["warn"]),
                         ("archive: 1 entry; text EN: 1 line, 1 left out (3.1: no such line in the game's text)", "1"))
        self.assertIn("text EN: 1 line, 1 left out", out["summary"])

    def test_no_pack_without_python_or_with_mods_off(self):
        usr = self.game()
        put(os.path.join(self.mods, "Words", "text", "en.tsv"), "1.1\tJeda\n")
        self.settings()
        _, m, out = self.run_t("pack", usr, env=self.pack_env(python=os.path.join(self.d, "no-python.exe")))
        self.assertTrue(out["packerror"].startswith("no Python"), out["packerror"])
        self.assertEqual((out["built"], out["dirs"], m["Words"]["error"]), ("-", "", ""))
        self.assertIn("not packed: no Python", out["summary"])
        self.settings(use="off")
        _, _, out = self.run_t("pack", usr, env=self.pack_env())
        self.assertEqual((out["built"], out["packerror"], out["dirs"]), ("-", "", ""))

    def test_no_folder(self):
        shutil.rmtree(self.mods)
        self.settings()
        order, _, out = self.run_t("scan")
        self.assertEqual((order, out["dirs"], out["summary"]), ([], "", "none"))


class VfsMods(unittest.TestCase):
    """ps3recomp's tests/test_vfs_mods.c: the lookups the game's file system makes."""

    def test_the_lookups(self):
        env = toolchain_env()
        clang = shutil.which("clang-cl", path=env.get("PATH"))
        if not clang:
            if os.environ.get("CI"):
                raise AssertionError("no clang-cl")
            raise unittest.SkipTest("no clang-cl")
        with tempfile.TemporaryDirectory(prefix="vfs-mods-") as d:
            exe = os.path.join(d, "t.exe")
            r = subprocess.run([clang, "/nologo", "/O1", "-D_CRT_SECURE_NO_WARNINGS",
                                "/I" + str(ROOT / "ps3recomp" / "runtime" / "syscalls"),
                                str(ROOT / "ps3recomp" / "tests" / "test_vfs_mods.c"), "/Fe" + exe, "/Fo" + d + "\\"],
                               capture_output=True, text=True, env=env, cwd=d)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            for mode in ([], ["untitled"]):   # PS3_TITLE set, as the port sets it, and not
                r = subprocess.run([exe, *mode], capture_output=True, text=True, cwd=d)
                self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
                self.assertIn("vfs_mods: ok", r.stdout)


if __name__ == "__main__":
    unittest.main()
