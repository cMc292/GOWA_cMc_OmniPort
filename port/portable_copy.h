/*
 * Ascension-R -- a portable folder's first start takes the files of %LOCALAPPDATA%\Ascension-R along (portable_copy.cpp)
 */
#ifndef GOWA_PORTABLE_COPY_H
#define GOWA_PORTABLE_COPY_H

#include <stddef.h>
#include <wchar.h>

/* The files of folder from copied into folder to, which has none of them yet: everything but the logs and the game
 * code build's work folder; a link to a folder (the game's folders) made again to the same place; the settings' paths
 * that named from moved to to. from is only read. The number of things that could not be copied or linked, and in
 * note what was done, for the log. */
int portable_copy_from(const wchar_t* from, const wchar_t* to, char* note, size_t n);

/* On the first start of a folder portable.txt marks, with none of the program's files in it yet: if
 * %LOCALAPPDATA%\Ascension-R has a settings file, the player is asked whether to copy its files here, and they are.
 * settings is the settings file the program is about to read. note, for the log, says what happened ("" for nothing). */
void portable_offer(const char* settings, char* note, size_t n);

#endif
