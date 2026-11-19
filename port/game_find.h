/*
 * Ascension-R -- finding the game on this PC (game_find.cpp)
 */
#ifndef GOWA_GAME_FIND_H
#define GOWA_GAME_FIND_H

#include <stddef.h>
#include <wchar.h>

enum { GAME_PATH_MAX = 1024 };

/* What a folder holds, worst to best. */
enum GameState {
    GAME_NONE,          /* no PARAM.SFO, or no GOWA.SELF or GOWA.elf under it (the game's data folder) */
    GAME_OTHER_TITLE,   /* another title, or another release of this one */
    GAME_INCOMPLETE,    /* this release, but some of the game's files are not in its folder, or not as 1.12 has them */
    GAME_NO_ELF,        /* this release, GOWA.SELF not decrypted to GOWA.elf yet */
    GAME_OTHER_BUILD,   /* this title, but another version, or another GOWA.elf */
    GAME_OK             /* what this port was lifted from */
};

/* What a disc lacks, for GAME_INCOMPLETE (game_files_text says it to the player). */
enum DiscLack {
    DISC_WHOLE,         /* nothing: the disc and its update are both there */
    DISC_NO_UPDATE,     /* the disc, but not its update to 1.12 (RPCS3 installs it in dev_hdd0\game\<id>) */
    DISC_NO_GAME,       /* the update, but not the disc it updates */
    DISC_ISO,           /* the disc only as an ISO (iso says where): it has to be extracted to a folder */
    DISC_ENCRYPTED,     /* the disc's files are encrypted: a dump that was never decrypted */
};

struct GameInfo {
    wchar_t dir[GAME_PATH_MAX];   /* the folder holding PARAM.SFO and USRDIR; for a disc, its PS3_GAME */
    wchar_t update[GAME_PATH_MAX];/* a disc's update, <RPCS3>\dev_hdd0\game\<id>; "" for a digital release */
    wchar_t iso[GAME_PATH_MAX];   /* DISC_ISO: the ISO RPCS3 has for the disc */
    char title[128], title_id[16], app_ver[16];   /* for a disc, APP_VER is its update's */
    char category[4];             /* PARAM.SFO's: HG a digital release, DG a disc, GD a disc's update */
    GameState state;
    int other_version;            /* GAME_OTHER_BUILD: PARAM.SFO says another version than 1.12 */
    int disc;                     /* a disc: dir its PS3_GAME, update its update (game_title.h's GAME_KIND_DISC) */
    DiscLack disc_lack;           /* GAME_INCOMPLETE of a disc: what it lacks */
    /* The game's files (its title's list), looked at when the folder is this release: how many there are, how many of
     * them are not in the folder and their bytes, how many are there but not as 1.12 has them, and the first of each.
     * base_bad: how many of those two are the base game's; none, and only the updates' files are at fault. */
    int files, files_missing, files_wrong, base_bad;
    unsigned long long bytes_missing;
    char first_missing[64], first_wrong[64];
};

/* What GAME_INCOMPLETE is about, and what to install again, in a line for the player. */
void game_files_text(const GameInfo* g, wchar_t* out, size_t n);

/* Whether a file of the game's list ("USRDIR/TPPS1.PSARC", either slash, any case) comes with one of the game's
 * updates, 1.04 to 1.12: the TPPS archives and PATCHES, which names TPPS1 to TPPS5. Each update brings its own files
 * and 1.12's is the EBOOT alone, so they come with the updates before it. */
int game_file_update(const char* path);

/* Whether a file of a title's folder ("USRDIR/GAME.PSARC", either slash, any case) is one the game cannot start
 * without (its list in game_find.cpp; a disc's has its disc's files and its update's). */
int game_file_needed(const char* title_id, const char* path);

/* Whether a file is the GOWA.elf this port was made from. */
int game_elf_ok(const wchar_t* path);

/* How far along a copy is, to choose between two: the one to play (GAME_OK), one of this version still to decrypt
 * (GAME_NO_ELF), one of this version with another GOWA.elf, another version, one that lacks files, another release.
 * Higher is better. The states' own order puts another version above GAME_NO_ELF: a search for "this release, whole"
 * stopped at a copy at 1.11 and never came to the one at 1.12. */
int game_rank(const GameInfo* g);

/* A file's SHA-256 in lower-case hex. 1 = read. */
int game_file_sha256(const wchar_t* path, char hex[65]);

/* What a folder given as it is holds. A disc's PS3_GAME is paired with its update, and an update with its disc,
 * from the RPCS3 folders known (game_note_rpcs3, its games.yml and dev_hdd0) and the copies a search saw. */
GameState game_probe(const wchar_t* dir, GameInfo* g);

/* The same with the update given (the launcher's link to it); NULL or "": looked for as game_probe does. */
GameState game_probe_pair(const wchar_t* dir, const wchar_t* update, GameInfo* g);

/* An RPCS3 folder (the one holding rpcs3.exe) to look in for a disc's update (dev_hdd0\game\<id>) and for where it
 * keeps a disc (config\games.yml). The usual places RPCS3 keeps its files are looked in anyway. */
void game_note_rpcs3(const wchar_t* root);

/* Only the title's id and version, from the folder's PARAM.SFO; no hash of the game's code. 1 = read. */
int game_read_info(const wchar_t* dir, GameInfo* g);

/* The same for what a player would pick: the game's folder, the one holding PS3_GAME, an RPCS3
 * folder, its dev_hdd0, its game folder, or USRDIR. The best of them; dir says which. */
GameState game_locate(const wchar_t* path, GameInfo* g);

struct GameSearch {
    const wchar_t* under;                    /* the folder to look under; NULL, every drive */
    volatile long* stop;                     /* set by another thread: ends the search */
    void (*now)(const wchar_t* dir);         /* the folder being read; may be NULL */
    int seconds;                             /* how long it may take */
    int copies;                              /* out: how many copies of this title it saw */
    int good_enough;                         /* the state that ends the search; 0, GAME_OK */
};

/* Walks the folders, the ones that sound like games first, and stops at the first copy that is
 * GAME_OK. The best one seen goes in *best. Only reads. */
GameState game_search(GameSearch* s, GameInfo* best);

/* The folder a link points at, kept even if that is gone. 0: path is no link. */
int game_link_target(const wchar_t* link, wchar_t* out, size_t n);

/* Whether game_link could put a link to target at link: nothing there, a link, an empty folder, or
 * target itself. A folder with files in it is the player's and stays. */
int game_link_possible(const wchar_t* link, const wchar_t* target, wchar_t* err, size_t n);

/* Makes link a directory junction to target (no administrator rights needed), as `gowa.py setup`
 * does. A link already there is replaced; what it pointed at is not touched. */
int game_link(const wchar_t* link, const wchar_t* target, wchar_t* err, size_t n);

#endif
