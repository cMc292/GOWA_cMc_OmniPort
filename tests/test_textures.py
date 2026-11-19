#!/usr/bin/env python3
"""Texture mods: ps3recomp's libs/video/rsx_tex_replace.c (the ids, the decode, the chain) and port/textures.cpp
(the pictures of the mods that are on, an overlay over the game's own, the dump, the Xbox button icons' ids).

Builds ps3recomp's tests/test_rsx_tex_replace.c, and a driver of port/textures.cpp with port/mods.cpp, both with
clang-cl; the pictures are PNGs this test writes. No game file is used.

Usage:
    python tests/test_textures.py
"""

import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PS3 = ROOT / "ps3recomp"

DRIVER = r'''
#include "mods.h"
#include "textures.h"
#include "rsx_tex_replace.h"
#include "ps3emu/ps3_settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
/* t.exe <ini> <dump dir or -> <xbox 0|1> <command>...: has <id>; picture <id> <w> <h> <r,g,b,a of the base>;
 * dump <id> <w> <h> <r,g,b,a> */
static void parse4(const char* s, unsigned char* px)
{
    unsigned a, b, c, d;
    sscanf(s, "%u,%u,%u,%u", &a, &b, &c, &d);
    px[0] = (unsigned char)a; px[1] = (unsigned char)b; px[2] = (unsigned char)c; px[3] = (unsigned char)d;
}
int main(int argc, char** argv)
{
    ps3_settings_load(argv[1]);
    ModList l;
    mods_scan(&l, 1);
    wchar_t dump[1024] = L"";
    if (strcmp(argv[2], "-")) MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, dump, 1024);
    printf("textures %d\n", gowa_textures_start(l, dump, atoi(argv[3])));
    const rsx_tex_hooks* hk = rsx_tex_get_hooks();
    if (!hk) { printf("hooks none\n"); return 0; }
    for (int a = 4; a < argc; a++) {
        if (!strcmp(argv[a], "has")) {
            const u64 id = _strtoui64(argv[++a], NULL, 16);
            printf("has %016llx %d\n", (unsigned long long)id, hk->has ? hk->has(id) : -1);
        } else if (!strcmp(argv[a], "picture") || !strcmp(argv[a], "dump")) {
            const int pic = !strcmp(argv[a], "picture");
            const u64 id = _strtoui64(argv[++a], NULL, 16);
            const u32 w = (u32)atoi(argv[++a]), h = (u32)atoi(argv[++a]);
            unsigned char px[4];
            parse4(argv[++a], px);
            unsigned char* base = (unsigned char*)malloc((size_t)w * h * 4);
            for (size_t i = 0; i < (size_t)w * h; i++) memcpy(base + i * 4, px, 4);
            if (pic) {
                rsx_tex_picture out;
                memset(&out, 0, sizeof out);
                const int got = hk->picture(id, w, h, hk->has(id) == 2 ? base : NULL, &out);
                printf("picture %d %ux%u", got, out.w, out.h);
                if (got)   /* the four corners and the middle */
                    for (int k = 0; k < 5; k++) {
                        const u32 x = k == 4 ? out.w / 2 : (k & 1) ? out.w - 1 : 0, y = k == 4 ? out.h / 2 : (k & 2) ? out.h - 1 : 0;
                        const unsigned char* p = out.rgba + ((size_t)y * out.w + x) * 4;
                        printf(" %u,%u,%u,%u", p[0], p[1], p[2], p[3]);
                    }
                printf("\n");
            } else if (hk->dump) {
                hk->dump(id, w, h, 0x88, base);
                hk->dump(id, w, h, 0x88, base);   /* once a run */
                Sleep(500);   /* the dump's own thread writes it */
                printf("dumped\n");
            }
            free(base);
        }
    }
    return 0;
}
'''


def png(path, w, h, px):
    """A PNG of w x h, RGBA8, every texel px(x, y)."""
    raw = b"".join(b"\0" + b"".join(bytes(px(x, y)) for x in range(w)) for y in range(h))
    def chunk(t, c): return struct.pack(">I", len(c)) + t + c + struct.pack(">I", zlib.crc32(t + c) & 0xFFFFFFFF)
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)) +
                           chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


def toolchain():
    sys.path.insert(0, str(ROOT / "scripts"))
    import gowa
    env = gowa.toolchain_env()
    clang = shutil.which("clang-cl", path=env.get("PATH"))
    if not clang:
        if os.environ.get("CI"):
            raise AssertionError("no clang-cl")
        raise unittest.SkipTest("no clang-cl")
    return clang, env


class TexReplace(unittest.TestCase):
    """ps3recomp's tests/test_rsx_tex_replace.c."""

    def test_the_ids_the_decode_and_the_chain(self):
        clang, env = toolchain()
        with tempfile.TemporaryDirectory(prefix="tex-replace-") as d:
            exe = os.path.join(d, "t.exe")
            r = subprocess.run([clang, "/nologo", "/O1", "-D_CRT_SECURE_NO_WARNINGS", "/I" + str(PS3 / "include"),
                                "/I" + str(PS3 / "libs" / "video"), str(PS3 / "tests" / "test_rsx_tex_replace.c"),
                                str(PS3 / "libs" / "video" / "rsx_tex_replace.c"),
                                str(PS3 / "libs" / "video" / "rsx_texture_layout.c"), "/Fe" + exe, "/Fo" + d + "\\"],
                               capture_output=True, text=True, env=env, cwd=d)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            r = subprocess.run([exe], capture_output=True, text=True, cwd=d)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            self.assertIn("rsx_tex_replace: ok", r.stdout)


class Textures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        clang, env = toolchain()
        cls.tmp = tempfile.mkdtemp(prefix="textures-")
        cls.exe = os.path.join(cls.tmp, "t.exe")
        src = os.path.join(cls.tmp, "t.cpp")
        Path(src).write_text(DRIVER)
        z = ROOT / "third_party" / "zlib"
        cmd = [clang, "/nologo", "/O1", "/EHsc", "/std:c++17", "-D_CRT_SECURE_NO_WARNINGS", "/I" + str(ROOT / "port"),
               "/I" + str(ROOT), "/I" + str(PS3 / "include"), "/I" + str(PS3 / "libs" / "video"), src,
               str(ROOT / "port" / "textures.cpp"), str(ROOT / "port" / "mods.cpp"),
               str(PS3 / "runtime" / "settings" / "ps3_settings.c"), str(PS3 / "libs" / "video" / "rsx_tex_replace.c"),
               str(PS3 / "libs" / "video" / "rsx_texture_layout.c"),
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
        Path(self.ini).write_text("[mods]\nfolder = %s\n" % self.mods)

    def run_t(self, dump="-", xbox=0, *a):
        r = subprocess.run([self.exe, self.ini, dump, str(xbox), *a], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        return [ln for ln in r.stdout.splitlines() if not ln.startswith("[")]   # not the log's lines

    def test_a_picture_in_the_place_of_one_and_over_another(self):
        # A: a whole picture for one id, 8 x 4 red; and an overlay for another, a mod below having one for it too.
        png(os.path.join(self.mods, "A", "textures", "00000000000000aa.png"), 8, 4, lambda x, y: (255, 0, 0, 255))
        png(os.path.join(self.mods, "A", "textures", "sub", "00000000000000BB_64x64_88.overlay.png"), 4, 4,
            lambda x, y: (0, 0, 255, 255) if x == 0 else (0, 255, 0, 128) if x == 1 else (9, 9, 9, 1) if x == 2
            else (0, 0, 0, 0))
        png(os.path.join(self.mods, "B", "textures", "00000000000000bb.png"), 2, 2, lambda x, y: (1, 2, 3, 255))
        png(os.path.join(self.mods, "B", "textures", "not-an-id.png"), 2, 2, lambda x, y: (1, 2, 3, 255))
        Path(self.mods, "B", "textures", "00000000000000cc.png").write_bytes(b"not a png")
        out = self.run_t("-", 0, "has", "aa", "has", "bb", "has", "cc", "has", "dd",
                         "picture", "aa", "2", "2", "0,0,0,0",
                         "picture", "bb", "2", "2", "100,100,100,200",
                         "picture", "cc", "2", "2", "0,0,0,0", "has", "cc")
        self.assertEqual(out[0], "textures 3")
        self.assertEqual(out[1:5], ["has 00000000000000aa 1", "has 00000000000000bb 2", "has 00000000000000cc 1",
                                    "has 00000000000000dd 0"])
        self.assertEqual(out[5], "picture 1 8x4 255,0,0,255 255,0,0,255 255,0,0,255 255,0,0,255 255,0,0,255")
        # Over the base (100,100,100,200), scaled to 4 x 4: column 0 the overlay's blue, column 3 the base, row 0
        # and 3 alike; the middle (2, 2), alpha 1: clear.
        self.assertEqual(out[6], "picture 1 4x4 0,0,255,255 100,100,100,200 0,0,255,255 100,100,100,200 0,0,0,0")
        # A file that is no picture: not taken, and not asked again.
        self.assertEqual(out[7:], ["picture 0 0x0", "has 00000000000000cc 0"])

    def test_the_dump_and_the_xbox_icons_ids(self):
        dump = os.path.join(self.d, "dump")
        out = self.run_t(dump, 1, "has", "d8c17e20cbbe71ac", "has", "2133aab0625680af", "has", "a4f295497b5454b7",
                         "dump", "0123456789abcdef", "3", "2", "10,20,30,40")
        self.assertEqual(out[0], "textures 5")   # Cross, Circle, Square, Triangle and Select's bar
        self.assertEqual(out[1:4], ["has d8c17e20cbbe71ac 1", "has 2133aab0625680af 1", "has a4f295497b5454b7 1"])
        self.assertEqual(os.listdir(dump), ["0123456789abcdef_3x2_88.png"])
        data = Path(dump, "0123456789abcdef_3x2_88.png").read_bytes()
        self.assertEqual(data[:8], b"\x89PNG\r\n\x1a\n")
        self.assertEqual(struct.unpack(">II", data[16:24]), (3, 2))
        # A mod's picture of a button wins over the port's.
        png(os.path.join(self.mods, "Mine", "textures", "d8c17e20cbbe71ac.png"), 2, 2, lambda x, y: (5, 5, 5, 255))
        out = self.run_t("-", 1, "picture", "d8c17e20cbbe71ac", "64", "64", "0,0,0,0")
        self.assertEqual(out[1], "picture 1 2x2 5,5,5,255 5,5,5,255 5,5,5,255 5,5,5,255 5,5,5,255")
        # Nothing at all: no hooks.
        os.remove(os.path.join(self.mods, "Mine", "textures", "d8c17e20cbbe71ac.png"))
        self.assertEqual(self.run_t("-", 0), ["textures 0", "hooks none"])


if __name__ == "__main__":
    unittest.main()
