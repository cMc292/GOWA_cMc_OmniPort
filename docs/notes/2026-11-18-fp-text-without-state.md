# The fragment text made from the program alone: step 1 of building pixel shaders before play (2026-11-18)

Step 1 of way 4 in `2026-11-18-pixel-shader-prebuild-study.md`: a buffered fragment program's text no longer depends
on what its draw binds or sets, so it can be built before a draw needs it. Behind a switch, `RSX_FP_DYNAMIC=1`, off
by default: no player's text changes yet.

## The text

- **Three views of each unit.** `rsx_fp_decompile_buffered_state_ex` declares `Texture2D rsx_tex[16]` (space 0),
  `TextureCube rsx_texc[16]` (space 1) and `Texture3D rsx_tex3[16]` (space 2), all over the draw's one window of 16
  descriptors: the root signature's SRV table takes two more ranges at offset 0. No more descriptors a draw.
- **A helper for each unit and opcode** the body uses (`_rsx_texN`, `_rsx_txpN`, `_rsx_txbN`, `_rsx_txlN`). It reads
  the unit's state from `asuint(fp_tcscale[u].w)` (bits 0-1 the view: 2D, cube, volume; the one its descriptor was
  made with, `dr_unit_view`) and takes the path the compiled-in masks gave: a 2D fetch scaled by `fp_tcscale[u].xy`
  (TEX, TXP), a cube with three coordinates and no projective divide, a volume with three (TXP divided); TXB and TXL
  neither compare nor scale, as before. Gamma as the `_dyn` entries: `_rsx_gamma_dyn` around each fetch.
- **`fp_state`** in b1 (c17; the inline constants move to c18): x bit 0 the sRGB output (the rewrite's conversion
  under an `if`), y `1/s_wide`, zw the AA sample grid (the WPOS grid line in every program that reads WPOS: on a 1x1
  grid `floor(p) + 0.5` is the position SV_POSITION gives). The WPOS formula is the ultrawide one with `fp_state.y`
  for its literal, `fp_alpha.w` 1 while the render scale is off, so neither setting changes the text.
- **Left as variants, in the text and the key:** the alpha test (13% of texts), sign expansion (none in this title),
  and the units a draw compares depth on. The comparison's path in every fetch made a text 105 ms to compile and
  22 KB of DXIL against 71 ms and 14.9 KB without it; one program compares, the shadow mask, on unit 5. Its function,
  filter and coordinates still come from b1 (bit 2, bits 3-5, bit 6, bit 7).
- **Replay** writes the state (`fp_dyn_state`): views, comparisons (`dr_shadow_descs`), the sRGB bit of `fp_ctrl`,
  the grid of the target. **The key** drops the cube and volume units, the grid and the sRGB bit; `shadow` holds the
  compared units.

## How it was tried

The parasite-fight replay (`build\probes\fight.pad`), 125-150 s a run, one run each:

- **Off, the old texts are unchanged.** From an empty cache, 288 texts: 279 byte for byte the study's, the other 9
  programs that run met and the study's did not (none of them near one of its texts).
- **Fewer texts and pipelines.** The same replay made 263 dynamic texts against 288, and 418 pipeline lookups that
  missed against 472: draws of one program with other states share them now.
- **Compile** (dxc.exe with the port's arguments, this PC's twelve threads): old 57 ms a text alone, 93 a second 11 at
  a time, 12.7 KB of DXIL; dynamic 71 ms, 70 a second, 14.9 KB (+25% and +17%). The 263 texts of that run are byte
  for byte the ones the variant makes.
- **Debug layer** (`D3D12_DBG=1`, `D3D12_IQ=2`), dynamic from an empty cache: no error, the overlapping ranges
  included.
- **GPU** (`RSX_CENSUS=20`, its new `[census] GPU` line), warm caches, the 20 s windows of the level that drew the same
  places: old 9.54 and 10.89 ms a frame (draws 4.82 and 6.56), dynamic 8.89 and 10.89 (draws 4.52 and 6.44), with
  1,037/971 frames against 1,071/985. No cost seen on the RX 6700 XT.
- **Pictures** at 70, 85 and 100 s, both warm: the street, the parasite fight, the courtyard; the same materials, the
  metal's reflections, the parasites' glow, blood, HUD.
- **Shadows on** (`--set patches.disable_shadows=off`, off in the Smooth preset): one comparing text in each mode, the
  shadow mask on unit 5 (GEQUAL, bilinear). Its dynamic text is the old one with each fetch through its helper,
  `_rsx_gamma_dyn` around it (lanes 0 here, so the same), and the sRGB conversion under its flag.

## Not tried yet

- vkd3d-proton (Proton) with the aliased ranges: the test bed did not come with the move to this PC. Before the
  switch is on by default.
- A cold driver cache, other GPUs (NVIDIA, Intel), MSAA targets (no AA grid in this title so far).

## Next

Step 2: the fragment programs found in the archives (30,572 of the single-player ones,
`2026-11-18-pixel-shader-prebuild-study.md`) built by the precompile in the background, with the switch on.
