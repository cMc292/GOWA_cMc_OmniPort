# The EU disc at 1.12 against our GOWA.elf (2026-11-17)

Asked for by issue #4 (players asking for the EU disc, BCES01741, and the US disc, BCUS98232). The question is whether
the EU disc's game at 1.12 is the program the port is built from, `GOWA.elf` of NPUA80918 1.12 (SHA-256 `4bf75c89...`).

## What was looked at

- The user's EU disc dump: a decrypted ISO (`PS3VOLUME`, the sector 0 region table present, every file outside the
  clear regions reads as plain data), BCES01741, APP_VER 01.00, category DG. Its extraction was checked against the
  ISO: 114 files, sizes the same, the first and last 64 KiB and both sides of every extent boundary of the 13 files
  over 1 GiB the same. The ISO9660 names write `-` as `_` (`MP_ENGLISH.PSARC`); the real (UDF) names, and the game's
  own `LOCALIZED_ARCHIVES`, have `MP-ENGLISH.PSARC`.
- Its updates 1.04 to 1.12 (`EP9000-BCES01741_00-GOWABDPATCH00001-A0104` ... `A0112`, 3.05 GB in all; 1.05, 1.07
  and 1.12 are 8.5 MB, the executable only), installed by RPCS3.

## The disc and its update

- The disc at 1.00 runs `USRDIR/EBOOT.BIN`, 8.6 MB, a disc SELF (app type APP).
- The update installs a 71 KB `EBOOT.BIN` that starts `USRDIR/GOWA.SELF`, as NPUA80918 does, and `GOWA.SELF` itself,
  `TLOUCOUNTDOWN.SELF` and patch archives (`TPPS.PSARC`, `TPPS1` to `TPPS5`, and per-language `TPPS2-*`, `TPPS4-*`).
- The disc's archives: `SHELL.PSARC`, `INTROCIN.PSARC` and `RESOURCES.PSARC` have NPUA80918's sizes; `GAME.PSARC`
  differs by 462 bytes (the disc's 1.00); ten language archives (English, French, Spanish, German, Italian,
  Portuguese, Russian, Polish, Turkish, Greek) and their `MP-*` ones.

## GOWA.SELF, EU against US

| | EU (BCES01741 1.12) | US (NPUA80918 1.12) |
|---|---|---|
| SELF | NPDRM, key revision 0x1C, ELF data 8,717,136 bytes | the same |
| license | free, app type 32, content id `EP9000-BCES01741_00-GOWABDPATCH00001` | local (the `.rap`) |
| ELF header (entry 0x8283F8) | the same as our GOWA.elf | the same |
| 8 program headers, 31 section headers | byte for byte our GOWA.elf's | the same |
| segments in the SELF | uncompressed, our GOWA.elf's sizes | the same |

The ELF also holds every region's title ids and save names (`BCES01741_00`, `BCUS98232_00`, ... at 0x6764483), the
table the game picks its region from while it runs: one program for all regions.

## The bytes: the same file

**The EU 1.12 `GOWA.elf` is our `GOWA.elf`: SHA-256 `4bf75c89...62b5` for both, and `cmp` finds no byte that differs.**

- RPCS3 named its PPU cache for the EU game `ppu-CZX99Q7PX8cN5P8LbjdjNq35CjvH-GOWA.SELF`, the name it has for NPUA80918
  (the name is a hash of the program), and the 71 KB launchers share theirs too (`ppu-6CYHxcJx...-EBOOT.BIN`).
- How it was decrypted. `rpcs3 --decrypt` on a copy (RPCS3 0.0.43, `RPCS3_CONFIG_DIR` pointed at a folder of ours)
  fails without a key, `SELF: Failed to decrypt SELF metadata info!`: a free-license SELF that another program starts
  is decrypted with the klicensee that program hands to `sceNpDrmIsAvailable2`, here the 71 KB `EBOOT.BIN`. RPCS3
  logs it when the game boots, `sceNp: npDrmIsAvailable(): KLicense key or KLIC=0x...` beside
  `drm_path=".../BCES01741/USRDIR/GOWA.SELF"`. After one boot of the EU game, the same `--decrypt` with that KLIC on
  its **standard input** (it asks `Enter KLIC of GOWA.SELF`, and reads stdin, not the console) wrote `GOWA.elf`.
  The key was kept in an ignored local file for the run and deleted after.

## What it means for EU support

The lifted code, the hooks, the patches and the native SPU work all apply as they are: the Kit builds the same DLL from
an EU copy. EU support is the disc layout: the extracted disc plus the update's folder (`dev_hdd0/game/BCES01741`),
BCES01741 as title and boot path, the disc's archive lists (ten languages, `TPPS*` patch archives), saves under
BCES01741, and the launcher finding both. The US disc (BCUS98232) likely the same; not checked.

Decrypting an EU copy cannot use a `.rap` as NPUA80918 does, and the port ships no keys. What works with what the
player already has: boot the game once in their RPCS3, read the KLIC from that RPCS3's log (the line above, for
`GOWA.SELF`), and give it to `rpcs3 --decrypt` on standard input.
