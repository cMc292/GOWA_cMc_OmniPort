# Issue #18: broken geometry the first time a scene plays (2026-11-16)

SoreteNuclear, 0.1.0-alpha.11, i3-10105F (four cores, eight threads), RX 5700 XT, 16 GB. "The same glitch happens in every
scene the first time you play it. But if you die and restart it's fixed." The screenshot (the opening chapter, the bridge
past the gate): rock and buildings torn into spikes and floating shards, a translucent shape over the sky, black gaps.
The report: two sessions of the opening chapter (HECA100 to HECA500, ~21 minutes), the settings, the Kit's build log.

## What the logs say

- Nothing about geometry: no `[edge-*]` warning, no `[vram]` cut, no dropped record, no index buffer full.
- Shaders were built during play: `guest shaders built by 2 worker threads`, `slow shader compile: guest_fp ... took
  107 ms` (up to 225 ms). The disk cache had 7,542 programs at the second session's start, and the vertex pass came
  from it (`read from the shader cache in 0.0 s`).
- Both sessions ran the game's code built without the profile; the Kit built the profiled DLL after them (`build.log`).
  The profile is not the cause.

## Reproduced

The bench route (Megaera's fight, the same chapter) on four cores (`GOWA_CPU_MASK=0xFF`), the reporter's graphics settings,
and a new, empty shader cache, one run each:

| background shaders | 22-38 s after the load | 45 s and later | frames |
|---|---|---|---|
| on (`async_shaders=2`, the default) | smeared, stacked copies of walls and floor, shards in the sky, gaps showing old frames, "Compiling shaders" | clean | 30-42 flips/s |
| off (`async_shaders=0`) | clean | clean | 1 and 16 flips/s at 26 and 34 s: the game waits for each build |

In both runs the vertex pass was being compiled for the first ~40 s and the CPU lit its draws meanwhile; the run with the
shaders built first was clean all the same, so that fallback is right.

## Why

`D3D12_ASYNC_SHADERS` (the launcher's "Build shaders in the background", on by default) builds a new guest program and
its pipeline on worker threads, and **skips the draws that need it** until it is ready (`rsx_d3d12_backend.c`, the draw
loop: `s_sj_stat_skipped`), rather than stop the picture or draw with another program. The game never clears its colour
target between frames: each frame draws over every pixel. A skipped draw leaves its pixels as an earlier frame had them,
and as the camera moves those show as copies smeared across the screen: [the prison's trails](2026-11-08-prison-trails.md)
again, from another cause. Every program is kept in the disk cache once built, so a place does it only the first time,
and a restart from a checkpoint comes back with the programs ready. Two workers on a four-core CPU take longer to catch
up, so the gaps last longer.

## Fixed (options 2 and 3 below, decided with the user)

- **A solid draw waits for its builds** (`D3D12_ASYNC_SOLID`, on; `=0` skips it as before): no blending, or blending
  that keeps only what it draws (ONE, ZERO, ADD). Its job moves to the front of the queue and the workers run at normal
  priority while it waits (below normal they lost the cores to the game's threads), up to `D3D12_ASYNC_SOLID_MS` (2000).
  A blended draw is still skipped. The log counts the waits at each power of two:
  `[d3d12] background shaders: N solid draws waited for their builds, S s in all`.
- **Workers by the CPU** when background shaders are on (`async_shaders=2`): 2 below eight threads, 3 at eight to eleven,
  4 at twelve and more, counted from the threads the process may run on.
- **The same reproduction again** (four cores, the reporter's settings, an empty cache): every shot clean, 3 workers;
  a stop at 26 s (one frame of 537 ms) while the programs built; 256 solid draws waited, 18.8 s in all over the run, an
  empty cache's worst case. The reporter's cache already held 7,542 programs, so far fewer of them come up there.

| background shaders | the shots at 22-45 s | flips/s, 20-100 s |
|---|---|---|
| on, before | smeared copies and shards | 34.87 |
| off | clean | 30.39 |
| on, after | clean | 32.61 |

## Looked at and not done: a program the GPU interprets, and unoptimized first builds

Asked for after the fix, to take the stops away as well.

- **An interpreter on the GPU** (RPCS3's way): one pixel shader that reads a fragment program's ucode from a buffer and
  runs it. Left undone, before any code:
  - it has to do all of `rsx_fp_decompiler.c` (2,057 lines) on the GPU, bit for bit: ~70 opcodes, the fp16 and fx12
    precisions with H registers aliasing R ones, condition codes, the texture forms (projective, bias, lod, gradients,
    shadow compare), WPOS and the comparison helpers;
  - a unit's texture type is fixed by each program (`Texture2D rsx_tex[16]`, cube and volume units declared one by one):
    one shader for every program needs a root signature with each unit's 2D, cube and 3D views, and the draw code to bind
    them;
  - its pipelines are still built per state, from a shader that size: on the reporter's AMD driver, which cannot be tried
    here, that may stop the picture as long as the builds it replaces, and a long program interpreted per pixel at render
    scale 1.5 may run at a few frames a second meanwhile;
  - weeks of work against stops of ~74 ms a new program, each place's first time only.
- **Unoptimized builds first** (`D3D12_SHADER_OPT=skip`), with the optimized ones later: the same reproduction gave
  builds no faster (the 25 KB program 215 ms against 225), 48.2 s of solid draws waiting against 18.8, and a median of
  4.18 flips/s from 20 to 100 s against 32.6. Not taken.

## What can be done

- **Now, for the reporter:** "Build shaders in the background" off. No broken geometry; a short stop the first time
  something new is drawn.
- **Wait for solid geometry, skip effects:** a draw that writes depth and is not blended waits for its program (a longer
  budget than the 3 ms of `D3D12_ASYNC_WAIT_MS`); blended draws (fire, smoke, sparks) are still skipped, which leaves no
  gap. Stops instead of trails, only where a hole would show.
- **More workers on CPUs with eight threads or more,** so the window is shorter.
- **A program the GPU interprets** while the real one builds (as RPCS3's shader interpreter): no gaps and no stops, and
  the largest piece of work.
