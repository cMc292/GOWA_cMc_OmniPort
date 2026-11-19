# Flips one vblank period apart (2026-11-17)

On an i5-12400F with an RX 6700 XT and a 1080p display at 59.94 Hz, VSync and the frame rate cap off, the intro and
the menu ran at 62.9-64.5 flips a second while the vblank beat stayed at 59.8-60.0 (`GOWA_THREADS=10`). A PS3 completes
a VSYNC flip only at a vblank, so it never shows more than 59.94, and the game counts on that.

## The game's step

`GOWA.elf` 1.12 takes the display's rate at start (`0x3A539C`: 50.0 for a 576-line mode, otherwise 59.94 from
`0x3A52B4`) and hands it to `0x2D01E8`, which keeps it at `0x83B45C`, its rounding at `0x9424A0` and the period 1/rate
at `0x9424A4`. Each frame the sync code (`0x2D02A8`-`0x2D0548`) turns the time since the last frame into a step of 1 to
4 periods: the clamps at `0x2D0398` and `0x2D03A0`, 100 and 400 hundredths of a period from `0x2D0248` and
`0x2D024C`. Mode 4 in `0x9424A8` takes a fixed 2 periods instead.

So from 15 to 60 frames a second the game runs at the right speed, and a frame shorter than a period still counts as
a whole one. At 64 flips a second the menu ran 7% fast.

## Two causes in the frame clock

- The drain's own completion of a flip (`flip_on_reach`) waited 16 ms, not a period of 16.683 ms: 62.5 flips a second
  at most.
- With no frame cap the vblank beat completed any flip the drain had reached, also one that came a few milliseconds
  after the drain had completed the one before.

## What changed

In `port/main.cpp`:

- `flip_min_us` is one vblank period by default (1001000/60 us), and the beat checks it too, except with
  `GOWA_FLIP_ON_BEAT=1`, whose flips are a period apart by themselves. The frame rate cap and frame generation keep
  their own interval and their path as before.
- A held flip less than 2 ms from its time is completed to the microsecond: the frame clock spins to it rather than
  waiting in the 1 ms kick wait, which can wake a millisecond late on every flip.
- `GOWA_FLIP_MIN_MS=0` with `D3D12_VSYNC=0` still lets the title run past 60, for measuring it.

## Measured

| Run | Intro and menu, flips/s | vblanks/s | `frame clock` thread |
|---|---|---|---|
| before: 16 ms, the beat unchecked (borderless, `GOWA_THREADS=10`) | 62.9-64.5 | 59.8-60.0 | under 4% |
| after (windowed, `GOWA_THREADS=5`, 75 s from boot) | 59.83-60.21, mean 59.95 (and one 58.0 window at a load) | 59.8-60.2 | 9.3-10.0% |

The spin costs about 6% of a core where the game runs ahead of the vblanks. Below 60 a flip is never held, so the
fights, at 30-55 flips a second on this PC, are not touched.

## RPCS3's "Unlock FPS", for later

RPCS3's patch file has "Unlock FPS" by ZEROx and illusion for 1.04 (v2.0, eight edits); for 1.12 it was moved to the
canary list, v1.0, two edits: `0x3A52B4` = 1000.0 and `0x2D024C` = 6667.0. It moves the same rate to 1000 Hz, so a step
can be 1 ms and the game can run past 60 on a faster vblank, with 66.7 ms as the longest step. On its own it also hands
1000 to every other reader of `0x83B45C`. The 1.04 version keeps that global at 60 and points only the step's readers at
1000.0. Its sites in 1.12, each checked in our ELF:

| 1.04 v2.0 | 1.12 | In 1.12 |
|---|---|---|
| `bef32 0x0039E284` 1000 | `0x3A52B4` | 59.94, the NTSC rate |
| `bef32 0x002CAE2C` 10000 | `0x2D024C` | 400.0, the longest step |
| `bef32 0x0083B2CC` 60 | `0x83B45C` | the rate's global |
| `be32 0x002CADD8` nop | `0x2D01F8` | `stfs f1,-0x4BA4(r4)`, the rate into the global |
| `be16 0x002C537E`, `0x002CAEC6`, `0x002CAF6E`, `0x003EF71A` | `0x2CA79E`, `0x2D02E6`, `0x2D038E`, `0x3F679A` | `lfs` of the global in the time-in-frames function, the sync code (2) and the movie player |

Still to read before an option is made of it: the restore at `0x2D0608`, which writes the periods again from the
global, and where mode 4 is used.
