# Ultrawide (2026-11-10)

`display.ultrawide` (off by default) draws the game to the full width of a screen wider than 16:9. At 16:9, or with the
setting off, `rsx_d3d12_backend_wide_ratio()` returns 1, the hooks do nothing and no program text changes.

## The camera

The render settings are at `0x9B99B8` (getter `func_003B6180`). `+32`, `0x9B99D8`, holds the aspect, written once by
`func_003A3DD8`. The title keeps a camera's horizontal half-width and divides it by that aspect for the vertical one, so
a wider aspect alone crops the picture top and bottom. From the matrix at `c264`, at 21:9:

| | x scale | y scale |
|---|---|---|
| 16:9 | 2.2315 | 3.9671 |
| aspect only | 2.2315 | 5.2067 |
| aspect and half-width | 1.7002 | 3.9671 |

The hooks are in `port/ultrawide.cpp` and `scripts/ppu_hooks.json`:

- `0x003A3F90`: the aspect, 16:9 times the screen's ratio to 16:9.
- `0x003B05A4` in `func_003B0380`: the tangent of a perspective camera's field of view, times that ratio.
- `0x003AFE1C` in `func_003AFDC8` and `0x003B06F4` in `func_003B0380`: an orthographic camera's half-width, times that ratio.
  The 2D screens go through those cameras.
- `0x003148F0` in `func_00314780`: the camera controller turns its framing limits into angles with the aspect. That is how
  the camera follows Kratos, not what it shows, so it stays at 16:9.

A hardware watchpoint on `+32` during play caught those readers and no others. `func_00442568` reads it as well and puts
movies between bars by itself.

## 2D

With the orthographic cameras wider, the title lays its 2D out in the 16:9 in the middle without any help. A first version
also kept the HUD and movies narrow in the backend, and they came out 1.31 times too narrow (the death screen's text 638
pixels wide against 838 at 16:9). That code is gone. Two things are left to the backend:

- A flat vertex at or past the edge of the 16:9 goes as far past the edge of the screen, for draws on the display target in
  a frame with a 3D scene (a line added to the vertex programs, asked for through `vp_posoffset.w`). Fades, the death
  screen's backdrop and a cutscene's bars reach the sides that way.
- A frame with no 3D scene gets black bars beside the 16:9 (`present_to_backbuffer`). A scene is at least
  `WIDE_SCENE_MIN` (8) draws through the camera into a target the size of the display other than the display itself,
  counted per frame between flips and carried to the present on the frame's FLIP record (2026-11-16, below).

## The pause menu (2026-11-16)

Its backdrop is not 2D: it is a mesh in front of the menu's camera, made to fill a 16:9 view, drawn in two or so draws
into the scene's target (the dark panel and its red glow), over the last frame of play left in that target; the item's 3D
model is drawn on the display itself. At a wider view the mesh covered the middle only, 21% to 79% of the width at 32:9,
and that last frame of play showed beside it. Found with a recording cut in the first fight and a START press added
(scratchpad `pad_pause.py`), `RSX_CAP` frames at 32:9 and the targets' BMPs.

The flat-vertex rule cannot reach a perspective mesh, and the frame counted as a scene: one camera draw was enough. Now a
scene takes eight such draws into an offscreen target the size of the display, which play makes by the thousand (1,065 in
the scene's target in a frame of the first fight) and the pause menu by the few (4). The pause menu gets black bars beside
its 16:9, as it looks on the PS3. The count is made per frame on the walk and kept on its FLIP record: counted at the
present instead, the batches of one menu frame added up to a scene.

Checked in a 1600x450 window (32:9): play full width, the pause menu between bars.

## Drawing wider

Targets the size of the display are `width * scale * ratio` wide while the guest goes on seeing its own size, the same path
as internal resolution. On those targets `fp_alpha.w` is `-1/sy`, which tells a fragment program's WPOS to divide x by the
ratio as well.

## Verification

In a 1680x720 window (no ultrawide monitor here), with FSR 1 at 1.5x and 1x and FSR 3.1 with and without frame generation:

- Intro movies: 16:9 between black bars, the text as wide as at 16:9 (718 pixels).
- HUD in a fight, in the 16:9 in the middle. Death screen: text 836 pixels against 838, backdrop out to the edges.
- A cutscene and gameplay: nothing stretched, and more at the sides.
- Ultrawide off: 16:9 pictures as before, and no crash in any run.

## Not fixed, not tried

- Frame generation's pass for what moves (`fg_objects`) is off at these widths: with it on, close-ups of Kratos got coloured
  noise at 21:9 and not at 16:9. The cause was not found. Frame generation goes by the camera alone there.
- The first session at a wider aspect compiles the vertex programs again, because their text differs; until that is done,
  a cutscene can show dark or missing objects.
- A real ultrawide monitor; 32:9 only in windows (1600x450, 1920x540). The sides of some cutscenes may show places the
  camera was never meant to see.

## Issue #23: stretched pictures and HUD (2026-11-18)

At 5120x1440 the interface stretched. Reproduced on the EU disc in a 1920x540 window with the fight recording replayed
(`PAD_REPLAY`), the game's own F12 screenshots and `RSX_CAP` frames:

- The studio logos, "Sony Computer Entertainment presents" and the online notice came out twice as wide, the notice
  cut by the bars beside the 16:9. Each is one triangle over the whole display, through a camera-less program,
  sampling a 1280x720 picture in main memory (`0x75600000`, `0x75A00000`, `0x75E00000`; dumped with `RSX_CAP_MEM`,
  the pictures themselves are plain 16:9). The movies' own quad (`func_00442568`, four vertexes) takes its width
  from the aspect; these do not.
- The HUD's blade, top left, now and then reached from its place to the screen's left edge. At rest the HUD is
  where it should be; the edge rule above moved every flat vertex at or past the 16:9's edge, so a piece out past
  the edge, as the HUD slides in or out, was pulled to the screen's edge and stretched across the side.

Both in `vp_record_cb` and the vertex programs' added line:

- A draw on the display through no camera whose first texture is in main memory at the display's size has
  `vp_posoffset.w` -1: its vertexes at or past the screen's edges are divided by the ratio, which puts the picture
  in the 16:9. A quad already inside (the movies) has no vertex there and does not move.
- With `vp_posoffset.w` 1, only a flat vertex within two of the title's pixels of the 16:9's edge goes to the
  screen's edge. Fades, the tutorial's band and the title screen's brush strokes end on that edge and still reach
  the screen's; a piece well past it stays where the title put it.

Checked at 32:9 after the change: the logos, the notice and the intro movie in the 16:9 at their shapes; the title
screen and the profile menu; the HUD in a hundred screenshots 0.1 s apart as it came up and over the fight; the
load into play. Not caught: a frame of the HUD half way in (it comes up between two screenshots), the death screen
and a cutscene's bars.
