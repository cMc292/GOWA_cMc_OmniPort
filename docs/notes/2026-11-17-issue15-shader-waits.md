# Issue #15: shader waits, a portable folder's first start, and a hang at Megaera (2026-11-17)

Two reports from the issue's reporter on 0.1.0-alpha.13 (Ryzen 5 5600X, six cores, RTX 5060, 1080p at 144 Hz, render
scale 2, SMAA, MLAA and depth of field on, shadows and motion blur off): the game ran slower and stuttered more than
before, after "several minutes of two build processes", and a later session hung right after Kratos pulls the first
Blade back through Megaera.

## What the reports show

- **The stutter is issue #18's fix.** Since alpha.13 a solid draw whose shader is still being built waits for it, up
  to 2 s, instead of leaving a gap (docs/notes/2026-11-16-issue18-first-play-trails.md). The first session's log: 128
  such waits, 10.8 s in all; its pad recording's pauses, up to 1.03 s. Between them 55-65 frames a second.
- **The caches were empty twice.** The first session was the first of alpha.13 (the vertex shaders' precompile, 65 s,
  and no pipeline kept). Then the reporter put portable.txt beside the exe, which keeps the files in that folder: its
  Kit build wrote to `<settings>` at 11:14 and to `<exe>` at 11:35. The new folder started with nothing, so the Kit's
  build, the precompile (64 s), every shader and the saves came again.
- **The hang:** loading the next part of the level (HECA100) after the Blade, the game's VRAM pool allocator
  (`func_0039E3F8`) gave no block. `func_0039B93C` stored through it (`block + 0x24`, the log's null write), the
  texture kept a null block, the main thread then spun on a core, and the RSX met its jump-park with nothing after it
  (the FIFO resync in the log came after, not before). From the level's load (R_MEGAERA03) on, `gowa_vram_cut` had
  stopped one texture's trim more than 128 times: the pool was short all along.
- **Not D, U, S or J.** S and J were never written (docs/notes/2026-11-16-s-j-review.md); U and D change EDGE geometry
  jobs, not shaders or the VRAM pool. Each has a switch (`GOWA_EDGE_UNPACK`, `GOWA_EDGE_VEL_BINORM`) should the hang
  ever come here.

## The hang, not seen here

| Run | Trims by `gowa_vram_cut` |
|---|---|
| Megaera's fight from the dev save, MLAA on, 240 s | 0 |
| A new game with the reporter's patches, its first checkpoint and 180 s of fighting after it | 0 |
| The reporter's own pad recording played on a new profile with their patches (PAD_REPLAY) | 0 |
| The same at their render scale 2 (2560x1440), SMAA, 16x filtering, alpha.13's 2 s waits, no pipeline kept | 0 |
| A new game with all of that, through R_MEGAERA03's load to 240 s of fighting after its checkpoint | 0 |

The first replay followed the route to the start of Megaera's fight and went its own way before the Blade: HECA100's
main part was never loaded. At 2560x1440 this PC's GPU made ~35 frames a second against their ~60, the replay's marks
came late, and it went astray in the menus. The last run had every setting of theirs, and the shader waits as many
as their session's (64, 3.2 s here, 1.7 s there): no trim at R_MEGAERA03, where theirs began. What is left differs
from this PC, not from the settings: their CPU and GPU, and the way they played. A save made just before the Blade,
from the reporter, would let it be played here.

## What changed

- **graphics.shader_wait** (`D3D12_SHADER_WAIT`; the backend's `sj_wait_cap`): how long a solid draw waits for its
  shader. short, the default: 0.25 s, then the draw is skipped and its gap shows until the build is done; full: 2 s,
  as alpha.13; off: no wait, as alpha.12. `D3D12_ASYNC_SOLID=0` and `D3D12_ASYNC_SOLID_MS` still set it, for tests. The
  log's line on the waits now says how many met the cap.
- **The launcher's Graphics tab:** "Wait for new shaders", Off, Short, Full; recomplauncher.json has it too
  (advanced).
- **A portable folder's first start** (`port/portable_copy.cpp`): with portable.txt beside the exe and none of the
  program's files there yet, if `%LOCALAPPDATA%\Ascension-R` has a settings file, the player is asked whether to copy
  its files here. Copied: the settings (their paths moved to the new folder), the saves and backups, GOWA.elf, the
  built game code, the shader caches, the screenshots, what the game wrote under its hdd0; not the logs or the game
  code build's work folder. The links to the game's folders are made again to the same folders, never followed. The
  old folder is only read. No starts the folder empty, and the next start does not ask again.

## How it was tried

- **The waits,** on an empty shader cache, Megaera's fight from the dev save (Continue, 121 s of it): 128 waits, 5.25 s
  in all, one of them past the 0.25 s cap.
- **The launcher's row,** in the Graphics tab after "Build shaders in the background", Short chosen.
- **The copy:** `tests/test_portable_copy.py` (4, in CI): the files, the work folder and logs left out, the settings'
  paths in either slash, the links made again and the game's files not copied through them, the old folder as it
  was, a link whose folder is gone. And the program itself, on a new portable folder with a made-up
  `%LOCALAPPDATA%\Ascension-R`: the question (9 MB), Yes copied 5 files and 2 links and the launcher had the game and
  the save at once; No copied nothing and said so in the log.

## A frame's share of waiting (later the same day)

Short still stopped the game in a new place: the quarter second was each draw's, so a frame that met several new
shaders waited them one after the other, and #15's later session waited 1,024 times, 79 s in all over 38 minutes. A
player asked for no pauses at all while shaders build. Short is now a frame's share instead: the solid draws of one
frame wait 33 ms in all, and past that a draw whose shader is not built is skipped that frame, as with Off, while its
build goes on (`sj_frame_budget`, `SJ_FRAME_SHORT`). Full keeps up to two seconds a draw; Off does not wait.

From an empty shader cache each time, #9's checkpoint in the temple (Continue into TEM400-500, then 150 s of a
fight), render scale 1.5 with FSR 3.1, one run of each (`RSX_CENSUS=15` for the presents, `D3D12_SJ_WAITLOG=1` for
each wait):

| | Short, each draw's 0.25 s (before) | Short, 33 ms a frame | Off |
|---|---|---|---|
| Waits | 68, 3.4 s in all, the longest 250 ms | 73, 1.9 s in all, the longest 34 ms | none |
| The longest present | 2,068 ms | 318 ms | 354 ms |
| Presents over 250 ms | 6 | 1 | 1 |
| Presents over 100 ms | 12 | 8 | 3 |
| Presents of 60-100 ms | 10 | 59 | 28 |

The second-long stop is gone, and what is left over a quarter second is as with Off (loading). In its place come
more frames of 60-100 ms where a frame waits its 33 ms. A quarter to a third of the waits still ended with the shader
built (19 and 16 of the first 64 in two runs), so those draws show at once instead of leaving a gap. The log's summary
line says how many waits ran out and how many draws a frame did not wait for.

Off is the default now, at the player's word: no wait at all, and a gap for a moment where a new shader is still being
built, as 0.1.0-alpha.12 had it (the trails of #18, the first time a place is drawn). Short and Full stay for a picture
without those gaps. A settings file that already says `shader_wait = short` keeps it.
