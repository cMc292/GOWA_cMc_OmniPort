# One-frame flashes with MLAA on: RSX copies made before the draws ahead of them had read their textures (2026-11-15)

Issue #15 (0.1.0-alpha.10, RTX 5060): graphical flickering and "smudging" since earlier versions, with five videos.
Their settings: Internal resolution 2x, SMAA, and MLAA, depth of field and dynamic shadows on (Disable MLAA off), async
shaders 2, the render thread on.

## What the videos show

Counted per frame: the mean change of each frame from the one before (ffmpeg `tblend` difference and `signalstats`),
and from the frame two before, so a frame that differs from both neighbours while they are alike counts as a flash and a
camera cut does not. The cinematic of Kratos in chains had 38 flashes in 13.6 s. Each is one frame: the background
washed out and sharp where the depth of field blurs it, the chains in rainbow colours, sharp shadows of the chains on a
white wall; elsewhere single frames with large translucent orange triangles or noise on a wall.

## Reproduced

- The reporter's pad recording of their 0.1.0-alpha.10 session (from a new profile) on a fresh test hdd0, their
  settings, the window recorded at 60 fps: the flashes came at the same cinematic, 12 to 30 a minute there.
- For shorter runs, the user played the same path once (the brightness screen, a new game, the cinematic, the first
  fight), and its recording was played in the A/B below: 5 minutes a run, the cinematic at about 3:40.

| Run (the same recording, 5 min) | Flashes, score 8+ | score 15+ |
|---|---|---|
| as the reporter has it | 27 | 16 |
| MLAA off (the reporter's recording, 8 min) | 1 | 0 |
| `TEX_CHECK_ALL=1` (every part of a texture compared each frame) | 16 | 9 |
| `TEX_NOCACHE=1` (every texture read again each frame) | 4 | 0 |
| every texture-read label held until the textures before it were read | 1 | 1 |
| label 0x4E alone held | 26 | 9 |
| label 0x4F alone held | 22 | 12 |
| the fix | 0 | 0 |
| the fix, as committed (the one in the second minute, none in the cinematic) | 1 | 0 |

What it was not:
- The asynchronous pipeline builds: `async_shaders = 0` flashed as often.
- The exposure's 1x1 surface written back to main memory: no zero reached it in the cinematic (one run of 45 zeros was
  a black screen).
- The blit snapshots of the frame for MLAA: none was turned down for its age or size, and the display buffer held the
  scene, not the composite, when it was copied.
- The flips: every present had one flip op, none dropped.
- A change the texture cache's one-part check missed: only the opening movie's texture, never in the cinematic.
- The title acting on a texture-read label (`TEXTURE_READ_SEMAPHORE_RELEASE`, 0x1D74) that the walk wrote before the
  render thread had read the textures: holding labels 0x4E (the VRAM manager's count) or 0x4F (released after the
  composite reads its 1280x720 surface) changed nothing. Holding all of them did, but only because the title releases
  ~2,200 more a frame for its rings (labels 0xC8-0xD1, every draw or two), which kept the render thread within a record
  or two of the walk, at 13% of the first fight's frame rate.

## The cause

The RSX makes a copy (NV3089) after the draws before it have read their textures. The walk made each copy in guest
memory at once (`nv3089_blit`, cellGcmSys.c), while the render thread read the textures of the draws before it from
guest memory only when it recorded them (`rf_prepass`), later. A draw before a copy could sample what the copy wrote.

God of War: Ascension makes two such copies over textures still to be read:
- Its VRAM manager moves textures within VRAM with them (`CopyLocalMemory`, up to 16 MB a frame, 2026-11-12 temple
  textures note). The textures that changed while they were read were its 4096x4096 and 2048x4096 DXT atlases there.
- With MLAA on, each frame's scene is copied into a 1280x720 surface in main memory (0x70B30000 in these runs), and the
  frame after composites from it. A composite before the copy got the scene as copied, before the post-processing that
  follows: the washed-out frame, sharp where the depth of field blurs it. With MLAA off this copy is not made.

## The fix

cellGcmSys.c tells a renderer of each NV3089 copy's destination before it writes it (`cellGcm_copy_write_observer`).
The D3D12 backend (`d3d12_copy_write_observed`) looks through the records the render thread has not read yet, and when
a texture one of them samples overlaps the destination, the walk takes the render thread's lock and reads and records
the records so far (`rf_ahead_force`) before the copy goes on. In the first cinematic ~3.6 copies a second need it.
`RSX_COPY_TEX_BARRIER=0` turns it off.

Cost, the first fight (`gowa.py bench`, two runs each, ABBA): on 30.26 flips/s (30.26, 30.23), off 30.23 (30.23,
30.23), both at the title's 30 fps lock. Holding every texture-read label had taken the same bench to 26.26 there, so a
cost of that size would show; a smaller one would not, at the lock.

## Open

- Videos 4 and 5, the "smudging" (in video 4 a white speckled patch on Megaera), were not looked at yet.
