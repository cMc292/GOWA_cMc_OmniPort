# Skipping the intro, the movies and the cinematics: a review before any code (2026-11-17)

Issue #15's reporter asked for a way past the opening movie on a fresh install. This reads what the game has, and
what a patch could do. No code changed.

## What the game has

- **Its movies** are MPEG-2 streams (`.M2V`) in the archives, played by the game's own player through cellVdec (the
  calls come from the code around `0x003F4DF0-0x003F5748`, driven by `func_0020848C`); no cellDmux or cellPamf.
  About 25: the opening one before the menu (`INTROCIN.PSARC: OPENINGINT6.M2V`), the levels' (`HECA.PSARC:
  SPGAMEINT00`, `HECAHANDTRANS`, `HECAHEADTRANS`; `CRTN`, `CYN`, `FURY` with `CREDITS`, `LAN`, `STAT`, `STINT`), the
  multiplayer one (`MPGAMEINT`), and the fourteen behind-the-scenes extras (`EXTRASCIN1/2: BTS_*`). The cinematics in
  the engine are drawn as the game plays, not from a file.
- **Its own way past them:** the text has "Skip Cinematic" and "Are you sure you want to Skip?". Tried here, US
  release, a new profile:

| | The opening movie | A level's movie (SPGAMEINT00, at the first level) |
|---|---|---|
| First start (no system save yet) | nothing skips it: Square, Cross, Circle, Triangle, Start, Select; 141 s | Start opens nothing |
| The next start, same profile | Square ends it (25 s in) | Start opens **Pause: Continue, Skip Cinematic, Quit Game** |

So everything is there, and a first start closes it: the opening movie plays to its end, and the pause menu does not
open in a movie. The reporter meets that first start each time they begin from an empty folder.

## What a patch could do, smallest first

1. **Open the first start's gate** (a title patch): Square ends the opening movie and Start offers Skip Cinematic from
   the first start on, by the game's own paths. To find: the condition the first start sets (most likely a flag of the
   system save, written when the opening movie ends). Found by the two runs above with a probe on the movie player and
   the pause menu's items (the `ppu_hooks.json` way `vram_guard.cpp` uses), or by the system save before and after
   the first start. About a day; little risk, as nothing runs that the game does not run on a later start.
2. **Skip the opening movie** (a launcher option): Square pressed for the player while it plays, once 1 is in. Small.
3. **Skip every movie and cinematic by itself** (an option): the Skip Cinematic action called as each one starts, not
   pressed through the menu. A day or two more after 1. More risk: some cinematics end in play or a quick-time event,
   though the game's own skip is made to leave the game where it should be.

Not worth it: ending a movie inside cellVdec. The game's player decides when its stream ends; a decoder that stops
answering risks a hang, and it does nothing for the engine's cinematics.

alpha.14's offer to copy `%LOCALAPPDATA%\Ascension-R` into a new portable folder already keeps the system save, so the
first start's movie does not come back on a move; it comes back only when the saves themselves are new.
