# The mods folder, step 1: a folder, a tab, and files over USRDIR (2026-11-18)

The aim: a mod is dropped into a folder and works, with no setting to write by hand. Before this, `paths.mods` named one
folder laid out like the PS3's paths (`dev_hdd0\game\NPUA80918\USRDIR\...`), so there was one mod at a time, its paths
named one release, and only the settings file could set it. Step 1 of the plan:

1. The mods folder, the Mods tab, and files over USRDIR (this note).
2. Entries inside the archives (`archive\`) and the game's text (`text\`), packed by the launcher into one more update.
3. Textures replaced by a hash of the game's picture (`textures\`): the Xbox button icons of #21 as the first.
4. Maybe: RPCS3's `patch.yml` (`patches\`), which changes the game's code and so its build.

## What a mod is

- **The folder.** `mods` beside the settings file (`build\mods\` in a build tree, beside `ascension-r.ini`), or
  `[mods] folder`. `GOWA_MODS_DIR` wins over both, for the tests. Each folder or `.zip` in it is a mod; a name that
  starts with a dot is not (`.unpacked`).
- **Its files** stand for the game's USRDIR. `files\` is the layout this project writes; `USRDIR\`, `PS3_GAME\USRDIR\`
  and `dev_hdd0\game\<id>\USRDIR\` are taken as mods for RPCS3 come, and so is a folder with its files right in it.
  `dev_hdd0` itself in the mods folder is one mod: what `gowa.py text build` wrote before.
- **Its own folder.** A mod's folder may hold one folder and a read-me (a zip of a folder, extracted into a folder of
  its own): the mod is the folder inside, two down at most. Not when the one folder is one of the game's own
  (`build`, `data`, `ICONS`): then it is laid out as USRDIR is.
- **mod.ini**: `[mod]` name, version, author, description. Without a name, the folder's.
- **A zip** is unpacked into `mods\.unpacked\<name>.zip\`, then put in place of the copy before, and a stamp keeps its
  size and time: the next start reads the copy. Stored and Deflate (the port's zlib); names that lead out of the
  folder, a drive, or anything Windows would not keep as it is (`CON`, a name ending in a dot) refuse the zip, and so do
  encrypted, Zip64 and split zips.
- **The order** is the settings': `[mods] order`, the names as the folder has them, `|` between them (no Windows
  name has one), then the new ones by name. `off` lists the ones switched off, and `use` switches them all.

## The game's side

`PS3_MODS_DIRS` (ps3recomp's `runtime/syscalls/vfs_mods.h`): the USRDIR folders of the mods that are on, `|` between
them, the first one first. The three path translators ask it first, then `PS3_MODS_ROOT` (`paths.mods`), then the
game's folders.

- **One list for each translator.** At the first lookup, each folder's files are listed and sorted. A lookup is then
  a binary search: no file is read from the disk to find a mod's file, and one that is not there costs nothing
  either. It stays header-only, as before.
- **Which guest paths.** `/dev_bdvd/PS3_GAME/USRDIR/...` and `/dev_hdd0/game/<title>/USRDIR/...`, the title being
  `PS3_TITLE`'s, which main.cpp sets.
- **Not the game data folder.** The first game run took the title from the path, any title. The EU disc's log then
  had `[fs] mod '/dev_hdd0/game/BCES01741-DATA/USRDIR/PATCHES'`, and the game opened `PATCHES` from there. It looks for
  `PATCHES` in its game data folder (`BCES01741-DATA`, which `cellGameDataCheck` makes for the DCDS and MOTD files)
  before its own. The file was the game's own `5`, so nothing showed, but a mod's `PATCHES` must not stand for that
  folder's. Now the title is `PS3_TITLE`'s, and without it, a name as titles are named (four capitals, five
  digits), never `<title>-DATA`.

## The launcher

The Mods tab, between Cheats and Trophies: **Use mods**, the folder with **Open folder** (which makes it with a
read-me), then each mod. A mod's row has its name, version, author and zip, two lines of its description, and what it
gives: its file count (not its `mod.ini` or read-me), how many of them a mod above it has too and which one, the
folders this build does not read yet, or why none of it is read. Its switch, up and down are on the right. A change
goes into the settings at once and the folder is read again for the counts; Save or Play writes it. The folder is
read again when the tab is shown and when the window comes back to the front.

The problem report has a `Mods:` line, the log's `[mods]` line, with no paths; `[mods] folder` is hidden as
`<mods folder>`.

`gowa.py text build` now writes a mod: `build\mods\translation\files\TPPS6.PSARC`, `PATCHES` and a `mod.ini`
(`--name` for another folder).

## The tests

- **`tests/test_mods.py`** (in CI): the layouts, a folder in a folder, the old text tool's `dev_hdd0`, the order with
  a name gone and a name in another case, the counts of what a mod above has, Use mods off, what the tab writes, a zip
  unpacked once (its file's time the same after a second scan) and again when it changes (an old file gone), zips that
  are refused and nothing written outside the folder, no folder at all. And ps3recomp's `tests/test_vfs_mods.c`,
  built and run twice, with `PS3_TITLE` and without: the first folder winning, the disc's USRDIR and the title's,
  not another title's nor `<title>-DATA`, names in another case, `PS3_MODS_ROOT` after them. 9 tests, all pass.
- **The tab** (GOWA_LAUNCHER_SHOT, a made-up mods folder of five: a mod.ini and `files\`, a translation laid out
  loose under it with a read-me, a zip of a folder, one switched off, one with `textures\` alone): the order by name
  with the zip among the folders, "2 files, 2 of them from Indonesian menus, above" in the warning colour, the one
  switched off dimmed, the first one's up and the last one's down greyed, "Not read: this build does not read its
  textures yet".
- **The game**, three runs of the parasite-fight replay on the EU disc, with two mods that each have `files\PATCHES`
  holding the game's own `5`, and one more file in the second:
  - the log: `[mods] First 1.0 (1 file), Second (2 files, 1 from a mod above)`, then
    `[fs] mod '/dev_hdd0/game/BCES01741/USRDIR/PATCHES' -> '.../First/files/PATCHES'` and that file opened. The
    updates opened as before (`TPPS.PSARC`, `TPPS1` to `TPPS5`).
  - the first run is the one with `BCES01741-DATA` above; the second and third, after the fix, have no such line.
  - 40 to 48 flips a second in the fight from 62 s, and at 78 s the picture of Megaera as it should be.
