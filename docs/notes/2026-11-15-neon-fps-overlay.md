# The frame rate on screen: the neon panel (2026-11-15)

The overlay (`display.show_fps = overlay`, "On screen") was the stats panel: the GPU, CPU, PPU, SPU and RSX load, memory
with its totals, the frame rate and frame time, a graph, the graphics card's and CPU's names and Kratos's position. The
user asked for a frame rate panel in an 8-bit, retro-futuristic neon look that names no hardware. Five directions were
mocked up (a score counter, a cabinet panel, a synthwave grid, a CRT terminal, a strip along the top), and the user took
the cabinet panel.

## What it shows

- FPS: the presents of the last half second, as before. Cyan from 29, amber from 20, red under that, as the game holds 30.
- The mean frame time of the same half second.
- A graph of the last frames, two to a bar, the slower of each pair, newest on the right, against the median of them:
  cyan, amber from 1.25 times it, red from 1.8 times (a hitch). At 1.15 a third of the bars of the 60-frame opening movie
  were amber, from presents one and two vblanks apart.
- No hardware and no totals.

`show_fps = stats` in `ascension-r.ini` keeps the stats panel, for diagnosis. The GPU timestamps and the thread sampler it
reads now run only for it; they ran for the overlay before.

## How it is drawn

The stats panel's path: a thread of its own, below normal, paints one of two images ten times a second, the presenting
thread copies the last one finished into the overlay texture, and a quad draws it a texel to a pixel in the top right
corner.

- The layout is the design's at 720 lines, scaled by k = the window's height / 720, every font pixel a whole number of
  screen pixels so it stays sharp. k stops at 1.88, where the image (box, border and glow) fills the overlay's 640
  columns: a 4K window gets the panel a window of about 1350 lines would.
- A font of its own, 7 x 7 in an 8 x 8 cell, with only what the panel writes: the digits, `.`, `-` and A E F I M P R S T.
- The box is dark glass at 84% with a scan line every third row, the border 4 outside it with the corners left out, and
  the graph's two lines. The border's glow is the border blurred twice and added to what is behind, as light with no cover
  (alpha 0 outside the box). All of it is worked out once a scale.
- The readings are painted over that each time, and their glow (the label, the number, the bars) is worked out at half
  size over the box only, blurred twice and filtered back up.

Cost, the painting thread at 720 lines over 4.8 minutes of the user's recording, one run each, timed around each paint:
with the glow in full over the whole image 3.5 ms a paint on average, 5.4 in the minute of play, the longest 12.6; at half
size over the box 1.3, 1.9 in that minute, the longest 7.2. At ten paints a second that is 1-2% of a core, on a thread
below normal priority. A taller window paints more pixels: about 2.3 times as many at 1080 lines.
