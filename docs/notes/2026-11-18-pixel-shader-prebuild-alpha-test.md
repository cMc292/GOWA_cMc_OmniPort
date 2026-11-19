# Pixel shaders built before play: the alpha test read at run time (2026-11-18)

After step 3 (`2026-11-18-pixel-shader-prebuild-step3.md`) the build ahead gave 84% of the fight replay's pixel
shaders; 38 of the 39 texts it lacked were the alpha test's variants. The alpha test is now read as the program runs,
in every dynamic text, so those draws take the same text, and the same program built ahead, as the rest.

## What it was

- A draw with the alpha test on (SET_ALPHA_TEST_ENABLE, its function and reference) got a text of its own: the colour
  output kept, its alpha compared and the pixel discarded when it fails (`rsx_fp_apply_alpha_test_buffered`), the
  reference read from b1 (`fp_alpha.x`) but the comparison written into the text, a text and a key a function.
- In the fight replay: one function only, GREATER (0x204), on 38 of 304 texts, 28 programs (4 of them drawn without it
  too). Short programs: 40 statements and 2 fetches at the median, against 249 and 7 for the rest.
- The game's files do not say which programs are drawn with it: no SET_ALPHA_TEST_* methods in the WADs (GAME and
  SHELL looked at); the engine sets it from its materials as it draws.

## Three ways looked at

- **A variant for every program** (GREATER): its text compiles as fast as the plain one (33 ms against 36 a text, the
  35 of the replay one at a time) and its DXIL is 1% larger: the build twice as long and its file twice as large
  (some 14 minutes, 610 MB), and another function still not built.
- **A variant for the short programs**: cheaper, but a guess: a longer alpha-tested program would miss.
- **The test read at run time in every text**: the build unchanged, every function, but every program has a
  discard, so the driver may not test depth before it runs. Measured first with a discard that never runs added to
  every text (`if (fp_alpha.x < -1.0e30) discard;`, kept by DXC: `dx.op.discard` in its listing), the fight replay
  from warm caches each time, the census's 20 s windows of the fight:
  - Smooth preset: GPU 9.91 ms a frame against 9.92 (draws 4.57 against 5.39), 56.2 flips a second against 55.1.
  - Shadows on: GPU 10.99 against 11.57 (draws 7.06 against 7.25), 51.5 against 47.6.
  No cost on the RX 6700 XT; the replay's own spread is larger. This one was taken.

## The text and the draw

- `rsx_fp_apply_alpha_test_dyn` (decompiler) ends every dynamic text with the test: `asuint(fp_state.x)` bit 11 on,
  bits 8-10 the function (CELL_GCM's less 0x200, NEVER 0 to GEQUAL 6), `fp_alpha.x` the reference already there, the
  colour output's alpha (the first target's with several) compared under a branch, and discarded when it fails.
- `fp_dyn_state` sets the bits from the draw's alpha state at replay (ALWAYS, or NO_ALPHATEST=1, as none).
  `vp_get_fp_pso` leaves the alpha test out of the text and the key: one text and one pipeline a program and state.
- `RSX_FP_ATEST_VARIANT=1`: the variants as before, for a GPU where the discard would cost.
- The texts' form is 2 (`FP_DYN_FORM`): the file built ahead carries it (`FPB_TEXT`), so a file of form 1 is begun
  again at the next Build (and the launcher says not built yet); a pipeline's description carries it too
  (`PSOC_VERSION_DYN` 3), so the store's pipelines of form 1 are left out at the start and go at the next pack. With the
  setting off, the start builds the old texts' pipelines only (1), not a dynamic form's.

## How it was tried

- **The build**: 39,598 texts, none failed, 407 s; the file 309 MB (305 before).
- **From an empty store with the new file**: 282 pixel shaders taken from it, 1 compiled (the ELF's program without a
  header): 99.6%, from 84%. The run made 283 texts, every one with the test read at run time (215 of them on the first
  of several targets), none where the rewrite did not take; no compile failed.
- **Pictures** at 85 and 100 s: the grass on the courtyard's floor drawn as blades, the cages' bars with the room
  behind them, blood, sparks, the parasites and the HUD as before.
- **GPU, the test as it is against the variants** (`RSX_FP_ATEST_VARIANT=1`), warm, the fight's four windows: 10.23,
  10.08, 10.33, 10.04 ms a frame against 10.48, 10.01, 10.45, 10.76; 53.5 flips a second against 52.6. The draws used
  as many pipelines (300 against 305); the session from an empty store had made 473, the replay's own drift.

## Not done

- NVIDIA and Intel: no discard cost measured there; `RSX_FP_ATEST_VARIANT=1` is the way back if one shows.
- The ELF's program without a header (one compile at the boot).
