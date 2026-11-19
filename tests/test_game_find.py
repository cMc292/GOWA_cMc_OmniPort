#!/usr/bin/env python3
"""game_find.cpp on a made-up tree: what a folder holds, the search, and the links.

Builds a small driver with clang-cl and runs it on folders under a temporary directory. No game
file is used: a PARAM.SFO is written here, and a GOWA.elf of the right hash cannot be, so the
best a made-up copy reaches is GAME_OTHER_BUILD. The game's files are made-up ones, in lists of
the test's own (GAME_FILES_INC, and GAME_FILES_BCES01741_INC for a disc and its update) worked out
by gowa.py's ends_sha256, as `game-files` does.

Usage:
    python tests/test_game_find.py
"""

import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DRIVER = r'''
#include "game_find.h"
#include "game_title.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
static const char* name(GameState s)
{
    static const char* const n[] = { "none", "other_title", "incomplete", "no_elf", "other_build", "ok" };
    return n[s];
}
static void out(const wchar_t* tag, const GameInfo& g, int copies)
{
    printf("%ls %s %ls %s %s %d\n", tag, name(g.state), g.dir[0] ? g.dir : L"-", g.title_id[0] ? g.title_id : "-",
           g.app_ver[0] ? g.app_ver : "-", copies);
}
int wmain(int argc, wchar_t** argv)
{
    GameInfo g;
    wchar_t err[2048] = L"", t[2048] = L"";
    if (!wcscmp(argv[1], L"probe")) { game_probe(argv[2], &g); out(L"probe", g, 0); }
    else if (!wcscmp(argv[1], L"files")) {
        game_probe(argv[2], &g);
        wchar_t text[300] = L"-";
        if (g.state == GAME_INCOMPLETE) game_files_text(&g, text, 300);
        printf("files %s %d %d %d %llu %s %s %d\n%ls\n", name(g.state), g.files, g.files_missing, g.files_wrong,
               g.bytes_missing, g.first_missing[0] ? g.first_missing : "-", g.first_wrong[0] ? g.first_wrong : "-",
               g.other_version, text);
    } else if (!wcscmp(argv[1], L"needed")) {
        char p[512], id[16] = "NPUA80918";
        WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, p, sizeof p, NULL, NULL);
        if (argc > 3) WideCharToMultiByte(CP_UTF8, 0, argv[3], -1, id, sizeof id, NULL, NULL);
        printf("needed %d\n", game_file_needed(id, p));
    }
    else if (!wcscmp(argv[1], L"locate")) { game_locate(argv[2], &g); out(L"locate", g, 0); }
    else if (!wcscmp(argv[1], L"pair") || !wcscmp(argv[1], L"pairsearch")) {
        /* a disc and its update: what locate (Browse to a folder) or a search under it pairs */
        if (!wcscmp(argv[1], L"pair")) game_locate(argv[2], &g);
        else {
            volatile long stop = 0;
            GameSearch s = { argv[2], &stop, NULL, 20, 0, 0 };
            game_search(&s, &g);
        }
        printf("pair %s %d %s %ls %ls\n", name(g.state), (int)g.disc_lack, g.app_ver[0] ? g.app_ver : "-",
               g.dir[0] ? g.dir : L"-", g.update[0] ? g.update : L"-");
        if (!wcscmp(argv[1], L"pair") && argc > 3) {   /* pair <folder> files: what game_files_text says */
            wchar_t text[300] = L"-";
            if (g.state == GAME_INCOMPLETE) game_files_text(&g, text, 300);
            printf("%d %d %d %s\n%ls\n", g.files, g.files_missing, g.files_wrong,
                   g.first_missing[0] ? g.first_missing : g.first_wrong[0] ? g.first_wrong : "-", text);
        }
    }
    else if (!wcscmp(argv[1], L"title")) {
        wchar_t ids[96];
        game_titles_text(ids, 96);
        printf("title %s %ls\n", game_title_of_folder(argv[2])->id, ids);
    }
    else if (!wcscmp(argv[1], L"search") || !wcscmp(argv[1], L"search_first")) {
        /* search_first: as the launcher looks, the first copy that is this release will do */
        volatile long stop = 0;
        GameSearch s = { argv[2], &stop, NULL, 20, 0, !wcscmp(argv[1], L"search_first") ? GAME_NO_ELF : 0 };
        game_search(&s, &g);
        out(L"search", g, s.copies);
    } else if (!wcscmp(argv[1], L"link")) {
        const int ok = game_link(argv[2], argv[3], err, 2048);
        printf("link %d %ls\n", ok, err);
    } else if (!wcscmp(argv[1], L"target")) {
        const int ok = game_link_target(argv[2], t, 2048);
        printf("target %d %ls\n", ok, t);
    }
    return 0;
}
'''


def sfo(title_id="NPUA80918", app_ver="01.12", title="God of War: Ascension", category=None):
    """A PARAM.SFO with three text entries, and CATEGORY when given (DG a disc, GD its update or game data)."""
    items = [("APP_VER", app_ver)] + ([("CATEGORY", category)] if category else []) + \
            [("TITLE", title), ("TITLE_ID", title_id)]
    keys = b""
    data = b""
    table = b""
    for k, v in items:
        raw = v.encode("utf-8") + b"\0"
        table += struct.pack("<HHIII", len(keys), 0x0204, len(raw), len(raw), len(data))
        keys += k.encode() + b"\0"
        data += raw
    key_tab = 20 + len(table)
    data_tab = key_tab + len(keys)
    return struct.pack("<4sIIII", b"\0PSF", 0x101, key_tab, data_tab, len(items)) + table + keys + data


# The game's files of the test's list: one larger than an end (64 KiB), one with ends that overlap, smaller ones; the
# base game's and update 1.12's (its TPPS archives and PATCHES), in the order game_files.inc has them.
TEST_FILES = (("USRDIR/GAME.PSARC", 200000), ("USRDIR/ENGLISH.PSARC", 70000), ("USRDIR/TPPS1.PSARC", 1000),
              ("USRDIR/ARCHIVES", 40), ("USRDIR/PATCHES", 1))


def file_bytes(rel, size):
    seed = sum(rel.encode())
    return bytes((i * 31 + (i >> 9) + seed) & 0xFF for i in range(size))


TEST_DATA = {rel: file_bytes(rel, size) for rel, size in TEST_FILES}

# A disc's list: the disc's own files (GAME.PSARC starts PSAR, as a decrypted dump's does) and its update's, whose
# TPPS.PSARC is the update's although the disc has one of its own (BCES01741 has both; the game opens the update's).
TEST_DISC_FILES = (("USRDIR/GAME.PSARC", 70000), ("USRDIR/ARCHIVES", 40), ("USRDIR/TPPS.PSARC", 3000),
                   ("USRDIR/TPPS1.PSARC", 1000), ("USRDIR/PATCHES", 1))
TEST_DISC_DATA = {rel: (b"PSAR" + file_bytes(rel, size)[4:]) if rel.endswith("GAME.PSARC") else file_bytes(rel, size)
                  for rel, size in TEST_DISC_FILES}


def update_file(rel):
    return rel.upper().startswith("USRDIR/TPPS") or rel.upper() == "USRDIR/PATCHES"


def make_disc(root, title_id="BCES01741", encrypted=False):
    """A disc as extracted: <root>\\PS3_GAME with PARAM.SFO (DG, 01.00), EBOOT.BIN, the disc's files of the test's
    list and a TPPS.PSARC of its own (GAME.PSARC's first bytes are PSAR, or not for a dump never decrypted). The
    PS3_GAME folder."""
    g = Path(root) / "PS3_GAME"
    (g / "USRDIR").mkdir(parents=True)
    (g / "PARAM.SFO").write_bytes(sfo(title_id=title_id, app_ver="01.00", category="DG"))
    (g / "USRDIR" / "EBOOT.BIN").write_bytes(b"self")
    for rel, data in TEST_DISC_DATA.items():
        if not update_file(rel):
            Path(g, *rel.split("/")).write_bytes(data)
    (g / "USRDIR" / "TPPS.PSARC").write_bytes(b"PSAR" + bytes(28))
    if encrypted:
        (g / "USRDIR" / "GAME.PSARC").write_bytes(b"\x9a\x13\x77\x02" + bytes(60))
    return str(g)


def make_rpcs3(root, title_id="BCES01741", app_ver="01.12", disc=None, update=True, yml="config"):
    """An RPCS3 folder: rpcs3.exe, the disc's update in dev_hdd0\\game\\<id> (GD, GOWA.SELF), the game's data beside it
    (<id>-DATA, GD, no GOWA.SELF), and games.yml naming disc (in config\\, or the folder itself as older RPCS3s did)."""
    r = Path(root)
    r.mkdir(parents=True, exist_ok=True)
    (r / "rpcs3.exe").write_bytes(b"MZ")
    if update:
        u = r / "dev_hdd0" / "game" / title_id
        (u / "USRDIR").mkdir(parents=True)
        (u / "PARAM.SFO").write_bytes(sfo(title_id=title_id, app_ver=app_ver, category="GD"))
        (u / "USRDIR" / "GOWA.SELF").write_bytes(b"self")
        for rel, data in TEST_DISC_DATA.items():
            if update_file(rel):
                Path(u, *rel.split("/")).write_bytes(data)
    d = r / "dev_hdd0" / "game" / (title_id + "-DATA")
    (d / "USRDIR").mkdir(parents=True)
    (d / "PARAM.SFO").write_bytes(sfo(title_id=title_id, app_ver="01.00", category="GD"))
    if disc:
        y = r / "config" / "games.yml" if yml == "config" else r / "games.yml"
        y.parent.mkdir(parents=True, exist_ok=True)
        y.write_text('%s: "%s/"\n' % (title_id, disc.replace("\\", "/")))
    return str(r)


def make_game(path, elf=True, files=True, **kw):
    usr = Path(path) / "USRDIR"
    usr.mkdir(parents=True)
    (Path(path) / "PARAM.SFO").write_bytes(sfo(**kw))
    (usr / "GOWA.SELF").write_bytes(b"self")
    if elf:
        (usr / "GOWA.elf").write_bytes(b"not the real one")
    if files:
        for rel, data in TEST_DATA.items():
            Path(path, *rel.split("/")).write_bytes(data)


@unittest.skipUnless(os.name == "nt", "Windows only")
class GameFind(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="game-find-")
        cls.exe = os.path.join(cls.tmp, "t.exe")
        src = os.path.join(cls.tmp, "t.cpp")
        with open(src, "w") as f:
            f.write(DRIVER)
        sys.path.insert(0, str(ROOT / "scripts"))
        import gowa
        cls.gowa = gowa
        # The test's lists of the game's files, worked out from files on disk as `gowa.py game-files` does.
        for name, data_of in (("test_files.inc", TEST_DATA), ("test_disc_files.inc", TEST_DISC_DATA)):
            rows = []
            for rel, data in data_of.items():
                f = os.path.join(cls.tmp, "file.bin")
                Path(f).write_bytes(data)
                rows.append('{ "%s", %dull, "%s" },' % (rel, len(data), gowa.ends_sha256(f)))
            Path(cls.tmp, name).write_text("\n".join(rows) + "\n")
        cls.inc = os.path.join(cls.tmp, "test_files.inc")
        env = gowa.toolchain_env()
        clang = shutil.which("clang-cl", path=env.get("PATH"))
        if not clang:
            raise unittest.SkipTest("no clang-cl")
        cmd = [clang, "/nologo", "/O1", "/EHsc", "/std:c++17", "/I" + str(ROOT / "port"), "/I" + cls.tmp,
               '/DGAME_FILES_INC="test_files.inc"', '/DGAME_FILES_BCES01741_INC="test_disc_files.inc"', src, str(ROOT / "port" / "game_find.cpp"),
               "/Fe" + cls.exe, "/Fo" + cls.tmp + "\\"]
        r = subprocess.run(cmd, capture_output=True, text=True, env=env, cwd=cls.tmp)
        if r.returncode:
            raise AssertionError(r.stdout + r.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def setUp(self):
        self.d = tempfile.mkdtemp(prefix="tree-", dir=self.tmp)

    def run_t(self, *a):
        # The usual places RPCS3 keeps its files (%LOCALAPPDATA%\rpcs3 ...) point into the test's folder: a disc's
        # update is looked for there, and this PC's own RPCS3 must not be found.
        env = dict(os.environ, LOCALAPPDATA=self.tmp, APPDATA=self.tmp, USERPROFILE=self.tmp)
        r = subprocess.run([self.exe, *a], capture_output=True, text=True, env=env)
        self.assertEqual(r.returncode, 0, r.stderr)
        if a[0] in ("files", "needed"):
            first, _, text = r.stdout.strip().partition("\n")
            return first.split() + ([text] if text else [])
        return r.stdout.strip().split(" ", 5) if a[0] != "link" and a[0] != "target" else r.stdout.strip().split(" ", 2)

    def p(self, *parts):
        return os.path.join(self.d, *parts)

    def test_a_folder_with_no_game(self):
        self.assertEqual(self.run_t("probe", self.d)[1], "none")

    def test_this_title_with_no_elf(self):
        make_game(self.p("g"), elf=False)
        self.assertEqual(self.run_t("probe", self.p("g"))[1], "no_elf")

    def test_an_elf_of_another_build(self):
        make_game(self.p("g"))
        out = self.run_t("probe", self.p("g"))
        self.assertEqual(out[1], "other_build")
        self.assertEqual(out[3:5], ["NPUA80918", "01.12"])

    def test_another_title(self):
        make_game(self.p("g"), title_id="BLES99999", title="God of War: Ascension")
        self.assertEqual(self.run_t("probe", self.p("g"))[1], "other_title")

    def test_the_titles(self):
        # game_title.h: a folder's title when the table has it, else the default (the C++ and gowa.py alike), and the
        # ids the launcher takes for its lines. A disc the launcher does not take yet is another release to it.
        make_game(self.p("eu"), title_id="BCES01741")
        make_game(self.p("other"), title_id="BLES99999")
        out = self.run_t("title", self.p("eu"))
        self.assertEqual(out[1], "BCES01741")
        self.assertEqual(" ".join(out[2:]), "NPUA80918, BCES01741 or BCUS98232")
        self.assertEqual(self.run_t("title", self.p("other"))[1], "NPUA80918")
        self.assertEqual(self.run_t("title", self.p("nothing"))[1], "NPUA80918")
        self.assertEqual([self.gowa.title_of(self.p(d)) for d in ("eu", "other", "nothing")],
                         ["BCES01741", "NPUA80918", "NPUA80918"])
        # A disc's title in a folder that is neither its disc (DG) nor its update (GD) is not the game.
        self.assertEqual(self.run_t("probe", self.p("eu"))[1], "other_title")

    # --- a disc and its update (docs/notes/2026-11-17-eu-support-plan.md, phase 3) ---
    # pair: state, what the disc lacks (0 nothing, 1 its update, 2 the disc, 3 an ISO, 4 encrypted), APP_VER, the
    # disc's PS3_GAME, the update.

    def test_an_rpcs3_folder_gives_the_update_and_its_games_yml_the_disc(self):
        disc = make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        out = self.run_t("pair", emu)
        self.assertEqual(out[1:4], ["no_elf", "0", "01.12"])
        self.assertEqual(out[4], disc)
        self.assertEqual(out[5], os.path.join(emu, "dev_hdd0", "game", "BCES01741"))

    def test_the_update_finds_its_disc_and_an_older_rpcs3s_games_yml_too(self):
        disc = make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"), yml="root")
        out = self.run_t("pair", os.path.join(emu, "dev_hdd0", "game", "BCES01741"))
        self.assertEqual(out[1:5], ["no_elf", "0", "01.12", disc])

    def test_a_disc_finds_the_rpcs3_beside_it(self):
        # E:\PS3\Games\BCES01741 and E:\PS3\rpcs3, no games.yml
        disc = make_disc(self.p("PS3", "Games", "BCES01741"))
        make_rpcs3(self.p("PS3", "rpcs3"))
        out = self.run_t("pair", self.p("PS3", "Games", "BCES01741"))
        self.assertEqual(out[1:5], ["no_elf", "0", "01.12", disc])
        self.assertTrue(out[5].endswith(os.path.join("rpcs3", "dev_hdd0", "game", "BCES01741")), out)

    def test_a_disc_without_its_update(self):
        disc = make_disc(self.p("dumps", "BCES01741"))
        out = self.run_t("pair", self.p("dumps", "BCES01741"))
        self.assertEqual(out[1:3], ["incomplete", "1"])
        self.assertEqual(out[4:6], [disc, "-"])

    def test_an_update_without_its_disc(self):
        emu = make_rpcs3(self.p("emu"))
        out = self.run_t("pair", emu)
        self.assertEqual(out[1:3], ["incomplete", "2"])
        self.assertEqual(out[5], os.path.join(emu, "dev_hdd0", "game", "BCES01741"))

    def test_rpcs3_has_the_disc_as_an_iso(self):
        iso = self.p("isos", "God of War - Ascension (Europe).iso")
        Path(iso).parent.mkdir(parents=True)
        Path(iso).write_bytes(b"\0" * 2048)
        emu = make_rpcs3(self.p("emu"))
        Path(emu, "config").mkdir()
        Path(emu, "config", "games.yml").write_text("BCES01741: %s\n" % iso.replace("\\", "/"))
        self.assertEqual(self.run_t("pair", emu)[1:3], ["incomplete", "3"])

    def test_a_disc_never_decrypted(self):
        make_disc(self.p("dumps", "BCES01741"), encrypted=True)
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        self.assertEqual(self.run_t("pair", emu)[1:3], ["incomplete", "4"])

    def test_an_update_before_1_12(self):
        make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), app_ver="01.11", disc=self.p("dumps", "BCES01741"))
        self.assertEqual(self.run_t("pair", emu)[1:4], ["other_build", "0", "01.11"])

    # pair <folder> files: the list's count, missing, wrong, the first of them, and what the launcher says.

    def test_a_disc_and_its_update_whole(self):
        make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        out = self.run_t("pair", emu, "files")
        self.assertEqual(out[1], "no_elf")
        self.assertEqual(out[5].split("\n")[1].split(), ["5", "0", "0", "-"])

    def test_a_disc_whose_update_lacks_a_file(self):
        make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        os.remove(os.path.join(emu, "dev_hdd0", "game", "BCES01741", "USRDIR", "TPPS1.PSARC"))
        out = self.run_t("pair", emu, "files")
        self.assertEqual(out[1:3], ["incomplete", "0"])
        lines = out[5].split("\n")
        self.assertEqual(lines[1].split(), ["5", "1", "0", "USRDIR/TPPS1.PSARC"])
        self.assertEqual(lines[2], "Not every update is installed: 1 file missing, TPPS1.PSARC first. "
                                   "Install 1.04 to 1.12, in order")

    def test_a_disc_with_its_tpps_alone_is_not_the_update(self):
        # The disc's own TPPS.PSARC does not stand in for the update's.
        make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        os.remove(os.path.join(emu, "dev_hdd0", "game", "BCES01741", "USRDIR", "TPPS.PSARC"))
        lines = self.run_t("pair", emu, "files")[5].split("\n")
        self.assertEqual(lines[1].split(), ["5", "1", "0", "USRDIR/TPPS.PSARC"])

    def test_a_disc_copied_in_part(self):
        disc = make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        os.remove(os.path.join(disc, "USRDIR", "ARCHIVES"))
        lines = self.run_t("pair", emu, "files")[5].split("\n")
        self.assertEqual(lines[1].split(), ["5", "1", "0", "USRDIR/ARCHIVES"])
        self.assertEqual(lines[2], "Not the whole disc: 1 of the game's 5 files missing (40 bytes), ARCHIVES first. "
                                   "Copy all of the disc again")

    def test_a_disc_file_with_other_bytes(self):
        disc = make_disc(self.p("dumps", "BCES01741"))
        emu = make_rpcs3(self.p("emu"), disc=self.p("dumps", "BCES01741"))
        Path(disc, "USRDIR", "GAME.PSARC").write_bytes(b"PSAR" + bytes(70000 - 4))
        lines = self.run_t("pair", emu, "files")[5].split("\n")
        self.assertEqual(lines[1].split(), ["5", "0", "1", "USRDIR/GAME.PSARC"])
        self.assertEqual(lines[2], "1 of the disc's files is not as the disc has it, GAME.PSARC first. "
                                   "Copy all of the disc again")

    def test_what_a_disc_cannot_start_without(self):
        for path, title, want in (("USRDIR/GAME.PSARC", "BCES01741", "1"), ("USRDIR/TPPS1.PSARC", "BCES01741", "1"),
                                  ("USRDIR/ENGLISH.PSARC", "BCES01741", "0"), ("USRDIR/ENGLISH.PSARC", "NPUA80918", "1"),
                                  ("USRDIR/GAME.PSARC", "BCUS98232", "0")):
            with self.subTest((path, title)):
                self.assertEqual(self.run_t("needed", path, title)[1], want)

    def test_a_search_pairs_a_disc_and_an_update_it_saw_apart(self):
        # The disc in one place, an RPCS3 with its update elsewhere, no games.yml: the search sees both and pairs them.
        disc = make_disc(self.p("a", "games", "BCES01741"))
        make_rpcs3(self.p("b", "emulators", "RPCS3"))
        out = self.run_t("pairsearch", self.d)
        self.assertEqual(out[1:5], ["no_elf", "0", "01.12", disc])

    def test_a_broken_param_sfo(self):
        (Path(self.d) / "PARAM.SFO").write_bytes(b"\0PSF" + b"\xff" * 40)
        self.assertEqual(self.run_t("probe", self.d)[1], "none")

    def test_what_a_player_might_pick(self):
        make_game(self.p("emu", "dev_hdd0", "game", "NPUA80918"))
        for pick in (("emu",), ("emu", "dev_hdd0"), ("emu", "dev_hdd0", "game"),
                     ("emu", "dev_hdd0", "game", "NPUA80918"), ("emu", "dev_hdd0", "game", "NPUA80918", "USRDIR")):
            out = self.run_t("locate", self.p(*pick))
            self.assertEqual(out[1], "other_build", pick)
            self.assertTrue(out[2].endswith("NPUA80918"), out)

    def test_a_disc_layout(self):
        make_game(self.p("dump", "PS3_GAME"))
        self.assertTrue(self.run_t("locate", self.p("dump"))[2].endswith("PS3_GAME"))

    def test_the_search_finds_it_deep_down_and_counts_copies(self):
        # The copy deep down is the one to take: a renamed copy with another GOWA.elf in it comes after one still to
        # decrypt (game_rank), and another game does not count.
        make_game(self.p("a", "b", "c", "d", "e", "RPCS3", "dev_hdd0", "game", "NPUA80918"), elf=False)
        make_game(self.p("x", "renamed game"))
        make_game(self.p("y", "BLES00000"), title_id="BLES00000", title="Another game")
        out = self.run_t("search", self.d)
        self.assertEqual(out[1], "no_elf")
        self.assertTrue(out[2].endswith("NPUA80918"), out)
        self.assertEqual(out[5], "2")

    def test_two_releases_on_one_pc(self):
        # The US digital release and the EU disc with its update in the same RPCS3: two copies (the disc once, though
        # the walk comes to its folder and to its update's), and the search keeps the one further along.
        make_game(self.p("emu", "dev_hdd0", "game", "NPUA80918"), elf=False)
        make_disc(self.p("discs", "BCES01741"))
        make_rpcs3(self.p("emu"), disc=self.p("discs", "BCES01741"))
        out = self.run_t("search", self.d)
        self.assertEqual(out[1], "no_elf")
        self.assertIn(out[3], ("NPUA80918", "BCES01741"))
        self.assertEqual(out[5], "2")
        # The disc without its update: the US release is the one to take.
        shutil.rmtree(self.p("emu", "dev_hdd0", "game", "BCES01741"))
        out = self.run_t("search", self.d)
        self.assertEqual(out[1:4], ["no_elf", self.p("emu", "dev_hdd0", "game", "NPUA80918"), "NPUA80918"])
        self.assertEqual(out[5], "2")

    def test_a_disc_with_an_old_update_and_a_new_one(self):
        # Two RPCS3s with the disc's update, one at 1.11: the search ends on the pair with 1.12.
        disc = make_disc(self.p("discs", "BCES01741"))
        make_rpcs3(self.p("old"), app_ver="01.11")
        new = make_rpcs3(self.p("new"))
        out = self.run_t("pairsearch", self.d)
        self.assertEqual(out[1:5], ["no_elf", "0", "01.12", disc])
        self.assertEqual(out[5], os.path.join(new, "dev_hdd0", "game", "BCES01741"))

    def test_a_disc_picked_takes_its_update_at_1_12(self):
        # Browse to the disc, two RPCS3s beside it, the one walked first at 1.11.
        disc = make_disc(self.p("PS3", "Games", "BCES01741"))
        make_rpcs3(self.p("PS3", "a-old"), app_ver="01.11")
        new = make_rpcs3(self.p("PS3", "b-new"))
        out = self.run_t("pair", self.p("PS3", "Games", "BCES01741"))
        self.assertEqual(out[1:5], ["no_elf", "0", "01.12", disc])
        self.assertEqual(out[5], os.path.join(new, "dev_hdd0", "game", "BCES01741"))

    def test_the_search_goes_past_another_version(self):
        # As the launcher searches, stopping at the first copy of this release: a copy at 1.11, walked first, is not
        # one, and the search goes on to the copy at 1.12.
        make_game(self.p("a", "NPUA80918"), elf=False, app_ver="01.11")
        make_game(self.p("b", "NPUA80918"), elf=False)
        out = self.run_t("search_first", self.d)
        self.assertEqual(out[1], "no_elf")
        self.assertTrue(out[2].endswith(os.path.join("b", "NPUA80918")), out)
        self.assertEqual(out[5], "2")

    def test_the_games_data_folder_is_no_game(self):
        # RPCS3 keeps NPUA80918-DATA beside the game: this title's PARAM.SFO, and no GOWA.SELF or GOWA.elf.
        data = Path(self.p("NPUA80918-DATA"))
        (data / "USRDIR").mkdir(parents=True)
        (data / "PARAM.SFO").write_bytes(sfo())
        self.assertEqual(self.run_t("probe", str(data))[1], "none")

    def test_the_search_takes_the_game_beside_its_data_folder(self):
        # As the launcher searches, stopping at the first copy of this release: the data folder, walked first, is
        # not one.
        game = self.p("RPCS3", "dev_hdd0", "game", "NPUA80918")
        make_game(game, elf=False)
        data = Path(game + "-DATA")
        (data / "USRDIR").mkdir(parents=True)
        (data / "PARAM.SFO").write_bytes(sfo())
        out = self.run_t("search_first", self.d)
        self.assertEqual(out[1], "no_elf")
        self.assertTrue(out[2].endswith("NPUA80918"), out)

    def test_the_three_files_alone_are_not_the_game(self):
        # PARAM.SFO and GOWA.SELF, as a license would decrypt: nothing to decrypt or build from.
        make_game(self.p("g"), elf=False, files=False)
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:5], ["incomplete", "5", "5", "0"])
        self.assertEqual(out[5], str(sum(size for _, size in TEST_FILES)))
        self.assertEqual(out[6], "USRDIR/GAME.PSARC")
        self.assertEqual(out[9], "Not the whole game: 5 of its 5 files missing (271041 bytes), GAME.PSARC first. "
                                 "Install the game again")

    def test_a_decrypted_elf_does_not_make_up_for_them(self):
        make_game(self.p("g"), files=False)
        self.assertEqual(self.run_t("files", self.p("g"))[1], "incomplete")

    def test_one_file_missing(self):
        make_game(self.p("g"))
        os.remove(self.p("g", "USRDIR", "ARCHIVES"))
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:6], ["incomplete", "5", "1", "0", "40"])
        self.assertEqual(out[6], "USRDIR/ARCHIVES")

    def test_a_file_cut_short(self):
        make_game(self.p("g"))
        f = Path(self.p("g", "USRDIR", "ENGLISH.PSARC"))
        f.write_bytes(f.read_bytes()[:-1])
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:5], ["incomplete", "5", "0", "1"])
        self.assertEqual(out[7], "USRDIR/ENGLISH.PSARC")
        self.assertEqual(out[9], "1 of the game's files is not as version 1.12 has it, ENGLISH.PSARC first. "
                                 "Install the game again")

    def test_the_update_missing(self):
        # Issue #2: the base game's data under 1.12's EBOOT, without PATCHES or the TPPS archives it names.
        make_game(self.p("g"))
        os.remove(self.p("g", "USRDIR", "TPPS1.PSARC"))
        os.remove(self.p("g", "USRDIR", "PATCHES"))
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:5], ["incomplete", "5", "2", "0"])
        self.assertEqual(out[6], "USRDIR/TPPS1.PSARC")
        self.assertEqual(out[9], "Not every update is installed: 2 files missing, TPPS1.PSARC first. "
                                 "Install 1.04 to 1.12, in order")

    def test_an_update_file_of_another_version(self):
        # The base game's own TPPS archive is another, much smaller one.
        make_game(self.p("g"))
        Path(self.p("g", "USRDIR", "TPPS1.PSARC")).write_bytes(b"PSAR" + bytes(108))
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:5], ["incomplete", "5", "0", "1"])
        self.assertEqual(out[9], "Not every update is installed: 1 file differs, TPPS1.PSARC first. "
                                 "Install 1.04 to 1.12, in order")

    def test_the_game_again_when_one_of_its_own_is_missing_too(self):
        make_game(self.p("g"))
        os.remove(self.p("g", "USRDIR", "ENGLISH.PSARC"))
        os.remove(self.p("g", "USRDIR", "PATCHES"))
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1:5], ["incomplete", "5", "2", "0"])
        self.assertEqual(out[9], "Not the whole game: 2 of its 5 files missing (70001 bytes), ENGLISH.PSARC first. "
                                 "Install the game again")

    def test_a_file_of_its_size_with_other_bytes(self):
        for rel, size in TEST_FILES:
            with self.subTest(rel):
                g = self.p(rel.replace("/", "_"))
                make_game(g)
                Path(g, *rel.split("/")).write_bytes(bytes(size))
                out = self.run_t("files", g)
                self.assertEqual(out[1:5], ["incomplete", "5", "0", "1"])
                self.assertEqual(out[7], rel)

    def test_its_last_byte_counts(self):
        make_game(self.p("g"))
        f = Path(self.p("g", "USRDIR", "GAME.PSARC"))
        b = bytearray(f.read_bytes())
        b[-1] ^= 1
        f.write_bytes(bytes(b))
        self.assertEqual(self.run_t("files", self.p("g"))[1], "incomplete")

    def test_whole_and_not_decrypted(self):
        make_game(self.p("g"), elf=False)
        self.assertEqual(self.run_t("files", self.p("g"))[1:5], ["no_elf", "5", "0", "0"])

    def test_another_version_is_said_before_its_files(self):
        make_game(self.p("g"), elf=False, files=False, app_ver="01.00")
        out = self.run_t("files", self.p("g"))
        self.assertEqual(out[1], "other_build")
        self.assertEqual(out[8], "1")

    def test_the_search_takes_a_whole_copy_over_one_that_is_not(self):
        # Walked first, the copy with the three files alone would have ended the search before.
        make_game(self.p("a", "NPUA80918"), elf=False, files=False)
        make_game(self.p("b", "NPUA80918"), elf=False)
        out = self.run_t("search_first", self.d)
        self.assertEqual(out[1], "no_elf")
        self.assertTrue(out[2].endswith(os.path.join("b", "NPUA80918")), out)
        self.assertEqual(out[5], "2")

    def test_what_the_game_cannot_start_without(self):
        for path, want in (("USRDIR/GAME.PSARC", "1"), ("usrdir\\game.psarc", "1"), ("USRDIR/ARCHIVES", "1"),
                           ("USRDIR/PATCHES", "1"), ("USRDIR/TPPS1-ENGLISH.PSARC", "0"), ("USRDIR/GAME.PSARC.PATCH", "0"),
                           ("USRDIR", "0"), ("", "0")):
            with self.subTest(path):
                self.assertEqual(self.run_t("needed", path)[1], want)

    def test_which_files_come_with_the_update(self):
        for rel, want in (("USRDIR/TPPS1.PSARC", True), ("usrdir\\tpps.psarc", True), ("USRDIR/TPPS4-ENGLISH.PSARC", True),
                          ("USRDIR/PATCHES", True), ("USRDIR/PATCHES.BAK", False), ("USRDIR/GAME.PSARC", False),
                          ("USRDIR/ARCHIVES", False)):
            with self.subTest(rel):
                self.assertEqual(self.gowa.game_file_update(rel), want)

    def test_the_list_reads_back(self):
        self.assertEqual(self.gowa.read_game_files_inc(self.inc),
                         [(rel, len(data), self.gowa.ends_sha256(self.p_file(rel, data))) for rel, data in TEST_DATA.items()])

    def p_file(self, rel, data):
        f = self.p(rel.replace("/", "_") + ".bin")
        Path(f).write_bytes(data)
        return f

    def test_the_search_goes_past_links(self):
        make_game(self.p("real", "g"))
        os.makedirs(self.p("l"))
        self.assertEqual(self.run_t("link", self.p("l", "loop"), self.d)[1], "1")
        self.assertEqual(self.run_t("search", self.d)[5], "1")

    def test_a_link_is_made_read_and_replaced(self):
        make_game(self.p("one"))
        make_game(self.p("two"))
        link = self.p("deep", "er", "PS3_GAME")
        self.assertEqual(self.run_t("link", link, self.p("one"))[1], "1")
        self.assertTrue(os.path.isfile(os.path.join(link, "PARAM.SFO")))
        self.assertEqual(self.run_t("target", link)[2], self.p("one"))
        self.assertEqual(self.run_t("link", link, self.p("two"))[1], "1")
        self.assertEqual(self.run_t("target", link)[2], self.p("two"))
        self.assertTrue(os.path.isfile(self.p("one", "PARAM.SFO")), "what it led to stays")

    def test_a_folder_with_files_stays(self):
        os.makedirs(self.p("mine"))
        Path(self.p("mine", "save.dat")).write_bytes(b"x")
        make_game(self.p("g"))
        self.assertEqual(self.run_t("link", self.p("mine"), self.p("g"))[1], "0")
        self.assertTrue(os.path.isfile(self.p("mine", "save.dat")))

    def test_an_empty_folder_gives_way(self):
        os.makedirs(self.p("empty"))
        make_game(self.p("g"))
        self.assertEqual(self.run_t("link", self.p("empty"), self.p("g"))[1], "1")
        self.assertTrue(os.path.isfile(self.p("empty", "PARAM.SFO")))

    def test_the_game_in_the_links_place_needs_no_link(self):
        make_game(self.p("here"))
        self.assertEqual(self.run_t("link", self.p("here"), self.p("here"))[1], "1")
        self.assertTrue(os.path.isfile(self.p("here", "PARAM.SFO")))


if __name__ == "__main__":
    unittest.main()
