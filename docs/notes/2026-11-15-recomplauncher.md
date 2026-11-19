# RecompLauncher support (2026-11-15)

[RecompLauncher](https://recomplauncher.com) installs recomps from their GitHub releases and keeps them up to date;
Ascension-R was already in its catalog (`ascension-r`, from `rigaz29/Ascension-R`, the Kit zip taken as the Windows
build). `recomplauncher.json`, beside `Ascension-R.exe` in every release (`gowa.py package`) and in `build\` (`gowa.py
build`, for its Developer tools' "Add a build"), says how to start it and what its settings are. The guide and the
schema: https://recomplauncher.com/sdk-files/recomplauncher-agent.md and /schema/recomplauncher.json (the two schemas
were the same).

## What the file says

- **Level 1, start only.** No `game`: the program finds the game itself, and the first start's work (the game's
  folder, RPCS3 and the `.rap`, the decryption, the game's code built in about ten minutes) stays in Ascension-R's own
  launcher. `start.args` is `--set launcher.show=off`: an override for that run alone (`ps3_settings_override`, never
  written to the file), so the launcher's window is skipped once the game can start, and still forced when it cannot
  (no `GOWA.elf`, the game's DLL missing or built for another version). Shift still brings it back. The setting
  "Ascension-R's launcher" (`"in": "args"`) adds `--set launcher.show=on` after it: the last override wins
  (`ps3_settings.c` looks them up from the last).
- **Level 2, the settings.** The INI where the program reads it (`default_settings_path`): `{install}/ascension-r.ini`
  when it is there or `portable.txt` is, else `{local}/Ascension-R/ascension-r.ini`. 34 settings in six groups, keys
  and values as `launcher.cpp`'s `s_opt` and the patches' catalog have them, defaults the program's own; Preset sets
  the five patches as the launcher's presets do. Frame generation is greyed out unless MLAA is off (it needs it).
- **Saves and logs.** `<settings folder>\gamedata\dev_hdd0\home\00000001\savedata` (`paths.hdd0` as the launcher fills
  it), `<settings folder>\logs`. Exit code 3, a file of the game missing (`game_open_failed`), has its message.
- **Not done: Level 3.** `prepare` would be a windowless step that links the player's folder, decrypts it with RPCS3
  and builds the game's code, printing `progress <0-1> <text>` lines. `--prepare-game` does the decryption alone; the
  rest is in the launcher's code (`game_use`, `kit_start`). `game.dumps` does not apply: the game is a PSN download,
  not a disc Redump lists.

## Checked

- The file against the schema, with a validator of the draft-07 keywords it uses (it found the seven faults put into a
  broken copy), and the guide's own rules (each setting's keys in its defaults, its group listed, actions naming
  folders).
- The start arguments on the dev build, test settings and the replay's test hdd0: with `--set launcher.show=off` the
  game's window opened within 30 s; with `--set launcher.show=on` after it, the launcher's window opened and waited.
- No code changed: the program is the same for a player who does not use RecompLauncher.
