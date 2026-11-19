# A report's saves as its sessions found them (2026-11-16)

## Why

Issue #9's latest report held the player's saves for the first time, and a session in it was played again here from
them (`PAD_REPLAY`, `temple_repro.ps1 -SaveDir`): it went to the temple where the session had gone to Kirra. The saves in
a report are as the last session left them. That session had written the profile at its end (`DATFEST` and `DAT-31`,
20:48), and the game, reading the new `DATFEST`, loaded another checkpoint (`DAT-12` where the session read `DAT-13`).
Save backups (`system.save_backups`, five by default) keep the copy from before each such write, in
`<savedata>_backup/<folder>/<YYYYMMDD-HHMMSS>`, but the report left them out.

Two things stood in the way of using them:

1. The copies had the time of the copy. A profile's `DATFEST` holds the time it was written (a big-endian u32 at 4), and
   the game refuses a save whose files have another time (issue #7, [notes](2026-11-12-grotto-freeze.md)). The issue's
   `DATFEST` holds that one time only, the save's own, not one for each slot.
2. Nothing said which copy goes with which session.

## What changed

- **cellSaveData (fork):** `copy_host_file` gives the copy its source's time (`_utime`, `utime` off Windows). A backup
  is loaded as it was saved. `test_savedata_backup.c` checks the time of a kept copy.
- **report.cpp:** with the saves, for each session in the report, each save folder written since the session began has
  its first copy made after the start packed, once, under `save-backup/<folder>/<copy>/`, with the files' times in
  `save-backup/times.txt`. `save-backup/starts.txt` says which copies make each session's start
  (`<session> <folder> <copy>`, and `oldest-kept` when that copy is the oldest of a full set, so an older one may have
  been dropped). A folder with no copy since a session began is as `save/` has it. `report.txt` says the same per
  session.

To play a session again: `save/`, with each folder `starts.txt` names for it replaced by that copy, file times from the
two `times.txt`. A copy made before this build has the time of the copy; its `DATFEST` says when the save was written,
and its files go at that time, as #7's did.

## Verification

- The temple's replay bed, 45 s: the game writes `BCUS98232-SYSDATA` at boot; its new backup's four files have the
  times the save had (`ICON0.PNG`, `PARAM.SFO` and `SAVEDATA` 1794451817, `PARAM.PFD` 1794451772), where the copy
  before had 10:06 of today.
- `Ascension-R --report --with-saves` on a settings folder with the replay's session 100537 and that bed's backup of
  10:06:02: the zip has `save-backup/BCUS98232-SYSDATA/20261116-100602/` (4 files), `starts.txt` says
  `20261116-100537 BCUS98232-SYSDATA 20261116-100602`, and `report.txt` "BCUS98232-SYSDATA from the copy of
  20261116-100602"; the profile, not written in that session, is as `save/` has it.
