# Plan: the EU disc (and the other versions) from start to finish (2026-11-17)

Issue #4 asks for the EU disc, BCES01741; one comment for the US disc, BCUS98232. What is known is in
[the EU disc against our GOWA.elf](2026-11-17-eu-disc-gowa-self.md): at 1.12 the EU game's `GOWA.elf` is ours byte
for byte, so the game's code, the Kit's DLL, the hooks, the patches and the native SPU work stay as they are. This plan
is everything else: the disc layout, the launcher, decrypting an EU copy without keys, the files that name NPUA80918,
tests, and the release. About five to seven days of work; the user's EU copy on this PC lets every step be tested here.

## Goals and what is out

- **In:** BCES01741 at 1.12, from an extracted disc (or RPCS3's disc folder) and RPCS3's update folder, found,
  decrypted, built and played from the launcher as NPUA80918 is; its ten languages; its saves.
- **Then, cheaply, from the same work:** BCUS98232 (same layout; needs one check of its 1.12 `GOWA.elf`), other disc
  SKUs the ELF names (BCES01742, BCES01838, BCJS37008, BCAS25016) once someone checks one, NPEA00445 (EU digital,
  a `.rap` like NPUA80918) once someone checks it.
- **Out for now:** encrypted (Redump, not decrypted) ISOs; moving saves between regions; reading ISOs directly (Phase 8).

## What is known (all measured on the user's copy)

| | NPUA80918 (US digital) | BCES01741 (EU disc) |
|---|---|---|
| base files | `dev_hdd0/game/NPUA80918/USRDIR` | the disc's `PS3_GAME/USRDIR` (extracted, or RPCS3's `games.yml` names the folder) |
| update to 1.12 | in the same folder | `dev_hdd0/game/BCES01741` (RPCS3 installs the PKGs A0104 to A0112 there) |
| executable | `USRDIR/GOWA.SELF`, started by a 71 KB `EBOOT.BIN` | the same two files, from the update folder |
| `GOWA.SELF` license | NPDRM local: the `.rap` | NPDRM free: a KLIC held in plain bytes in the 71 KB `EBOOT.BIN` |
| `GOWA.elf` 1.12 | `4bf75c89...62b5` | the same file |
| PARAM.SFO | HG | disc DG 01.00; update GD, APP_VER 01.12 |
| saves | `BCUS98232-*` | `BCES01741-*` (`-SYSDATA`, `-PROFDATA-n`) |
| game data | `NPUA80918-DATA` | `BCES01741-DATA` (0.1 MB) |
| languages | English, French, LA Spanish, BR Portuguese | English, French, Spanish, German, Italian, Portuguese, Russian, Polish, Turkish, Greek |
| patch archives | `TPPS*.PSARC`, `PATCHES` | the same scheme, EU languages (`TPPS-GERMAN`, ...) |

- The runtime already boots a disc game: `cellGame_init_from_paramsfo` reads `<vfs>/PS3_GAME/PARAM.SFO`, CATEGORY DG
  gives the disc type and `/dev_bdvd/PS3_GAME` paths, and `/dev_bdvd/` maps to `paths.vfs`.
- **Decrypting without keys works.** `rpcs3 --decrypt` opens the update's `EBOOT.BIN` with no key (free license). Of
  its file-backed data, the 8-aligned 16-byte windows that look like a key (12 or more different bytes, at most 10
  printable) are 2; given to `rpcs3 --decrypt GOWA.SELF` on standard input (it asks `Enter KLIC` and reads stdin, not
  the console), the second one opened it in 0.8 s and the result was `4bf75c89...`. No key is in the port; the key is
  read from the player's own `EBOOT.BIN` each time, as RPCS3 itself learns it when the game runs.

## Phase 1: the EU game runs from a layout made by hand (1 day)

Prove the game itself before building the launcher around it. Dev build, a test hdd0, nothing of the player's written.

- A test setup: `paths.vfs` = the extracted disc (`E:\PS3\Games\BCES01741`), `paths.hdd0` = a new test hdd0 whose
  `game\BCES01741` is a junction to `E:\PS3\rpcs3\dev_hdd0\game\BCES01741` (removed as a link after), `PS3_TITLE`
  BCES01741, `PS3_BOOT_PATH` `/dev_hdd0/game/BCES01741/USRDIR/GOWA.SELF`.
- `port/main.cpp`: take `PS3_TITLE` from the vfs PARAM.SFO's TITLE_ID and build the boot path from it (both kinds
  boot `/dev_hdd0/game/<id>/USRDIR/GOWA.SELF`), instead of the NPUA80918 constants; `game_open_failed` checks
  `/dev_bdvd/PS3_GAME/` and `/dev_hdd0/game/<id>/`.
- `cellGame.c`: for a disc game with an update, APP_VER and VERSION from the update's PARAM.SFO (`/dev_hdd0/game/<id>/
  PARAM.SFO`, 01.12), as the PS3 reports them; CATEGORY and TITLE_ID stay the disc's. Check what the game asks
  (`cellGamePatchCheck` answers "not a patch", as it should for a booted title).
- Watch the log for: the update's `PATCHES` and `TPPS*` opened from `/dev_hdd0/game/BCES01741/USRDIR`, the base
  archives from `/dev_bdvd`, `BCES01741-DATA` made, saves as `BCES01741-*`, no failed opens.
- **Done when:** title screen, a new game through the opening chapter's first fight, a save made and loaded after a
  restart, the game in French and in German (`system.language`), screenshots clean.

**Done (2026-11-17).** The dev build on the user's EU copy: a vfs folder of ours whose `PS3_GAME` is a junction to the
extracted disc's, a test hdd0 with a copy of the update folder, nothing else set.

- **What was missing:** `cellGamePatchCheck` always answered "not a patch". The game's sequence is the one RPCS3
  logs (BootCheck, ContentPermit, DataCheck of `BCES01741-DATA` and of `BCES01741`, PatchCheck, ContentPermit); with
  "not a patch" it looked for `PATCHES` in the disc's USRDIR, ran 1.12's code on 1.00's data and sat on a loading
  screen. RPCS3 answers a GD boot (the update's PARAM.SFO) as a patch. Now `cellGame.c` does too: a disc game whose
  `PS3_BOOT_PATH` is under `/dev_hdd0/game/<id>/` and whose PARAM.SFO there says GD is a patch boot: BootCheck gives
  the PATCH attribute, PatchCheck answers OK and the next ContentPermit gives `/dev_hdd0/game/<id>`, APP_VER and
  VERSION come from the update (01.12). `main.cpp` takes `PS3_TITLE` and the boot path from the vfs PARAM.SFO.
- **What runs:** the base archives from `/dev_bdvd/PS3_GAME/USRDIR`, `PATCHES` and `TPPS*` from the update, as in
  RPCS3; the menu shows `01.12F0.1`; a new game on Normal into `R_MEGAERA03`, its cinematic and the first fight
  (Megaera, 44 to 54 flips/s at 1600x900 here), saves `BCES01741-SYSDATA` and `BCES01741-PROFDATA-0`; after a
  restart the profile shows its time and Continue Game loads the checkpoint. French and German: their archives and
  their text, from a new hdd0 each.
- **Learnt:**
  - The first boot asks **LANGUAGE: Triangle Greek, Cross Default**: that is how Greek is chosen (Phase 5's question).
  - The game keeps the language its first start had (in `BCES01741-SYSDATA`): a later `system.language` changes the
    boot logos' language only. The launcher should say so beside the setting.
  - On the first boot the opening movie cannot be skipped; later ones can.
  - **Never make a player's disc folder the vfs root.** The first try did, and the runtime wrote into it: the
    game's FIOS cache (`/dev_hdd1`, 1 GB, mapped under the vfs root when `PS3_HDD1_ROOT` is not set) and
    ContentPermit's folders under `<vfs>/game` (now only made when `PS3_HDD0_ROOT` is not set). Both were moved to the
    Recycle Bin. The launcher's own vfs folder with a `PS3_GAME` junction (Phase 3) keeps every write in our folder.
- **Not done here:** finishing the first fight (its tutorial wants the right stick, which the test input lacks).

## Phase 2: one table of titles (1 day)

Replace the single title with a table, so every place that says NPUA80918 asks it instead.

- `port/game_title.h` (new): for each known title, its id, kind (digital or disc), where the update lives, the
  license (`.rap` name, or the KLIC from `EBOOT.BIN`), save prefix, the file list to check, and whether it is enabled:
  NPUA80918 (on), BCES01741 (on after Phase 1), BCUS98232, NPEA00445 and the other disc ids (off until checked).
- The places: `game_find.cpp` (`k_title_id`, `k_title_dir`, the folder candidates), `game_prepare.cpp`
  (`k_rap_name`), `launcher.cpp` (texts that say "This port runs NPUA80918 1.12", the game row), `main.cpp`,
  `precompile.cpp` (the archive folders: the update's `USRDIR`, then the disc's), `scripts/gowa.py` (`TITLE_ID`,
  `game-files`, `kit_train`'s junction, `--game-dir`), `scripts/gametext.py` (the mod folder `dev_hdd0/game/<id>`).
- The chosen title lands in the settings (`[game] title`), written by the launcher with the paths.
- **Done when:** NPUA80918 behaves exactly as before (tests, a bench run, the Kit's DLL hash unchanged) and the
  Phase 1 setup runs from settings alone.

**Done (2026-11-17).**

- `port/game_title.h`: NPUA80918 (US digital, on), BCES01741 (EU disc) and BCUS98232 (US disc), off for the launcher
  until it finds and decrypts a disc (phases 3 and 4); each with its kind and its `.rap` (none for a disc).
  `game_title_find`, `game_titles_text` (the ids the launcher takes, for its lines), and in `game_find.cpp`
  `game_title_of_folder` / `game_title_of_vfs`. `scripts/gowa.py` has the same table (`TITLES`, `title_of`);
  `scripts/gametext.py`, which gowa.py imports, keeps a list of the ids and its own reader.
- **No `[game] title` setting**, as planned at first: the title is what PARAM.SFO under `<vfs>\PS3_GAME` names, the
  folder the launcher links, read there by `main.cpp`, `precompile.cpp`, the launcher and `kit_train`. One place,
  nothing to fall out of step.
- What asks it now: `game_find.cpp` (the titles taken, the folders tried, the search's hint), `game_prepare.cpp`
  (the `.rap` by title; a title without one is not decrypted that way), the launcher (the hdd0 link is
  `<hdd0>\game\<id>` of the folder linked or picked, and its lines name the ids it takes), `main.cpp`,
  `precompile.cpp`, `gowa.py` (`doctor`, `game-files` and `setup` for a digital release, `kit_train`'s link and
  their removal), `gametext.py` (the mod folder).
- `precompile.cpp` now takes every PSARC of its folders, one per name from the first folder that has it: a disc's
  base archives and its update's TPPS ones. For NPUA80918 the two folders are one and the list is the same.
- **Checked:** the port's tests (`test_game_find`: 31, one for the table); NPUA80918's precompile read no archive
  (its list of 6,911 programs is keyed by the archives' count and bytes, so the list is the same) and the fight bench
  as before; the EU copy's precompile, from settings alone, read its 83 archives (the disc's and the update's) and
  found 6,907 vertex programs, all already in the cache NPUA80918's run had made. The game's DLL is built from none of
  the files changed.

## Phase 3: the launcher finds an EU install (1 to 1.5 days)

- **From RPCS3:** `config/games.yml` names each disc's folder (`BCES01741: E:/PS3/Games/BCES01741/`), and the same
  RPCS3's `dev_hdd0/game/BCES01741` holds the update. `game_search` already visits RPCS3 folders; read `games.yml`
  there and in `%APPDATA%\rpcs3`.
- **From the disks:** a folder with `PS3_GAME/PARAM.SFO` saying BCES01741 and DG is a disc; its update is looked for
  in every RPCS3 found. **Browse** takes either folder and finds the other.
- **States the game row says:** found and 1.12; disc without update ("install updates 1.04 to 1.12 in RPCS3"); update
  older than 1.12; disc files missing (the file list, Phase 5); only an ISO ("extract it: right-click, Mount, copy
  everything to a folder", until Phase 8); an encrypted ISO or folder (the PSARCs do not start with `PSAR`: "not
  decrypted, not supported").
- **Links, as for NPUA80918:** `<vfs>\PS3_GAME` to the disc's `PS3_GAME`, `<hdd0>\game\BCES01741` to the update
  folder; both only read. A PC with two versions: the row lists both and the player picks.
- **Done when:** a new portable Kit on this PC finds the EU install by itself (through `E:\PS3\rpcs3`) and by Browse to
  either folder, and each wrong state above, made in a test folder, shows its message.

**Done (2026-11-17).** BCES01741 is on in both tables.

- `GameInfo` has the update (`update`), whether it is a disc, PARAM.SFO's CATEGORY, and for an incomplete disc what
  it lacks (`DiscLack`: the update, the disc, an ISO, encrypted). `game_probe_pair(dir, update)` is `game_probe` with
  the update given (the launcher's hdd0 link). A disc's PS3_GAME (DG) finds its update in an RPCS3 noted
  (`game_note_rpcs3`; a search notes every folder holding rpcs3.exe), the usual places (`%LOCALAPPDATA%\rpcs3` ...),
  one beside the disc (a folder with rpcs3.exe in, or next to, the disc's folder or the three above it), or one a
  search saw; an update (GD with GOWA.SELF; a GD folder without it is the game's data) finds its disc in its RPCS3's
  `config\games.yml` (older RPCS3s: `games.yml`), another RPCS3's, or one a search saw. An encrypted disc is told by
  GAME.PSARC not starting with `PSAR`; APP_VER is the update's.
- The launcher links `<vfs>\PS3_GAME` to the disc's PS3_GAME and `<hdd0>\game\<id>` to the update, and the folder
  line shows both; an update found without its disc, or a disc RPCS3 has as an ISO, is said and nothing is linked.
- `Ascension-R --find-game [folder]`: what Auto detect (every disk) or Browse (that folder) comes to, a line each.
- **Checked:** `test_game_find` (40: nine for discs and updates, with made-up RPCS3 folders and the usual places moved
  into the test's folder); `--find-game` on this PC's EU copy from `E:\PS3\rpcs3`, `E:\PS3\Games\BCES01741` and the
  update folder, and every disk of E: (125 s); a fresh settings file: the launcher found and linked the EU copy by
  itself (E: is searched first here) and shows `BCES01741 · 1.12`; the dev settings still show NPUA80918 1.12 as
  ready.
- **Not done:** listing two copies for the player to pick (Auto detect takes the first one it can set up; Browse picks
  the other); the disc's file list (phase 5). Until phase 4 the System tab still asks a disc for a `.rap`.

## Phase 4: decrypting an EU copy without keys (1 day)

`game_prepare.cpp`, for a disc title:

1. Copy the update's `EBOOT.BIN` and `GOWA.SELF` to the work folder (never write beside the game or in RPCS3).
2. `rpcs3 --decrypt EBOOT.BIN`: free license, no key.
3. From `EBOOT.elf`, the KLIC candidates (rule above), kept in memory and in a stdin file inside the work folder only.
4. `rpcs3 --decrypt GOWA.SELF` with the candidates on standard input; the result must hash to `4bf75c89...`.
5. If none fits: every 8-aligned window of the data (about 9,000 here; a few seconds), then the RPCS3 log
   (`log/RPCS3.log`, `RPCS3.log.gz`, the KLIC beside `drm_path=".../GOWA.SELF"`) if the player has started the game
   in that RPCS3, then a message asking them to start it once in RPCS3 and press **Try again**.
6. Remove the work folder; log the step's result, never a key or a candidate.

- Same RPCS3 handling as now: `RPCS3_CONFIG_DIR` at a folder of ours, the downloaded RPCS3 or the player's.
- **Done when:** on this PC, from a clean Kit, the EU copy is decrypted with no player step, the DLL the Kit builds is
  `f317f78f...` (the same as from NPUA80918), with RPCS3 0.0.43-20572 and -20607 and the one the launcher downloads.

**Done (2026-11-17).** `game_prepare_disc(update, rpcs3, data)` does the steps above; `run_hidden_in` gives RPCS3 a
standard input from a file and its output to NUL. The windows (`klic_windows`): 8-aligned, 16 bytes, from the
PT_LOAD segments without PF_X, each once; narrow keeps 12 or more different bytes with at most 10 printable, wide
every window with more than 3 different bytes. The RPCS3 log (`klic_logged`): every `KLIC=0x` of `log\RPCS3.log`
(older RPCS3s: `RPCS3.log`). Logged: the step, how many keys of which kind, the time; never a key.
`GOWA_KLIC_TRIES=narrow,wide,log` picks the tries, for tests. The launcher decrypts a disc with the RPCS3 its update is
in, never asks it for a `.rap`, and says so; `--prepare-game` takes a disc, its PS3_GAME or its update folder.

| on this PC's EU copy | keys tried | GOWA.SELF | in all |
|---|---|---|---|
| narrow, RPCS3 0.0.43-20607 (E:) | 2 | 0.5 s | 10.8 s (EBOOT.BIN 6.6 s, RPCS3's first start in its folder) |
| wide only | 285 | 27.5 s | 29.3 s |
| the RPCS3 log only | 1 | 0.5 s | 3.0 s |
| narrow, RPCS3 0.0.43-20572 (D:) | 2 | 0.5 s | 8.1 s |

Each made `4bf75c89...`, left no work folder, and no 32-hex run in the output. In the launcher, on the EU copy linked
in phase 3 and not decrypted, Auto detect found it and decrypted it with no step: `BCES01741 · 1.12` ready, `paths.elf`
the decrypted copy. The launcher's own download is RPCS3 0.0.43-20573, between the two tried; not downloaded for this.
The Kit's DLL from this GOWA.elf is phase 6's whole run (the same file as NPUA80918's, so the same build).

## Phase 5: everything else that knew one title (1 day)

- **The file check:** `port/game_files.inc` gets a BCES01741 list, the disc's 1.00 files and the update's 1.12 files,
  names, sizes and the SHA-256 of their first and last 64 KiB, made by `gowa.py game-files --title BCES01741` from the
  user's copy. ISO9660 names write `-` as `_`; the list uses the real names (`MP-ENGLISH.PSARC`).
- **Shader precompile:** archives from the update's `USRDIR` and the disc's; the vertex pass should find what it finds
  for NPUA80918 (the same shaders but for language files).
- **Game text and mods:** `gowa.py text` for the EU languages (`R_LANG_<language>.WAD` from `TPPS<n>`, the same
  `PATCHES` scheme); mods under `dev_hdd0/game/<id>`.
- **Optimise for this PC:** `kit_train` links the player's disc and update the way Phase 3 does, in its own hdd0.
- **Language:** the EU languages are all PS3 system languages but Greek, which the PS3 never had: find how the game
  picks `GREEK.PSARC` (a region rule or its own menu) and say so in the launcher if it cannot be chosen.
- **Reports:** the zip says which title and version, and which update folder.
- **Done when:** precompile, a text export of the German text, a training run and a report all work on the EU copy.

**Done (2026-11-17).**

- **The file check:** `gowa.py game-files` takes a disc's `PS3_GAME` with `--update-dir` its update, and wrote
  `port/game_files_bces01741.inc`: 86 files, 40.4 GB, the disc's archives and two lists from the disc, the
  `TPPS*.PSARC` and `PATCHES` from the update (`game_file_update`). The disc has a `TPPS.PSARC` of its own; the game
  opens the update's, and the list has that one. `game_find.cpp` keeps a list per title and looks for each file in its
  own folder; a disc's texts say to copy the disc again where an HDD game's say to install it again. The runtime's
  check (`main.cpp`) takes the title's list, a disc's files on `/dev_bdvd` and its updates' on `/dev_hdd0` only: the
  game looks for `TPPS1-<LANGUAGE>` and `TPPS5-<LANGUAGE>` on the disc, which no release has. NPUA80918's list, made
  again with the new command, is the same file.
- **Precompile:** phase 2's run stands: the EU copy's 83 archives, 3,453 vertex programs found (NPUA80918: 3,455).
- **Text and mods:** `gowa.py text` works on the update as it is. The update's language archives
  (`TPPS<n>-<LANGUAGE>.PSARC`) hold speech only; the text is in `TPPS1` to `TPPS5`, and the EN-GB export is byte for
  byte NPUA80918's. German menus: 5,943 lines of 2,063 entries. A build writes `TPPS6.PSARC` and `PATCHES` under
  `dev_hdd0/game/BCES01741/USRDIR`. The earlier runs' EN-GB came from this PC's language (en-GB), not the release.
- **Training:** the route reaches the level on the EU copy unchanged (`kit_train_run` on a test install, the dev exe):
  the first start's movie from 55 to 198 s, the opening level at 237 s. Without ffmpeg the movie never ends and the
  route stops there, as it would for NPUA80918.
- **Language:** Greek is picked in the game's own prompt at its first start (Cross this language, Triangle Greek),
  and the game keeps what it took; the launcher's Language row says so.
- **Reports:** `Game: BCES01741 (EU disc), the disc 01.00, its update 01.12 in <folder>`, and the files and `GOWA.elf`
  as the launcher sees them: a disc with its update, and `paths.elf` (before, a player's report said `GOWA.elf: not
  there`, as the launcher keeps its own copy). NPUA80918's report: `all 51 as 1.12 has them`, as before.
- **Checked:** `test_game_find` 46 (six for a disc's list), the other port tests, `check_repo.py`; `--find-game` on the
  EU copy (whole, no ELF where it looks) and on NPUA80918 (`ok`); the EU game for 80 s on the new build, no file
  missed.

## Phase 6: tests (0.5 to 1 day)

- `tests/test_game_find.py`: EU layouts made in a temporary folder (disc DG 01.00 + update GD 01.12; no update; old
  update; ISO only; an encrypted file), `games.yml` read, two versions on one PC.
- A test for the candidate rule on a made-up ELF (the windows it keeps and drops), and that no candidate or key is
  printed.
- `tests/test_kit_paths.py` and the others still pass; `check_repo.py`.
- **On this PC, end to end:** a fresh portable Kit in `dist/test/eu-run`; find, decrypt, build, precompile, play the
  opening chapter, save, quit, load, French and German, a report; then the same Kit on NPUA80918, and the fight bench
  once (expect the same 29 to 31.5 flips/s on six cores).

**Done (2026-11-17).**

- **Tests:** `tests/test_game_prepare.py` (new, in CI) runs `game_prepare_disc` with a stand-in RPCS3 that records
  what it reads on standard input, on a made-up big-endian EBOOT.elf with a window of each kind: narrow offers the one
  that looks like a key, once though the data has it twice; wide adds the printable and few-byte windows, never one of
  3 bytes or fewer, one from code, or one from a segment not loaded; the RPCS3 log's KLIC; narrow, then wide, then the
  log, stopping at the first that decrypts; no 32-hex run printed, the work folder gone, RPCS3's folder and the update
  unchanged, RPCS3 run on our config folder. `test_game_find`: 50, with two releases on one PC, a disc with an update
  at 1.11 in one RPCS3 and 1.12 in another (search and Browse), a search past a copy at 1.11.
- **Found by them:** the states' order put another version above GAME_NO_ELF, so the launcher's search stopped at a
  copy at 1.11 and never came to the one at 1.12, and a search could replace a better copy with it: `game_rank`
  orders copies now. `find_update` takes the update at 1.12 of several, and a disc and its update count as one copy.
- **End to end, a fresh Kit** (`package kit --test`, unpacked to `dist/test/eu-run` with `portable.txt`, so the PC's
  own `%LOCALAPPDATA%\Ascension-R` stayed out of it):

| step | result |
|---|---|
| first start | the launcher searched by itself: the EU disc and its update on E:, `Game folder set` |
| decrypt | at once after it: EBOOT.BIN 4.5 s, GOWA.SELF with 2 narrow keys 0.6 s, `4bf75c89...`; no key in the log |
| build | at once after it, 5:44: `ascension_game.dll` `f317f78f...`, the DLL NPUA80918's GOWA.elf makes (alpha.13's) |
| first play | the shader precompile 243 s; the language prompt (Cross), the movie 142 s, New Game, the opening level |
| save, quit, load | a second start's New Game saved its first checkpoint 97 s into the level; a third start's Continue Game loaded it, into Megaera's fight |
| French, German | first starts on fresh hdd0s with `system.language` fr and de: the menus and the welcome note in each |
| report | `Game: BCES01741 (EU disc), the disc 01.00, its update 01.12 in E:\...`, all 86 files, GOWA.elf right; no 32-hex run, no user name |
| the same Kit on NPUA80918 | links to the US game as Browse makes them: `Game found: NPUA80918 1.12`, no decrypt or build |
| fight bench (Kit exe and DLL, the dev settings and saves) | 30.26 flips/s, held at 30 by frame generation as the dev settings have it (the 29 to 31.5 above was that hold); 40.16 with frame generation and VSync off |

- The disc and update folders on E: (169 files) were the same before and after, and the RPCS3 there was not written
  to. The bench's menu route cannot run a profile that never saved its prefs since the welcome note (it shows again,
  and Cross does not take it away); a drive script took Circle there. The launcher's `Saves` line counted every
  release's profiles: it counts the game's own now (`game_title.h`'s save prefix: NPUA80918's are `BCUS98232-*`).

## Phase 7: documents and the release (0.5 day)

- README (what you need: a table of versions, ✅ NPUA80918, ✅ BCES01741, ❓ BCUS98232 and the rest), the Kit's
  `packaging/README.txt`, the site page, the launcher's help texts.
- Release notes (0.1.0-alpha.14): "EU disc supported", how to install its updates in RPCS3, ISO extraction steps.
- The reply to #4, after the user's OK: what is supported, the steps, a request for testers with BCUS98232 and other
  versions (their launcher's report says the `GOWA.elf` hash).

## Phase 8: later, each on its own

- **BCUS98232:** the user plans to get a copy; check its 1.12 `GOWA.elf`, add its file list, turn it on.
- **NPEA00445 (EU digital) and the other disc ids:** turned on when a player's report shows `4bf75c89...` and the
  layout works.
- **ISOs without extracting:** a decrypted ISO read in place (ISO9660 with multi-extent files over 1 GiB, and the UDF
  names), or mounted by Windows at **Play**. Encrypted Redump ISOs stay out: they need disc keys the port does not ship.
- **Saves between regions:** a US save copied as `BCES01741-*` may or may not load; try it once and document.

## Risks and open questions

- The game's own handling of a disc plus an update under our runtime (Phase 1 answers it first; RPCS3 runs it).
- APP_VER: whether the game reads it, and from where; a wrong one could hide the update's text.
- Greek: how it is chosen.
- The KLIC rule on other versions: the wide search and the RPCS3 log back it up.
- RPCS3's `--decrypt` stdin behaviour across versions: tested on 0.0.43; the launcher checks the output's hash anyway.
- A player with only an ISO: until Phase 8, a clear message and the extraction steps.

## Order and size

| phase | days | needs |
|---|---|---|
| 1 the game in a hand-made layout | 1 | the user's EU copy |
| 2 the title table | 1 | phase 1 |
| 3 finding an EU install | 1 to 1.5 | phase 2 |
| 4 decrypting without keys | 1 | phase 2 |
| 5 the rest that knew one title | 1 | phases 3 and 4 |
| 6 tests | 0.5 to 1 | phases 3 to 5 |
| 7 documents and release | 0.5 | phase 6 |
| 8 later | each on its own | copies or players |
