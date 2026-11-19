# Building the pixel shaders before play: what it would take (2026-11-18)

Issue #15's reporter, on alpha.15: places met again (chapter select) play smoothly, new ones still stop while their
shaders build, and the prebuild should cover more. The vertex programs are built before the first start since
2026-11-08 (`2026-11-08-shader-precompile-review.md`); the fragment programs are not, because their text depends on
the state of the draws, which the files were thought not to say. This measures that state, looks for it in the files,
and prices building every fragment program before play.

## How it was measured

- A probe in `vp_get_fp_pso` (`FP_STUDY`, not kept) wrote every fragment text the backend made, with the state that
  made it and the 0x80 bytes before the program in guest memory, and timed every compile (`d3d_compile_cached`); the
  texts were written out too. One run of the parasite-fight replay (`build\probes\fight.pad`, 140 s: boot, menu, the
  level's street, courtyard and fight) from an empty shader cache, the precompile gate off.
- Those texts compiled again outside the game with dxc.exe as the port does (ps_6_0, -O1, -HV 2018, -Gis,
  -Qstrip_reflect), 11 at a time and one at a time, on the i5-12400F (twelve threads).

## What a text depends on

- **One state a program.** 285 distinct texts from 282 programs: 279 programs had one text, three had two (the sRGB
  output once, the alpha test twice).
- **Rarely the plain one.** 4% of the texts had no cube or volume unit, shadow, sRGB output, AA grid or alpha test.
  The sRGB output (SET_SHADER_PACKER) was in 92%, a cube unit in 42%, a volume unit in 8%, the alpha test in 13% (one
  function, 0x204), shadows and AA grids in none. The best single guess, the sRGB output and nothing else, makes 36%;
  the cubes are spread over units 1 to 11 (twelve masks). On 2026-11-08 the program alone made 27%: the sRGB output
  has been compiled in since.

## What the files say

- **The engine's header** is there before a program kept in main memory (47 of the 274 whose bytes could be read):
  its size, the texture coordinate inputs, the register count and control bits (+0x08, e.g. `03000400`), the program's
  RSX address as patched at load (+0x0C, the draw's `fp_addr` in all 47), 0x80 (+0x10), then the count and the places
  of the inline constants the engine patches. The other programs had been copied elsewhere, without it.
- **No texture dimensions and no alpha test in it.** The state is in the materials of the WAD files, a format this
  port does not read yet.

## What building them all costs

- **Compile:** a text takes 43 ms on its own (59 ms with dxc.exe's start) and 97 ms with 11 at a time: 92 a second on
  twelve threads. In play, four workers beside the game: 79 ms on the average, 384 ms the longest.
- **All of them:** the archives hold 33,525 distinct fragment programs (2026-11-08): about six minutes here at one text
  each, 10-15 on a four-core laptop, plus the archives' read as for the vertex programs (1.5-2.5 minutes, seconds
  once the list is kept). The multiplayer archives' programs could be left out.
- **Store:** DXIL is 12.5 KB a program, 420 MB for all. These 285 packed 3.6:1 with xz, near the store's zstd 19 with
  long matches, 2.0:1 with zlib: some 120 MB on disk. The store keeps every program in memory, which would not do for
  these: it would have to read them when they are needed.
- **What stays at first use:** the pipeline, about 7 ms from built programs (the reporter's 3,852 in 4.5 s on six
  threads) against the 80-140 ms of a compile now; and the programs that are not in the files (post-processing, 14% of
  a session's programs on 2026-11-08), built in the first frames as now.

## The ways

1. **Guess the state** (plain, or the sRGB output): 4-36% of the texts, most of the six minutes for nothing. No.
2. **Read it from the files:** not in the program's header; the WAD's materials would have to be worked out first.
   The most exact if it works, an unknown amount of work before knowing.
3. **Learn it from play:** a program meets one state, so a table of program hash to state, filled from sessions,
   would make the exact texts with the shaders as they are. It covers only what has been played and recorded; it holds
   hashes and bits, no game code.
4. **Make the text not depend on it:** the state read when the program runs, from b1. The sRGB output, a branch at the
   end. Cube and volume units declared as TextureCube and Texture3D as well, in register spaces 1 and 2 over the same
   descriptors (one table with overlapping ranges, if the runtime takes them, else three tables at one address), and
   sampled through a branch on the unit's dimension: no more descriptors a draw. The shadow comparison, already worked
   out in the shader, with its function and filtering from b1; the AA grid from b1. The alpha test stays a variant: a
   discard in every program would cost the early depth test. Then a program has one text, and the form without the
   alpha test of every program in the files, built before play, covers 87% of this session's texts. The rest (the
   alpha-tested 13%, the programs not in the files) build at first use as now.

## Recommended

Way 4, in steps tried one at a time:

1. The text that does not depend on the state, behind a switch: the replay's screenshots against today's text, the
   GPU's time (the overlay, `RSX_CENSUS`), the compile time of the new texts. The risk is in this step: a cube or a
   shadow drawn wrong, a slower GPU.
2. The fragment programs found in the archives (their header has been known since 2026-11-08) and built by the
   precompile: in the background at low priority with the game playable, not at the gate, as six minutes and more at a
   first start is too long to wait. What a draw needs first is built first, as now.
3. The store read when a program is needed, not kept whole in memory.

Way 3 can add the alpha-tested programs later. Way 2 is worth a look first only if step 1's cost turns out too high.
