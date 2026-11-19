# Pixel shaders built before play: step 3, the store and on by default (2026-11-18)

Step 3 of the plan in `2026-11-18-pixel-shader-prebuild-study.md`, after the texts (`2026-11-18-fp-text-without-state.md`)
and the build (`2026-11-18-pixel-shader-prebuild-step2.md`): the shader store rid of what the old pixel shaders left
in it, `graphics.shader_prebuild` on by default with the build offered on the Play tab, and reports that say what was
built.

## What the old pixel shaders left behind

With the new texts no draw asks again for a pixel shader compiled before, nor for its pipelines, but the store kept
them, and Build pipelines first built every one at each start. A copy of this PC's store (8,269 programs: 7,379 vertex,
903 pixel; 1,787 pipelines, all old) started with the setting on:

- 57 s to build the 1,787 pipelines on six threads with the driver's cache cold; the title waited its 30 s and went
  on without them; 918 MB more memory and 260 MB more video memory while they were held.
- With the driver's cache warm (the setting off, the same store): 178 ms, and still 702 MB and 256 MB more. On #15's
  PC, 3,852 pipelines took 4.5 s.
- 886 pixel programs only those pipelines used: 13 MB read into memory at each start.

## A pipeline's form

- A PIPE record's description now begins with 2 when the session that made it drew with the new texts
  (`PSOC_VERSION_DYN`, `fp_dyn_on`), 1 as before: every pipeline of a player's store is 1. Each form's draws ask for
  its own pipelines only, so a pipeline's key changes with it.
- The pixel program alone does not tell: a first try read it from the DXIL (resources in register spaces 1-2, the
  cube and volume views, in the PSV0 part), and took the new texts that fetch no texture for old ones: their pipelines
  went at every start, and a pack with them.

## At the start, and the pack

- `psoc_start`: only the pipelines of the session's form go into the table and the start's build; the log says how
  many stayed out. With the setting on and old ones there, the store is packed again at once, on a thread of its own
  at the lowest priority (`the old pixel shaders`), unless the store's opening has started a pack already.
- `shc_fold_start`, with the setting on: the old pipelines stay out of the pack, with the pixel programs only they had
  and the sources that name those. The vertex programs and everything else stay. With the setting off nothing goes:
  the new form's pipelines are kept, not built.
- `fp_dyn_on` no longer keeps "off" when it is asked before the device is made (it waits for the compiler's choice).

Measured, a copy of the store each time, 40-50 s of the fight replay:

- **On, the first start:** 0 pipelines to build, 1,787 left out, no wait; the pack: 7,383 programs, 0 pipelines,
  3.4 MB (6.4 before), 1,787 pipelines and 886 programs left out, 7.4 s on its idle thread.
- **On, the next start:** the 29 pipelines the first session made, all of the new form: built in 5 ms; nothing left
  out, no pack.
- **Off:** all 1,787 built (178 ms, the driver's cache warm by then), none left out or dropped.
- **A program left out and used again** (found the same day, fixed): a store whose new-text pipelines were made
  before their tag (so counted old) lost them at the pack, and with them pixel programs the session went on using
  from memory, never written again: the next start dropped 306 records and compiled them all. A program a pack leaves
  out is now marked (`SHP_GONE`), and goes back onto the journal with its source at its next use (`shp_put`,
  `shc_keep`, `d3d_compile_cached`): the same case then opened with 0 records dropped. A player's store cannot meet
  it (its old programs are the old texts, which no draw asks for again), a developer's could.

## On by default, and the offer

- `graphics.shader_prebuild` is on unless the settings say off (backend and launcher): every player's pixel shaders
  are the new texts. Those compiled before are not used again; without a build, places are drawn as with an empty
  cache at first.
- The Play tab's ready check has a line for them: built ahead (green, the `.done` names this version and the file is
  of this build's form, `rsx_d3d12_prebuilt_current`, which reads its header without a device), not built yet or
  partly built (Build), building (its percentage, Stop), or compiled as you play (the setting off). Build is the
  sidebar's Build all shaders now; the line is read again when a build ends.
- Problem reports say it in a line: the store's files and sizes, the mark the last finished build left (version and
  counts), and whether the file is of this build's form.

## Checked

- The ready check's Build, clicked: the build started with the window's settings; the second click (Stop) ended it.
  Its states drawn by `GOWA_LAUNCHER_SHOT` (not built, building).
- A report made with `--report`: `Shaders: dxil.journal 6.6 MB, dxil.prebuilt 291.3 MB, fprogs 101.6 MB, built ahead
  by Ascension-R 0.1.0-alpha.15; 49996 found, 0 compiled, 49996 had, 0 failed (the file is this build's)`.
- README and `docs/FEATURES.md` say how the pixel shaders are built.

## Not done

- The alpha test's variants (13% of the fight's pixel shaders): done the same day, the test read at run time
  (`2026-11-18-pixel-shader-prebuild-alpha-test.md`).
- NVIDIA and Intel: the three views over one range are the usual bindless pattern, and the debug layer and
  vkd3d-proton took them; a player on NVIDIA (#15's reporter has an RTX 5060) can confirm after a release.
