# Deleting a profile, and the system's own questions (2026-11-18)

Issue #24: in Game Profiles, Triangle (Delete Profile) flashed something and deleted nothing, every time.

## What the game does

Each press starts a guest thread, `SaveDataUtil::DeleteSaveProfile` (OPD `0x831C80`, code `0x2CBA70`), that only calls
`cellSaveDataFixedDelete` (NID `0xCED17573`): the prefix is the profile's own folder (`BCES01741-PROFDATA-1`), the
fixed callback (`0x2CC764`) names the listed save whose listParam is `PROF0`, and the done callback (`0x2CC868`) stores
`excResult` at `0x940A24`. Before the thread starts the game sets that word to 1; once the thread is done it counts the
profile deleted only if the word is 0. The game asks nothing itself: on a PS3 the system's save data utility asks
inside FixedDelete.

The runtime had no FixedDelete. An unregistered import returns 0 without calling anything, so the word stayed 1 and the
game went back to its list. The log never said so: `[hle] unresolved NID` was capped at 40 lines a run, and
`cellSpursJoinTask2` (`0xA7A94892`) used them all at boot. Found from the reporter's pad recording, whose thread marks
show the delete thread starting 0.2 s after each Triangle.

## What changed (ps3recomp)

- `cellSaveDataFixedDelete`: list, fixed callback, the system's question (unless `CELL_SAVEDATA_OPTION_NOCONFIRM`),
  a copy as `save_backup` makes before a write, the folder deleted, done callback with `excResult` 0 or
  `CELL_SAVEDATA_RET_CANCEL`.
- The listing fills each save's listParam from its PARAM.SFO (`SAVEDATA_LIST_PARAM`); it was left empty, and the game's
  fixed callback matches on it.
- `sys_dialog.c`: a question the system asks, drawn in the D3D12 overlay (heading, the save's ICON0.PNG, its title,
  subtitle and first line of detail, Yes and No with the cursor on No, the dimmed picture behind), answered with the pad
  (the enter button per `input.enter_button`, Left and Right move) or the keyboard (Enter, Esc, arrows). While it is up,
  and until what answered it is let go, the title's pad reads come back empty: a Back held a moment longer otherwise
  reached the game and took it to its main menu.
- Unresolved NIDs are reported once each, at their first call.

## Tested

EU disc, a copy of the saves in `build\replay-hdd0` with a second profile (PROFDATA-0 copied, its PARAM.SFO's folder
and subtitle patched), driven through `PAD_FILE` with the game's F12 screenshots:

- Triangle on Profile 2: the question, the save's picture and title; Circle held for 8 reports: kept, the game stayed
  on Game Profiles.
- Esc: kept, the game did not close.
- Left then Cross, and Left then Enter: deleted, the folder gone, a copy in `savedata_backup`, and Profile 2 at once
  "Create Profile".
