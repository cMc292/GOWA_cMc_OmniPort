# Pixel shaders built before play: step 2, the build and its file (2026-11-18)

Step 2 of way 4 in `2026-11-18-pixel-shader-prebuild-study.md`, on step 1's texts (`2026-11-18-fp-text-without-state.md`):
the fragment programs of the game's files compiled ahead of the draws that need them, into a file beside the shader
cache, by a process of their own that the launcher starts before play. Behind `graphics.shader_prebuild` (the
launcher's Graphics tab, "Pixel shaders built ahead"), off by default here and on from step 3
(`2026-11-18-pixel-shader-prebuild-step3.md`), which turns step 1's texts on too (`RSX_FP_DYNAMIC` is its other name). A build in the background while the game plays was made and measured too, and
taken out (below).

## The pieces

- **The program list** (`port/precompile.cpp`). The precompile's read of the archives, before the first start of each
  version, now finds the fragment programs too, by the engine's header: size, texture coordinate inputs, registers and
  control bits (0x84 in the second byte, in the files), 0, the program's offset, then the table of the inline constants
  the engine patches; the program ends at its size. The offset is 0x80, or 0x100 and 0x180 where that table is longer
  (below: the first scan took 0x80 alone). 49,996 programs on the EU disc, each once with its control bits, written to
  `<cache>.fprogs` (107 MB, form 2) so that no later build reads the archives again.
- **The build** (`rsx_d3d12_prebuild_fragment_programs`, fork). Each program's text as a draw makes it with no alpha
  test, sign expansion or depth comparison (`fp_dyn_text`, the control bits as the decompiler reads them, 0x4E):
  39,598 texts, the other 10,398 programs the same code as one of them with other constants. They are made 256 at a
  time on one thread, compiled by DXC on the others and appended to `<cache>.dxil.prebuilt` as each is done. One build
  a file (a named mutex of its path). A file of this build's form is gone on with, what it has passed over (a whole
  file in 9 s); one of another form is begun again. `<cache>.dxil.prebuilt.done` names the version it was finished
  for.
- **The file.** A header (its form, the texts' form, the compile flags, the compiler, the dictionary's length), a
  dictionary, then a record a program: its tag (written last), the source key and length, the program's length, its
  zstd frame with zstd's checksum. The dictionary is the first batch's programs, 1 MB of DXIL, which zstd takes as raw
  content for every frame. The 39,598 programs are 756 MB of DXIL (18.6 KB each), 305 MB in the file (2.49:1, level
  12). Packed one by one they made 1.57:1; with the dictionary at level 19, 2.82:1, but that takes 35 MB a compressing
  thread and 6 ms a program. The dictionary is written before the header, the record before its tag, and a frame that
  does not unpack whole to its checksum is a miss: a game that reads the file while it is written takes whole
  programs or none.
- **The game** (`fpb_find`, fork). When the shader store has not a source, it looks in the file, indexed at the
  store's opening and read on from where it stopped a second after a miss; what it takes goes into the store as a
  compiled program would. Unpacking a program takes about 50 us against some 70 ms to compile it.
- **From the launcher.** "Build all shaders now", down the sidebar under Clear shaders: the exe again with
  `--prebuild-pixel-shaders` and the window's settings, at the normal priority on every thread but one, its percentage
  on the link, stopped by a second click, by Play or by Quit (it never runs beside the game); the next click goes on
  where it stopped. It turns the setting on and saves it, since the game reads the file only with it on. Clear shaders
  moves the file and its `.done` to the Recycle Bin with the rest; not the program list.
- **Census** (`RSX_CENSUS`): how many pixel shaders the file gave and how many were compiled, and of the texts it did
  not have, how many had an alpha test, a sign expansion, a depth comparison, or none of these.

## How it was tried

This PC (i5-12400F, six cores and twelve threads; RX 6700 XT), the parasite-fight replay (`build\probes\fight.pad`),
one run each.

- **The build** as the launcher starts it (`--prebuild-pixel-shaders`, eleven threads at the normal priority): the 83
  archives read and 39,598 programs compiled in 420 s, none failed. With the list already there, the first scan's
  33,402 took 330 s: some 100 programs a second.
- **From an empty store, the file there** (the first scan's file): 212 and 210 pixel shaders taken from it in two
  runs, 98 and 100 compiled during the run: 68%. Of the texts it did not have (census), 37 had an alpha test and 37
  none of the variants. Those 37 were looked for in the game's files by their instructions: 36 sit behind headers
  whose program starts at 0x100 (35) or 0x180 (1), after a longer table of patched constants, in SHELL.WAD, R_FX.WAD,
  HECA*.WAD, R_HERO00.AS and others, which the scan passed over; one, 160 bytes of the boot, is in the ELF with no
  header. With the scan that takes them (49,996 programs, 39,598 texts): **252 taken, 48 compiled, 84%**; the texts it
  did not have, 38 with an alpha test and the ELF's one.
- **Pictures**: the street and the courtyard at 100 s in each run, the materials, the metal, the HUD as before.
- **vkd3d-proton 3.1** (the test bed of `2026-11-13-proton-own-shaders.md` again, on this PC: vkd3d-proton 3.1 and
  DXVK 3.1.1 beside a copy of the exe), from an empty store with the file: vkd3d-proton logged no error (its warnings
  are DXBC parsing and extensions); it runs the descriptors bindless (`VK_EXT_mutable_descriptor_type`,
  `VK_EXT_descriptor_buffer`), so the three views over one range are no matter to it. 200 pixel shaders taken from
  the file; Megaera's cutscene drawn right (skin, metal, light). Cold, 34 flips a second against 56.6 on D3D12, with
  the GPU at 10 to 14 ms a frame: the CPU's, every pipeline translated and compiled for the first time. Warm, 41.6
  (median 38.9), no error either: there the GPU's copies take 11 to 12 ms a frame against 3 to 6 on D3D12, the draws,
  which are all the texts change, under 1 ms. vkd3d-proton on this AMD's Vulkan driver for Windows, then, not the
  texts; on the NVIDIA PC of 2026-11-13 the pace was D3D12's, and Linux's drivers are others again.
- **In the background, during play** (the game started the build itself at idle priority, three threads, in a job
  that ended it with the game): the same replay from a full store, with the build running and without it: 53.2
  flips a second against 57.0 (median 54.2 against 59.1), some 7% fewer, where two runs of one setting differ by 2%;
  and in 150 s the build made 577 of the 49,996 programs, 4 a second against 100 in the foreground, so all of them would
  take some three hours of play. The threads at the idle priority still share the cores the game runs on (their
  second threads) on this six-core CPU. So it was taken out (2026-11-18): the launcher's build only.
- **The launcher** (its own settings, the click sent to the link): the setting goes on and is saved at once, the build
  starts with the window's settings, its percentage on the link, and a second click ends it, the programs it made kept
  and no `.done`. Play while it builds: the window's end ends it (`pb_stop`). A build begun again passes over what is
  there: the background one of the time passed over the 683 programs made before at its first line, and a finished
  file takes 9 s.

## Not done

- **The alpha test**: 38 of the 39 texts the replay met that the file lacks, 13% of its pixel shaders. Its function
  comes with the draw (only 0x204, GEQUAL, met so far, `2026-11-18-pixel-shader-prebuild-study.md`); a text a
  function, or the test read from `fp_state` as the rest is, would take them in.
- **On by default**: after that, other GPUs (NVIDIA, Intel) and the store's hygiene (step 3).
- The ELF's program without a header, met at the boot: one compile, once.
