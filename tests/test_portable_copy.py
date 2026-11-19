#!/usr/bin/env python3
"""portable_copy.cpp: a portable folder's first start taking the files of %LOCALAPPDATA%\\Ascension-R along.

Builds a driver with clang-cl that calls portable_copy_from on a made-up program folder: settings that name it, the
saves, GOWA.elf, the built game code with its build's work folder, logs, a shader cache, and links to a made-up game
folder as the launcher makes them. What is copied, what is left out, the settings' paths, the links made again (and
the game's files not copied through them), and the old folder as it was.

Usage:
    python tests/test_portable_copy.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DRIVER = r'''
#include "portable_copy.h"
#include <stdio.h>
int wmain(int argc, wchar_t** argv)
{
    char note[2000] = "";
    const int failed = portable_copy_from(argv[1], argv[2], note, sizeof note);
    printf("failed %d\n%s\n", failed, note);
    return 0;
}
'''


@unittest.skipUnless(os.name == "nt", "Windows only")
class PortableCopy(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="portable-copy-")
        sys.path.insert(0, str(ROOT / "scripts"))
        import gowa
        cls.gowa = gowa
        env = gowa.toolchain_env()
        clang = shutil.which("clang-cl", path=env.get("PATH"))
        if not clang:
            raise unittest.SkipTest("no clang-cl")
        cls.exe = os.path.join(cls.tmp, "t.exe")
        src = os.path.join(cls.tmp, "t.cpp")
        Path(src).write_text(DRIVER)
        cmd = [clang, "/nologo", "/O1", "/EHsc", "/std:c++17", "/I" + str(ROOT / "port"), src,
               str(ROOT / "port" / "portable_copy.cpp"), str(ROOT / "port" / "game_find.cpp"), "/Fe" + cls.exe,
               "/Fo" + cls.tmp + "\\"]
        r = subprocess.run(cmd, capture_output=True, text=True, env=env, cwd=cls.tmp)
        if r.returncode:
            raise AssertionError(r.stdout + r.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="run-", dir=self.tmp)
        self.old = os.path.join(self.d, "AppData", "Local", "Ascension-R")
        self.new = os.path.join(self.d, "Games", "Ascension-R-kit")
        self.game = os.path.join(self.d, "RPCS3", "dev_hdd0", "game", "NPUA80918")
        self.links = []

    def tearDown(self):
        for link in self.links:   # the links first, as links: never what they lead to
            self.gowa.junction_off(link)

    def put(self, *parts, data=b"x"):
        p = Path(*parts)
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)
        return str(p)

    def link(self, *parts):
        link = os.path.join(*parts)
        os.makedirs(os.path.dirname(link), exist_ok=True)
        self.assertTrue(self.gowa.junction_on(link, self.game))
        self.links.append(link)
        return link

    def make_old(self):
        o = self.old
        self.put(self.game, "PARAM.SFO", data=b"sfo")
        self.put(self.game, "USRDIR", "GAME.PSARC", data=b"game data, not to be copied")
        fwd = o.replace("\\", "/")
        self.put(o, "ascension-r.ini", data=(
            "[paths]\nvfs = %s\\vfs\nhdd0 = %s\\gamedata\\dev_hdd0\nelf = %s\\GOWA.elf\n[graphics]\n"
            "shader_cache = %s/d3d12_shader_cache.bin\nscaling = fsr3\n" % (o, o.upper(), o, fwd)).encode())
        self.put(o, "GOWA.elf", data=b"\x7fELF made up")
        self.put(o, "d3d12_shader_cache.bin.dxil", data=b"cache" * 100)
        self.put(o, "game-code", "ascension_game.dll", data=b"MZ dll")
        self.put(o, "game-code", "work", "analysis", "big.bin", data=b"w" * 5000)
        self.put(o, "logs", "session-1.log", data=b"log")
        self.put(o, "gamedata", "dev_hdd0", "home", "00000001", "savedata", "BCUS98232-PROFDATA-0", "SAVEDATA",
                 data=b"save")
        self.put(o, "gamedata", "dev_hdd0", "game", "NPUA80918-DATA", "USRDIR", "x.bin", data=b"game's own data")
        self.link(o, "vfs", "PS3_GAME")
        self.link(o, "gamedata", "dev_hdd0", "game", "NPUA80918")
        os.makedirs(self.new)
        self.put(self.new, "portable.txt", data=b"")

    def tree(self, top):
        out = []
        for d, dirs, files in os.walk(top):
            for name in list(dirs):
                if os.path.islink(os.path.join(d, name)) or self.gowa_is_junction(os.path.join(d, name)):
                    dirs.remove(name)
                    out.append((os.path.relpath(os.path.join(d, name), top), "link"))
            for f in files:
                p = os.path.join(d, f)
                out.append((os.path.relpath(p, top), os.path.getsize(p), os.path.getmtime(p)))
        return sorted(out, key=str)

    @staticmethod
    def gowa_is_junction(p):
        try:
            return bool(os.lstat(p).st_file_attributes & 0x400)   # FILE_ATTRIBUTE_REPARSE_POINT
        except OSError:
            return False

    def copy(self):
        r = subprocess.run([self.exe, self.old, self.new], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        first, _, note = r.stdout.strip().partition("\n")
        return int(first.split()[1]), note

    def test_the_files_come_along(self):
        self.make_old()
        before = self.tree(self.old)
        failed, note = self.copy()
        self.assertEqual(failed, 0, note)
        self.assertIn("[portable] copied from", note)
        n = self.new
        for rel in ("GOWA.elf", "d3d12_shader_cache.bin.dxil", os.path.join("game-code", "ascension_game.dll"),
                    os.path.join("gamedata", "dev_hdd0", "home", "00000001", "savedata", "BCUS98232-PROFDATA-0",
                                 "SAVEDATA"),
                    os.path.join("gamedata", "dev_hdd0", "game", "NPUA80918-DATA", "USRDIR", "x.bin")):
            self.assertEqual(Path(n, rel).read_bytes(), Path(self.old, rel).read_bytes(), rel)
        # Left out: the logs and the build's work folder.
        self.assertFalse(os.path.exists(os.path.join(n, "logs")))
        self.assertFalse(os.path.exists(os.path.join(n, "game-code", "work")))
        # The old folder as it was.
        self.assertEqual(self.tree(self.old), before)

    def test_the_settings_name_the_new_folder(self):
        self.make_old()
        self.copy()
        text = Path(self.new, "ascension-r.ini").read_text()
        self.assertNotIn(self.old.lower(), text.lower())
        self.assertNotIn(self.old.replace("\\", "/").lower(), text.lower())
        self.assertIn("vfs = %s\\vfs" % self.new, text)
        self.assertIn("hdd0 = %s\\gamedata\\dev_hdd0" % self.new, text)
        self.assertIn("elf = %s\\GOWA.elf" % self.new, text)
        self.assertIn("shader_cache = %s/d3d12_shader_cache.bin" % self.new.replace("\\", "/"), text)
        self.assertIn("scaling = fsr3", text)

    def test_the_links_are_made_again_not_followed(self):
        self.make_old()
        self.copy()
        for rel in (os.path.join("vfs", "PS3_GAME"), os.path.join("gamedata", "dev_hdd0", "game", "NPUA80918")):
            link = os.path.join(self.new, rel)
            self.links.append(link)
            self.assertTrue(self.gowa_is_junction(link), rel)
            self.assertEqual(os.path.realpath(link).lower(), os.path.realpath(self.game).lower())
        # The game's files are where they were, once.
        self.assertEqual(Path(self.game, "USRDIR", "GAME.PSARC").read_bytes(), b"game data, not to be copied")

    def test_a_link_whose_folder_is_gone(self):
        self.make_old()
        Path(self.game, "USRDIR", "GAME.PSARC").unlink()
        Path(self.game, "PARAM.SFO").unlink()
        os.rmdir(os.path.join(self.game, "USRDIR"))
        os.rmdir(self.game)
        failed, note = self.copy()
        # The files still come; the links to a folder no longer there are said, not made.
        self.assertTrue(Path(self.new, "GOWA.elf").is_file())
        self.assertGreaterEqual(failed, 0)
        self.assertIn("[portable]", note)


if __name__ == "__main__":
    unittest.main()
