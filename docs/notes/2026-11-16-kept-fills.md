# Kept draws that miss the resident part, counted (issue #9, Kirra, 2026-11-16)

## The report

Issue #9, comment 6903124589 (0.1.0-alpha.11; Ryzen 7 5700X3D, RX 7900 XT; 1920x1080 at 165 Hz; internal resolution
1.5, FSR 3.1): "on the water levels or like in this first screenshot". Two screenshots with RTSS's overlay: a cave in
Kirra at 25 frames a second with the GPU at 33% and the CPU at 45%; the harbor ("El puerto de Crisa") at 59, average 42.

## What the session shows

The session's log and its pad recording name the same 2,917 file opens in the same order, so every log line can be put
on the recording's clock. Session 164616, 37 minutes:

| Session time | | Flips a second |
|---|---|---|
| 0:00-18:40 | Kirra, CYN525 to CYN850: 16,384 kept fills in 18 minutes | about 64 |
| 18:40 | CYN900, CYN900LIGHTS and CYN950 load | |
| 19:03-19:47 | kept fills from 16,384 to 524,288: about 11,500 a second, 550 a frame | 45, then 22, then 21 |
| about 20:40 | back | 39, then 63 |
| 22:52 | "the frame's index buffer is full (4192707 of 4194304 indices)", batches to the CPU's conversion | 52 |

No video memory cuts, crashes, lost devices or failed pipelines; fewer waits for RSX copies (issue #15's) in the drop,
not more.

A fill is a draw of a kept block that did not go through the resident part (`upload_tris_kept` returned 0): the block's
bytes go back into the ring, and the raw path copies them again and converts the indexes, on the CPU. With the GPU at a
third, the frames waited on that. Why 550 draws a frame stopped going through the resident part there, the log could not
say: `upload_tris_kept` turns a draw away for 17 reasons, and none was counted. Only one restart of the resident part came
in the 44 s, so it was not that.

## The counters

Each reason has a counter now, and so does a draw of a kept block that went to another path than the raw one. The
`[vtx-kept] N fills` line, said at each power of two as before, ends with the reasons that are not zero and how many times
the frame's index buffer was full. The temple, from issue #9's session 204820 (`temple_repro.ps1`, 150 s):

```
[vtx-kept] 32 fills, 214605 draws from the resident part (starting over 26, no room 6)
```

With `VTX_RESIDENT_MB=8`, 87 restarts in the same 150 s:

```
[vtx-kept] 8192 fills, 152634 draws from the resident part (starting over 7072, no room 1120)
```

In both the reasons add up to the fills. They are counted on the declines only, so the draws that do go through the
resident part cost nothing more. The first 230 s of play1 have no kept draws and print no such line, as before.

## Next

The next report from Kirra names the reason. A save there from #9's reporter, with this session's pad recording, would
let the drop be replayed here.
