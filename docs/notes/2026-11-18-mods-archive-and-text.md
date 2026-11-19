# The mods folder, step 2: archive\ and text\ as one more update (2026-11-18)

Step 1 ([note](2026-11-18-mods-folder.md)) took a mod's files over USRDIR. A mod that changes something inside the
archives then had to bring an update of its own (`TPPS6.PSARC` and `PATCHES`), and two such mods could not both be
on. Step 2 reads two more folders of a mod and packs them, for every mod that is on, into one update the game reads
last:

- `archive\`: files for entries of the archives, by their names there (`archive\WAD\PS3_BE\SHELL.WAD`).
- `text\`: lines of the game's text, as `gowa.py text export` writes them (`en.tsv`, `fr\menus.tsv`).

## How the game takes an update's entry

The question first: how to put a mod's whole `SHELL.WAD` in, when the updates change that WAD through a
`SHELL.WAD.PATCH`.

- **The updates never bring a whole WAD of the base archives.** Of the 2,582 names in the EU disc's 19 archives, the
  updates (`TPPS.PSARC`, `TPPS1` to `TPPS5`) bring 142 again whole: 138 DCB files, two JSON, one GMX and one IFF, and
  no WAD. A base WAD is only ever changed by a `.PATCH`: 425 `.WAD.PATCH`, 43 `.TS.PATCH` and 11 `.AS.PATCH` in the
  updates. The whole WADs they bring are new ones, which a later update patches in turn (44 of them, such as
  `PHIARENA.WAD`: whole in `TPPS1`, patched in `TPPS2` to `TPPS5`).
- **The game looks for `<name>.PATCH` when it loads a file.** `func_002C2F58` copies the file's name, appends
  `.PATCH` (the string at `0x679110`) and looks it up, when a bit of the request (`0x00100000`) is set. `.PATCH.DIFF`
  is in the code too.
- **Four runs of the parasite-fight replay** on the EU disc, each with a mod of `files\TPPS6.PSARC` and `PATCHES`
  holding `6`, SELECT pressed at 80 s for the pause menu:

  | the mod's `TPPS6.PSARC` | language | the pause menu |
  |---|---|---|
  | the whole `R_LANG_EN.WAD`, two lines changed | EN | the game's own text |
  | a one-step `R_LANG_EN.WAD.PATCH`, the same | EN | the game's own text |
  | the whole `R_LANG_EN-GB.WAD`, two lines changed | EN-GB | none: the game never got past its load (60 flips a second, a black screen, to 84 s) |
  | a one-step `R_LANG_EN-GB.WAD.PATCH`, the same | EN-GB | "One step", "One step continue" |

  - **The first two changed a file the EU disc does not read.** Its `LANGS_BCES01741.TXT` has no `en`, only
    `18 en-gb GB_ENGLISH`, so with the system language at 1 (English, US) it shows `R_LANG_EN-GB.WAD`. The log had
    `TPPS6.PSARC` opened in each.
  - **A whole WAD in the newest update gets the older update's `.PATCH` put over it.** The run hung, as a WAD would
    whose patch copies ranges of a base that is not the one it was made for. It is how `PHIARENA.WAD` works: the
    patches of `TPPS2` to `TPPS5` are over `TPPS1`'s whole one.
  - **A `.PATCH` of one step works.** It takes all of its bytes from its own data (`4, 1, 0x80000000 | 0, size`), so
    whatever the base is, the file is the mod's.

So the packer puts an entry in as a one-step `.PATCH` when the game's updates have a `.PATCH` of that name, and whole
otherwise (as the updates bring the DCB files). A mod's own `.PATCH` goes in as it is.

## The packer: scripts/modpack.py

In Python, as `gowa.py text` is: it takes `gametext.py`'s archive reader and writer and its text checks, Unicode
letters included, rather than a second copy of them in C++. The game starts it with the Kit's Python
(`kit\python\python.exe`, which builds the game's code too); a build tree's is the `.venv` beside the tree, else one
on the PATH; `GOWA_PYTHON` and `GOWA_MODPACK` name others. It is in the Kit's scripts.

- **Its input** (`mods\.built\input.txt`): the game's update folder (`<hdd0>\game\<title>\USRDIR`), the number of
  updates before this one, and each mod that is on and has archive or text files, in the list's order.
- **The number.** The game's own `PATCHES`, or the one the first mod with a `PATCHES` in its files brings, if more: a
  mod with its own `TPPS7.PSARC` and `PATCHES` 7 gets the pack as `TPPS8.PSARC` after it. The pack's folder comes
  first in `PS3_MODS_DIRS`, so its `PATCHES` is the one the game reads.
- **Text.** For each language a mod has lines for, both of its patches (`R_LANG_<lang>.WAD.PATCH` and the `_MP` one)
  from the game's newest, with the lines put in by `text build`'s `apply`, mod after mod: a line a mod above has is
  left out, and so is a line that fails a check, where `text build` would refuse the file. English, `en`, is EN and
  EN-GB, unless the mod has an `en-gb` file: the UK table's keys are the US one's but 39 (8,813 and 8,852), and a key
  it lacks is not called a fault there.
- **The update**: `TPPS<n+1>.PSARC` (zlib level 1, the blocks that do not shrink stored) and `PATCHES`, and
  `result.txt`, what each mod gave.
- **When.** At the game's start (`title_environment`, not the precompile's), when a stamp differs: the packing mods'
  archive and text files (names, sizes, times) in order, and the game's side (`PATCHES`, the updates' sizes and times,
  the number before, the packer's own files). The launcher shows what the last start made, as long as the mods are
  as they were then.

## The tests

- `tests/test_modpack.py` (CI): entries that the updates patch, whole ones, a new WAD, a mod's own `.PATCH`, a file
  a mod above has, Windows' `Thumbs.db`; text lines mod after mod, English for both, a line the game lacks, a
  language it lacks, a language a mod brings whole; the names of languages; the command and its `result.txt`.
- `tests/test_mods.py` (CI), the C++ side with the real packer: packed once as `TPPS8.PSARC` after a mod's own
  seventh, the pack's folder first, not packed again when nothing changed (its `result.txt` untouched), the
  launcher's look before and after a file changed, packed again with a line left out; no Python; Use mods off.
- **The game**, two runs of the replay with screenshots at 83 s, where the fight's prompts show:
  - `text\en.tsv` with five lines, among them `1927.2` to `1927.4`: "Tekan □ untuk serangan cepat.", "Tekan △ untuk
    serangan berat.", "Tekan ○ untuk serangan fisik." on the EU disc, its EN-GB taking the `en` file. The log:
    `[mods] pack: one more update (0.2 s)`, `Teks uji 0.1 (text EN: 5 lines; text EN-GB: 5 lines)`. A run before,
    with two lines, had packed it first; this one packed it again for the changed file.
  - `archive\WAD\PS3_BE\R_LANG_EN-GB.WAD`, the whole WAD with those lines changed, and the text mod after it: "Press □
    for a WHOLE WAD attack." and the others, the WAD having gone in as a one-step `.PATCH` (`archive Arsip 1 1 0`),
    and the text mod's EN-GB lines left out: "0 lines, 5 left out (Arsip brings the EN-GB text whole)", which the
    Mods tab shows in the warning colour.
