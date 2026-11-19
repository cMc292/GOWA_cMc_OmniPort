/*
 * Ascension-R -- the game's trophies (trophies.cpp): the set the runtime reads from the game's own TROPHY.TRP
 * (ps3recomp/libs/network/trophy_set.h) and the unlocks it keeps beside the saves.
 */
#pragma once

#include "ps3recomp/libs/network/trophy_set.h"

/* Before the game starts: a notice over the picture for each trophy the game unlocks, and the multiplayer trophy
 * left out of the platinum. GOWA_TROPHY_DEMO=0,5,35 shows those trophies' notices at the start instead, unlocking
 * nothing: to look at them. */
extern "C" void gowa_trophies_start(void);

/* The set and the player's unlocks, as the launcher shows them. */
struct GowaTrophies {
    PS3TrophySet set;
    unsigned char unlocked[PS3_TROPHY_SET_MAX];
    unsigned char unobtainable[PS3_TROPHY_SET_MAX];   /* not to be had in this port: the multiplayer one */
    long long when[PS3_TROPHY_SET_MAX];               /* Unix seconds, 0 if not known */
    char store[1024];                                 /* the file of the unlocks, UTF-8 */
};

/* The game's set, from app_dir's TROPDIR (the game's update, or the digital game's own folder) or else disc_dir's
 * (the disc's PS3_GAME), its names in PS3 language lang; and the unlocks kept for it under hdd0. Paths in UTF-8.
 * 0 if the set was found and read; -1 with t->set.count 0 if not. */
int gowa_trophies_read(const char* hdd0, const char* app_dir, const char* disc_dir, int lang, GowaTrophies* t);

/* Trophy id's picture from the TROPHY.TRP at trp (a set's path): w x h straight-alpha RGBA, to free with
 * gowa_trophies_free; NULL if it has none. */
unsigned char* gowa_trophies_icon(const char* trp, int id, int* w, int* h);
void gowa_trophies_free(unsigned char* rgba);
