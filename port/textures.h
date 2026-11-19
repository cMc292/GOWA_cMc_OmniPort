/*
 * Texture mods: the pictures in the textures\ of the mods that are on (mods.h), and the dump for those who make
 * them, as the D3D12 backend's hooks (ps3recomp's libs/video/rsx_tex_replace.h).
 *
 * A picture is a PNG named by the id of the texture it stands for, sixteen hex digits, as the dump names the
 * textures it writes: <id>.png, or <id>_<anything>.png (a dumped file, renamed or not), is taken in the texture's
 * place, at any size; <id>.overlay.png (or <id>_<anything>.overlay.png) is drawn over the game's own texture,
 * scaled to the overlay's size first, where its alpha is above 1, and a texel of alpha 1 is made clear: a mod need
 * have nothing of the game's in it. The first mod in the list wins where two have the same id.
 */
#ifndef GOWA_TEXTURES_H
#define GOWA_TEXTURES_H

#include <string>

struct ModList;

/* The hooks, when a mod that is on has pictures, xbox_icons is set or dump_dir is not empty: the dump writes each
 * texture the game loads, once, as <id>_<w>x<h>_<format>.png there. xbox_icons: the port's own Xbox button icons
 * (app.rc) in place of the game's PlayStation ones, after any mod's. The number of textures given a picture. */
int gowa_textures_start(const ModList& l, const std::wstring& dump_dir, int xbox_icons);

/* The dump's folder: textures-dump beside the settings file (GOWA_TEXTURE_DUMP names another). */
std::wstring gowa_textures_dump_folder(void);

/* Button icons (input.button_icons): 1 for Xbox's, 0 for the game's own. "auto" is Xbox's when an XInput controller
 * is plugged in as the game starts. */
int gowa_textures_xbox_icons(void);

#endif
