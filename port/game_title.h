/*
 * Ascension-R -- the releases of God of War: Ascension the port knows
 *
 * Every one runs the same GOWA.elf at 1.12 (game_find.cpp's k_elf_sha256): the game is one program for all regions,
 * and the EU disc's was compared byte for byte (docs/notes/2026-11-17-eu-disc-gowa-self.md). They differ in where
 * their files are and in how GOWA.SELF is decrypted (docs/notes/2026-11-17-eu-support-plan.md).
 *
 * The title in use is the one PARAM.SFO under <vfs>\PS3_GAME names: the launcher links that folder, and main.cpp,
 * precompile.cpp and scripts/gowa.py read it there (gowa.py has the same table, TITLES).
 */
#ifndef GOWA_GAME_TITLE_H
#define GOWA_GAME_TITLE_H

#include <stdio.h>
#include <string.h>
#include <wchar.h>

enum GameKind {
    GAME_KIND_HDD,    /* a PSN download: the whole game in /dev_hdd0/game/<id>, its updates in the same folder */
    GAME_KIND_DISC,   /* a disc: the base game in the disc's PS3_GAME, its update to 1.12 in /dev_hdd0/game/<id> */
};

struct GameTitle {
    const char* id;          /* PARAM.SFO's TITLE_ID */
    const wchar_t* name;     /* for the player: "US digital", "EU disc" */
    GameKind kind;
    const wchar_t* rap;      /* the license RPCS3 decrypts GOWA.SELF with; NULL: the KLIC the update's EBOOT.BIN holds */
    int enabled;             /* the launcher finds, decrypts and plays it */
    const char* saves;       /* its save folders' prefix: <saves>-SYSDATA, <saves>-PROFDATA-<n> */
};

static const GameTitle k_game_titles[] = {
    { "NPUA80918", L"US digital", GAME_KIND_HDD, L"UP9000-NPUA80918_00-GOWADIGITALVERS1.rap", 1, "BCUS98232" },
    /* Runs (phase 1 of the plan) and is found with its update (phase 3); its GOWA.SELF is decrypted with the KLIC of
     * the update's EBOOT.BIN (phase 4). */
    { "BCES01741", L"EU disc", GAME_KIND_DISC, NULL, 1, "BCES01741" },
    /* Not looked at yet: its GOWA.elf at 1.12 and its files. */
    { "BCUS98232", L"US disc", GAME_KIND_HDD, NULL, 1, "BCUS98232" },
};
enum { GAME_TITLES = (int)(sizeof k_game_titles / sizeof k_game_titles[0]) };

/* The title with that id, or NULL. */
static inline const GameTitle* game_title_find(const char* id)
{
    for (int i = 0; id && i < GAME_TITLES; i++)
        if (!strcmp(k_game_titles[i].id, id)) return &k_game_titles[i];
    return NULL;
}

/* The release the port was made from, and the one a folder with none is taken for. */
static inline const GameTitle* game_title_default(void) { return &k_game_titles[0]; }

/* The ids the launcher takes, for a line to the player: "NPUA80918", "NPUA80918 or BCES01741". */
static inline void game_titles_text(wchar_t* out, size_t n)
{
    size_t k = 0;
    int shown = 0, total = 0;
    for (int i = 0; i < GAME_TITLES; i++) total += k_game_titles[i].enabled;
    out[0] = 0;
    for (int i = 0; i < GAME_TITLES && k < n; i++) {
        if (!k_game_titles[i].enabled) continue;
        const wchar_t* sep = !shown ? L"" : shown == total - 1 ? L" or " : L", ";
        const int w = swprintf(out + k, n - k, L"%ls%hs", sep, k_game_titles[i].id);
        if (w < 0) break;
        k += (size_t)w;
        shown++;
    }
}

/* The title PARAM.SFO in a folder names when the table has it, else the default (game_find.cpp). */
const GameTitle* game_title_of_folder(const wchar_t* dir);

/* The same for the folder <vfs>\PS3_GAME leads to, vfs in UTF-8 or the ANSI code page. */
const GameTitle* game_title_of_vfs(const char* vfs);

#endif
