# The pixel shaders on the precompile screen (2026-11-18)

The pixel shaders built ahead (`2026-11-18-pixel-shader-prebuild-step2.md` and after) were built only by the
launcher's Build all shaders now. Now the precompile gate builds them too, on its 8-bit screen, after the vertex
shaders: the first start of each version has both.

- **Two builds, one screen** (`port/loading_screen.cpp`): the vertex shaders as before (`--precompile-shaders`, the
  game waits for them), then the pixel shaders (`--prebuild-pixel-shaders`) when `graphics.shader_prebuild` is on, DXC
  is there and they are not done for this version: their file missing or of another form
  (`rsx_d3d12_prebuilt_current`), or its `.done` naming another version. The gate opens when either is due; on the
  pixel shaders alone it opens on them.
- **The player may leave them**: Enter (or Space), or a controller's Start (XInput, asked a few times a second while
  they build), ends that build where it is; what it wrote is whole (each record's tag last), the game starts, and the
  next start goes on from there, as the launcher's Build does. The screen says so ("Pixel shaders: the rest later"). Closing the
  window still quits, as during the vertex shaders. Not Esc, which the first try used: in the game it closes the game
  when `system.esc_quits` is on, so on this screen it now does the same (as closing the window), and nothing when it
  is off (the port's default).
- **What it costs**: the first time some 7 minutes on this PC's six cores (39,598 programs), after an update the time
  to go over the file, 9 s here; the vertex shaders' build failing leaves them for the next start too.
- Never while the game plays: built beside it they cost 7% of the frames.

## Checked

- The screen's two states (`--loading-preview`: `6_pixel_building`, `7_pixel_later`).
- The vertex shaders done, no pixel shaders: the gate opened on them; the key at 20 s left them at 1,970 of 49,996, and
  the game took the 1,940 pixel shaders already built.
- A whole file whose mark named an older version: gone over in 9 s, then marked; the next start had no gate.
- No vertex marker (an update) with both lists there: the vertex shaders in 1 s, then the pixel shaders in 9 s, and
  the game with 39,598 built ahead.
- Not tried: a controller's Start (no controller here); it leaves as Enter does, through the same call.
