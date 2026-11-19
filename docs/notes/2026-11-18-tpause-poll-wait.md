# TPAUSE for the main thread's polling waits: no gain, not kept (2026-11-18)

`port/poll_wait.cpp` makes the main thread's polling loops wait 20 us after 8 passes. On AMD the wait is MWAITX with
its timer, which hands the core to the SMT sibling; on Intel, which has no MONITORX, it is a spin on PAUSE. The
question was whether Intel's TPAUSE (WAITPKG, here an i5-12400F: WAITPKG 1, MONITORX 0) would do for Intel what MWAITX
does for AMD.

## What was tried

Where CPUID 7.0 ECX[5] (WAITPKG) is set and MONITORX is not, the wait was `_tpause(1, deadline)` (C0.1) in a loop to
the same TSC deadline, as TPAUSE also ends at an interrupt or at the OS's limit on one wait. The log said
`[poll-wait] 20 us after 8 passes, TPAUSE (TSC 2496 MHz)`; a 60 s start was clean. `GOWA_POLL_WAIT_TPAUSE=0` kept PAUSE
in the same executable for the A/B.

## Measured

`gowa.py bench`, two runs a side in ABBA order, one executable, windowed, on a copy of the saves
(`--set paths.hdd0=...`, `cheats.damage_taken=0`, `--menu-fps 59.7`). The route continued the profile's checkpoint in
the first level: Kratos stands at a gate with an enemy beyond it, 56-58 flips/s, GPU 56%.

| Variant | Runs (median flips/s) | All windows | `spurs pm` | `ppu main` | `frame clock` |
|---|---|---|---|---|---|
| TPAUSE | 57.22, 57.31 | 57.31 | 423% | 97.7% | 41-42% |
| PAUSE | 56.72, 57.87 | 57.22 | 427-429% | 97.3-97.7% | 41-42% |

+0.16%, against 2% between the two PAUSE runs. The main thread's CPU time is the same either way, as a TPAUSE is
still time on the thread. With six cores, twelve threads and about ten busy ones, an SPU worker that shares a core
with the main thread is not what holds the frame here.

## Not kept

`port/poll_wait.cpp` is as before. The patch is a dozen lines (the detection beside the MONITORX one, the loop, and the
branch in `gowa_poll_wait`), should a scene or a CPU with fewer cores ever show a reason to try it again.
