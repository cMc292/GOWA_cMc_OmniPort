/*
 * The mods folder: a folder or a .zip for each mod, beside the settings ("mods", or [mods] folder). The launcher's
 * Mods tab lists them, switches each on or off and orders them, the first winning where two have the same file;
 * the game takes the files of the ones that are on (PS3_MODS_DIRS, vfs_mods.h). The game's own folder is never
 * written, and a mod is files alone: no code of its own runs.
 *
 * A mod's files are laid out as the game's USRDIR has them, in files\ (or USRDIR\, PS3_GAME\USRDIR\ or
 * dev_hdd0\game\<id>\USRDIR\, as mods for RPCS3 come), so one mod is the same for every release the port takes.
 * Its archive\ has files for the entries of the game's archives, by their names there, and its text\ lines of the
 * game's text: both go into one more update of the game, which the game starts by packing (mods_pack).
 * Its mod.ini, if it has one, names it:
 *
 *   [mod]
 *   name = Readable name
 *   version = 1.0
 *   author = Who made it
 *   description = A line about what it changes.
 *
 * A .zip is unpacked once into mods\.unpacked\, and again when it changes. In the settings, [mods] use switches
 * them all, order lists them as the tab has them and off the ones switched off ('|' between names, a name being
 * the folder's or the zip's as it is in the folder); a mod not in order yet goes last, and is on.
 */
#ifndef GOWA_MODS_H
#define GOWA_MODS_H

#include <string>
#include <vector>

struct ModInfo {
    std::wstring id;                /* its name in the folder: the folder's, or the zip's with .zip */
    std::wstring root;              /* where its files are: the folder, or the zip's unpacked copy ("" if none) */
    std::string name, version, author, desc;   /* mod.ini's, in UTF-8; name is the id without one */
    int zip = 0;                    /* it is a .zip */
    int on = 1;                     /* switched on */
    std::vector<std::wstring> usrdirs;   /* its folders that stand for the game's USRDIR */
    int files = 0;                  /* the files in them */
    int shadowed = 0;               /* of those, how many a mod above it also has (that one's are taken) */
    std::wstring shadowed_by;       /* the first such mod's id */
    std::wstring archive, text;     /* its archive\ and text\ folders, or "" (modpack.py packs them) */
    int archive_files = 0;          /* the files in archive\ */
    int text_files = 0;             /* the .tsv files in text\ */
    std::string packed;             /* what the pack made of them, as of the mods as they are now; "" not packed yet */
    int pack_warn = 0;              /* something of them was left out */
    std::wstring textures;          /* its textures\ folder, or "": pictures by texture id (textures.h) */
    int texture_files = 0;          /* the pictures in it (.png files named by an id) */
    std::vector<std::string> later; /* what this build does not read yet: "patches" */
    std::string error;              /* why none of it is read, or "" */
};

struct ModList {
    std::wstring dir;               /* the mods folder */
    int use = 1;                    /* [mods] use */
    std::vector<ModInfo> mods;      /* in order: the first one wins */
    std::wstring built;             /* the pack's files\ (TPPS<n+1>.PSARC, PATCHES) when it is the mods' as they are */
    std::string pack_error;         /* why the pack could not be made, or "" */
};

/* The mods folder: [mods] folder, else "mods" beside the settings file (else beside the exe). */
std::wstring mods_folder(void);

/* What the folder holds, in the settings' order. unpack: a zip that is new or has changed since it was unpacked is
 * unpacked now; else such a zip is listed with an error. */
void mods_scan(ModList* out, int unpack);

/* The list's order and the ones that are off, into the settings ([mods] order, off); ps3_settings_save writes them. */
void mods_store(const ModList& l);

/* The archive\ and text\ folders of the mods that are on, as one more update in mods\.built\files\ (modpack.py, run
 * with the Kit's Python, else the build tree's): made again only when they, the game's updates (usrdir: the folder of
 * its PATCHES and TPPS<n>.PSARC) or the update number before it have changed. Sets l->built, each mod's packed, and
 * l->pack_error when it could not be made. */
void mods_pack(ModList* l, const std::wstring& usrdir);

/* PS3_MODS_DIRS: the pack's folder first, then the USRDIR folders of the mods that are on, the first one's first, '|'
 * between them. "" when mods are off or none is on. */
std::string mods_dirs(const ModList& l);

/* One line for the log and the report: "Xbox icons 1.0 (12 files), Big text (3 files); off: Other". */
std::string mods_summary(const ModList& l);

#endif
