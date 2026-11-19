#!/usr/bin/env python3
"""game_prepare.cpp's decrypt of a disc's GOWA.SELF, with a stand-in for RPCS3: the KLIC windows it offers, the order
of its tries, and that no key is printed or left behind.

Builds a driver and a stand-in rpcs3.exe with clang-cl. The stand-in "decrypts" EBOOT.BIN by copying it to
EBOOT.elf, and GOWA.SELF by copying it to GOWA.elf when a line of its standard input is the KLIC the test gave it
(FAKE_KLIC); it writes what it was given to FAKE_OUT. The update's EBOOT.BIN is a made-up big-endian ELF64 with a
window of each kind. A made-up GOWA.elf is never the 1.12 build, so a key found ends in "not version 1.12" with no
GOWA.elf kept, and a key not found in "was not found": enough to tell them apart. The keys here are made up.

Usage:
    python tests/test_game_prepare.py
"""

import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DRIVER = r'''
#include "game_prepare.h"
#include <stdio.h>
int wmain(int argc, wchar_t** argv)
{
    /* disc <update> <rpcs3.exe> <data> */
    if (argc != 5 || wcscmp(argv[1], L"disc")) return 2;
    wchar_t err[1024] = L"";
    GameProgress p = {};
    const int ok = game_prepare_disc(argv[2], argv[3], argv[4], err, 1024, &p);
    printf("disc %d %ls\n", ok, err[0] ? err : L"-");
    return 0;
}
'''

# RPCS3's --decrypt, as far as game_prepare_disc sees it: <file>.BIN or .SELF in, <file>.elf beside it out.
FAKE_RPCS3 = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
static int ends(const char* s, const char* t)
{
    const size_t a = strlen(s), b = strlen(t);
    return a >= b && !_stricmp(s + a - b, t);
}
int main(int argc, char** argv)
{
    const char* out = getenv("FAKE_OUT");
    FILE* o = out ? fopen(out, "a") : NULL;
    char cfg[1024] = "";
    GetEnvironmentVariableA("RPCS3_CONFIG_DIR", cfg, sizeof cfg);
    if (argc < 3 || strcmp(argv[1], "--decrypt")) return 2;
    char to[1024];
    snprintf(to, sizeof to, "%s", argv[2]);
    char* dot = strrchr(to, '.');
    if (!dot) return 3;
    strcpy(dot, ".elf");
    if (ends(argv[2], "EBOOT.BIN")) {
        if (o) fprintf(o, "eboot %s\n", cfg);
        CopyFileA(argv[2], to, FALSE);
    } else if (ends(argv[2], "GOWA.SELF")) {
        const char* want = getenv("FAKE_KLIC");
        char line[256];
        int found = 0;
        if (o) fprintf(o, "self %s\n", cfg);
        while (fgets(line, sizeof line, stdin)) {
            line[strcspn(line, "\r\n")] = 0;
            if (o) fprintf(o, "key %s\n", line);
            if (want && !strcmp(line, want)) found = 1;
        }
        if (found) CopyFileA(argv[2], to, FALSE);
    }
    if (o) fclose(o);
    return 0;
}
'''

# The windows of the made-up EBOOT.BIN's data. KEY looks like a key: 16 different bytes, 6 of them printable.
KEY = bytes.fromhex("8E1F23C4A75B90D2E66F01B83C7A5D94")
KEY_IN_CODE = bytes.fromhex("17A9C2E05B3D84F61E90B7254ACF6D38")    # in an executable segment: never offered
KEY_IN_NOTE = bytes.fromhex("29B4E7015CA38F6D92E04BC7351FA86E")    # in a segment that is not PT_LOAD
TEXT = b"GodOfWarAscensio"                                           # printable: wide only
FOUR = bytes([0x81, 0x92, 0xA3, 0xB4] * 4)                           # 4 different bytes: wide only
THREE = bytes([0x81, 0x92, 0xA3] * 5 + [0x81])                       # 3: never
ZERO = bytes(16)
LOGGED = "C3D5E7F90B1D2F41A3B5C7D9EB0D1F35"                          # only in the RPCS3 log


def short_path(p):
    """A path's short (8.3) form, which game_prepare hands to RPCS3."""
    import ctypes
    buf = ctypes.create_unicode_buffer(1024)
    return buf.value if ctypes.windll.kernel32.GetShortPathNameW(p, buf, 1024) else p


def hexs(b):
    return b.hex().upper()


def made_up_elf(segments):
    """A big-endian ELF64 with a program header for each (type, flags, bytes), its data after the headers, each
    segment at a 16-byte boundary."""
    phoff, phent = 64, 56
    at = (phoff + phent * len(segments) + 15) // 16 * 16
    head = bytearray(64)
    head[0:7] = b"\x7fELF\x02\x02\x01"
    struct.pack_into(">HHI", head, 16, 2, 21, 1)
    struct.pack_into(">QQQ", head, 24, 0, phoff, 0)
    struct.pack_into(">IHHHHHH", head, 48, 0, 64, phent, len(segments), 0, 0, 0)
    phs, body = b"", b""
    for ptype, flags, blob in segments:
        phs += struct.pack(">IIQQQQQQ", ptype, flags, at + len(body), 0, 0, len(blob), len(blob), 16)
        body += blob + bytes((-len(blob)) % 16)
    elf = bytes(head) + phs
    return elf + bytes((phoff + phent * len(segments) + 15) // 16 * 16 - len(elf)) + body


def eboot_elf():
    data = b"".join((ZERO, ZERO, KEY, ZERO, ZERO, TEXT, ZERO, ZERO, FOUR, ZERO, ZERO, THREE, ZERO, ZERO, KEY, ZERO))
    return made_up_elf([(1, 5, ZERO + KEY_IN_CODE + ZERO), (4, 4, ZERO + KEY_IN_NOTE + ZERO), (1, 6, data)])


@unittest.skipUnless(os.name == "nt", "Windows only")
class DiscDecrypt(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="game-prepare-")
        sys.path.insert(0, str(ROOT / "scripts"))
        import gowa
        env = gowa.toolchain_env()
        clang = shutil.which("clang-cl", path=env.get("PATH"))
        if not clang:
            raise unittest.SkipTest("no clang-cl")
        cls.exe = os.path.join(cls.tmp, "t.exe")
        cls.rpcs3_exe = os.path.join(cls.tmp, "bin", "rpcs3.exe")
        os.makedirs(os.path.dirname(cls.rpcs3_exe))
        for name, text, out, more in (("t.cpp", DRIVER, cls.exe, [str(ROOT / "port" / "game_prepare.cpp"),
                                                                   str(ROOT / "port" / "game_find.cpp")]),
                                      ("rpcs3.c", FAKE_RPCS3, cls.rpcs3_exe, [])):
            src = os.path.join(cls.tmp, name)
            Path(src).write_text(text)
            cmd = [clang, "/nologo", "/O1", "/EHsc", "/I" + str(ROOT / "port"), src, *more, "/Fe" + out,
                   "/Fo" + cls.tmp + "\\"]
            if name.endswith(".cpp"):
                cmd.insert(4, "/std:c++17")
            r = subprocess.run(cmd, capture_output=True, text=True, env=env, cwd=cls.tmp)
            if r.returncode:
                raise AssertionError(r.stdout + r.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="run-", dir=self.tmp)
        # An RPCS3 of its own each time (its log is a test's), with the update in its dev_hdd0 as RPCS3 installs it.
        self.emu = os.path.join(self.d, "rpcs3")
        os.makedirs(self.emu)
        shutil.copyfile(self.rpcs3_exe, os.path.join(self.emu, "rpcs3.exe"))
        self.update = os.path.join(self.emu, "dev_hdd0", "game", "BCES01741")
        os.makedirs(os.path.join(self.update, "USRDIR"))
        Path(self.update, "USRDIR", "EBOOT.BIN").write_bytes(eboot_elf())
        Path(self.update, "USRDIR", "GOWA.SELF").write_bytes(b"made-up GOWA.SELF")
        self.data = os.path.join(self.d, "data")
        os.makedirs(self.data)
        self.out = os.path.join(self.d, "rpcs3-saw.txt")

    def tree(self, top):
        return sorted((os.path.relpath(os.path.join(d, f), top), os.path.getsize(os.path.join(d, f)),
                       os.path.getmtime(os.path.join(d, f))) for d, _, fs in os.walk(top) for f in fs)

    def run_disc(self, klic, tries=None):
        """game_prepare_disc on the update: its result line, and the tries RPCS3 saw, each the keys it was given."""
        before = self.tree(self.emu)
        env = dict(os.environ, FAKE_OUT=self.out, FAKE_KLIC=klic)
        env.pop("GOWA_KLIC_TRIES", None)
        if tries:
            env["GOWA_KLIC_TRIES"] = tries
        r = subprocess.run([self.exe, "disc", self.update, os.path.join(self.emu, "rpcs3.exe"), self.data],
                           capture_output=True, text=True, env=env)
        self.assertEqual(r.returncode, 0, r.stderr)
        # No key in what it says; nothing of the work left; RPCS3's folder and the update as they were; RPCS3 run
        # with a config folder of ours.
        self.assertIsNone(re.search(r"[0-9A-Fa-f]{32}", r.stdout + r.stderr), r.stdout + r.stderr)
        self.assertEqual(os.listdir(self.data), [])
        self.assertEqual(self.tree(self.emu), before)
        saw, tried = Path(self.out).read_text().splitlines() if os.path.isfile(self.out) else [], []
        ours = [os.path.normcase(os.path.join(p, "decrypt", "rpcs3")) for p in (self.data, short_path(self.data))]
        for line in saw:
            what, _, rest = line.partition(" ")
            if what in ("eboot", "self"):
                self.assertIn(os.path.normcase(rest.rstrip("\\")), ours)
            if what == "self":
                tried.append([])
            elif what == "key":
                tried[-1].append(rest)
        self.assertEqual(saw[0].split(" ")[0], "eboot")
        return r.stdout.strip(), r.stderr, tried

    FOUND = "disc 0 The decrypted file is not version 1.12 of this game"
    NOT_FOUND = ("disc 0 The key to GOWA.SELF was not found. Start the game once in RPCS3 (to its first screen), close "
                 "it, and press Try again")

    def test_the_windows_that_look_like_a_key(self):
        out, err, tried = self.run_disc(hexs(KEY))
        self.assertEqual(out, self.FOUND)
        self.assertEqual(tried, [[hexs(KEY)]])   # the key once, though the data has it twice; no other window
        self.assertIn("1 narrow key to try", err)
        self.assertIn("decrypted\n", err)

    def test_every_window_when_none_of_those_is_it(self):
        out, err, tried = self.run_disc(hexs(FOUR))
        self.assertEqual(out, self.FOUND)
        self.assertEqual(len(tried), 2)          # narrow, then wide; no log, as wide found it
        wide = tried[1]
        self.assertEqual(len(wide), len(set(wide)))
        for w in (KEY, TEXT, FOUR):
            self.assertIn(hexs(w), wide)
        for w in (THREE, ZERO, KEY_IN_CODE, KEY_IN_NOTE):
            self.assertNotIn(hexs(w), wide)
        self.assertIn("narrow key to try", err)
        self.assertIn("wide keys to try", err)

    def test_the_key_rpcs3_logged(self):
        log = Path(self.emu, "log")
        log.mkdir()
        log.joinpath("RPCS3.log").write_text(
            "E: sceNpDrmIsAvailable2(): KLIC=0x%s, drm_path=\"/dev_hdd0/game/BCES01741/USRDIR/GOWA.SELF\"\n"
            "E: npDrmIsAvailable(): KLIC=0x12AB\n" % LOGGED.lower())
        out, err, tried = self.run_disc(LOGGED, tries="log")
        self.assertEqual(out, self.FOUND)
        self.assertEqual(tried, [[LOGGED]])

    def test_no_key_found(self):
        out, err, tried = self.run_disc("00112233445566778899AABBCCDDEEFF")
        self.assertEqual(out, self.NOT_FOUND)
        self.assertEqual(len(tried), 2)          # narrow and wide; no RPCS3 log, so no third
        self.assertIn("not decrypted", err)

    def test_an_update_without_its_eboot(self):
        os.remove(os.path.join(self.update, "USRDIR", "EBOOT.BIN"))
        r = subprocess.run([self.exe, "disc", self.update, os.path.join(self.emu, "rpcs3.exe"), self.data],
                           capture_output=True, text=True)
        self.assertEqual(r.stdout.strip(), "disc 0 The update's folder has no USRDIR\\GOWA.SELF or EBOOT.BIN: install "
                                           "the updates again in RPCS3")
        self.assertFalse(os.path.exists(self.out))


if __name__ == "__main__":
    unittest.main()
