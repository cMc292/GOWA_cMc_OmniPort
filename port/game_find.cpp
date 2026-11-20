/*
 * Ascension-R -- finding the game on this PC
 *
 * The game's folder reaches the port through two directory junctions, <vfs>\PS3_GAME and
 * <hdd0>\game\<id> (NPUA80918, the titles in game_title.h), as `gowa.py setup` makes them. The launcher reads the
 * first to show where the game is, and sets both when the player picks another folder or Auto detect finds one.
 *
 * What a folder holds comes from its PARAM.SFO, the game's files and its GOWA.elf: the port is lifted
 * from one release, so another version or another GOWA.elf is told apart (scripts/gowa.py has the same
 * three values, for `doctor`), and so is a folder that lacks some of that release's files
 * (game_files.inc, and game_files_<id>.inc for a disc). The search only reads. No game file is copied or changed.
 */
#include "game_find.h"
#include "game_title.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#include <bcrypt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <deque>
#include <set>
#include <string>
#include <utility>
#include <vector>
#pragma comment(lib, "bcrypt.lib")

static const char k_app_ver[] = "01.12";
static const char k_elf_sha256[] = "4bf75c897b82d1beb25514cbadaff3d03e1b0c7bf849fa5d289be4f9183462b5";

/* Whether a folder's name is the id of a title the launcher takes. */
static int is_title_dir(const wchar_t* name)
{
    for (int i = 0; i < GAME_TITLES; i++) {
        if (!k_game_titles[i].enabled) continue;
        wchar_t w[16];
        swprintf(w, sizeof w / sizeof w[0], L"%hs", k_game_titles[i].id);
        if (!_wcsicmp(name, w)) return 1;
    }
    return 0;
}

/* The game's files: USRDIR's archives and the three lists that name them, each with its size and the SHA-256 of its
 * first and last 64 KiB (made by `gowa.py game-files` from a whole copy of 1.12), a list for each title. The game
 * opens them at its start; without one it stopped a few seconds in, with a black window and nothing said. With 1.12's
 * EBOOT on the base game's TPPS.PSARC and no PATCHES it waited on its first loading screen (issue #2). A folder that
 * lacks any, or has one of another size or other ends, is not the game: nothing is decrypted, built or played from
 * it. A disc's list is its disc's files and its update's (game_file_update), each looked for in its own folder.
 * tests/test_game_find.py builds this file with lists of its own (GAME_FILES_INC, GAME_FILES_BCES01741_INC). */
struct GameFile { const char* path; unsigned long long size; const char* ends; };
static const GameFile k_files_npua80918[] = {
#ifdef GAME_FILES_INC
#include GAME_FILES_INC
#else
#include "game_files.inc"
#endif
};
static const GameFile k_files_bces01741[] = {
#ifdef GAME_FILES_BCES01741_INC
#include GAME_FILES_BCES01741_INC
#else
#include "game_files_bces01741.inc"
#endif
};
struct TitleFiles { const char* id; const GameFile* files; int n; };
#define TITLE_FILES(id, list) { id, list, (int)(sizeof list / sizeof list[0]) }
static const TitleFiles k_title_files[] = {
    TITLE_FILES("NPUA80918", k_files_npua80918),
    TITLE_FILES("BCES01741", k_files_bces01741),
};
#undef TITLE_FILES

/* A title's list, or NULL for one with none (its files are not looked at). */
static const TitleFiles* title_files(const char* id)
{
    for (const TitleFiles& t : k_title_files)
        if (id && !strcmp(t.id, id)) return &t;
    return NULL;
}
static const DWORD k_ends = 64 * 1024;

static uint32_t rd16(const unsigned char* p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t rd32(const unsigned char* p) { return rd16(p) | rd16(p + 2) << 16; }

static void copy_value(char* out, size_t n, const unsigned char* v, uint32_t len)
{
    size_t k = len < n - 1 ? len : n - 1;
    memcpy(out, v, k);
    out[k] = 0;
}

/* TITLE_ID, APP_VER and TITLE out of a PARAM.SFO. Everything is checked against the file's size: a
 * search reads whatever is called PARAM.SFO. */
static int sfo_read(const wchar_t* path, GameInfo* g)
{
    static const size_t CAP = 65536;
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    unsigned char* d = (unsigned char*)malloc(CAP);
    DWORD n = 0;
    const BOOL ok = d && ReadFile(h, d, (DWORD)CAP, &n, NULL);
    CloseHandle(h);
    int found = 0;
    if (ok && n >= 20 && !memcmp(d, "\0PSF", 4)) {
        const uint32_t keys = rd32(d + 8), data = rd32(d + 12), count = rd32(d + 16);
        if (keys < n && data < n && count <= 256 && 20 + count * 16 <= n) {
            for (uint32_t i = 0; i < count; i++) {
                const unsigned char* e = d + 20 + i * 16;
                const uint32_t ko = keys + rd16(e), len = rd32(e + 4), vo = data + rd32(e + 12);
                if (ko >= n || vo >= n || len > n - vo || !memchr(d + ko, 0, n - ko)) continue;
                const char* key = (const char*)d + ko;
                if (!strcmp(key, "TITLE_ID")) { copy_value(g->title_id, sizeof g->title_id, d + vo, len); found = 1; }
                else if (!strcmp(key, "APP_VER")) copy_value(g->app_ver, sizeof g->app_ver, d + vo, len);
                else if (!strcmp(key, "CATEGORY")) copy_value(g->category, sizeof g->category, d + vo, len);
                else if (!strcmp(key, "TITLE")) copy_value(g->title, sizeof g->title, d + vo, len);
            }
        }
    }
    free(d);
    return found;
}

int game_file_sha256(const wchar_t* path, char hex[65])
{
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                           FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (f == INVALID_HANDLE_VALUE) return 0;
    BCRYPT_ALG_HANDLE alg = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    int done = 0;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0 &&
        BCryptCreateHash(alg, &hash, NULL, 0, NULL, 0, 0) == 0) {
        static const DWORD CHUNK = 1u << 20;
        unsigned char* b = (unsigned char*)malloc(CHUNK);
        DWORD n = 0;
        int ok = b != NULL;
        while (ok && ReadFile(f, b, CHUNK, &n, NULL) && n) ok = BCryptHashData(hash, b, n, 0) == 0;
        free(b);
        unsigned char sum[32];
        if (ok && BCryptFinishHash(hash, sum, sizeof sum, 0) == 0) {
            for (int i = 0; i < 32; i++) snprintf(hex + i * 2, 3, "%02x", sum[i]);
            done = 1;
        }
    }
    if (hash) BCryptDestroyHash(hash);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    CloseHandle(f);
    return done;
}

int game_elf_ok(const wchar_t* path)
{
    char hex[65];
    return game_file_sha256(path, hex) && !strcmp(hex, k_elf_sha256);
}

static int is_file(const wchar_t* path)
{
    const DWORD a = GetFileAttributesW(path);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

/* A folder's path without the backslash at its end, a drive's root keeping it. */
static void folder_copy(wchar_t* out, size_t n, const wchar_t* in)
{
    swprintf(out, n, L"%ls", in);
    size_t k = wcslen(out);
    while (k > 3 && (out[k - 1] == L'\\' || out[k - 1] == L'/')) out[--k] = 0;
}

/* --- the game's files ----------------------------------------------------------- */
static int sha256_hex(const unsigned char* b, size_t n, char hex[65])
{
    BCRYPT_ALG_HANDLE alg = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    unsigned char sum[32];
    const int done = BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0 &&
                     BCryptCreateHash(alg, &hash, NULL, 0, NULL, 0, 0) == 0 &&
                     BCryptHashData(hash, (PUCHAR)b, (ULONG)n, 0) == 0 &&
                     BCryptFinishHash(hash, sum, sizeof sum, 0) == 0;
    if (done)
        for (int i = 0; i < 32; i++) snprintf(hex + i * 2, 3, "%02x", sum[i]);
    if (hash) BCryptDestroyHash(hash);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    return done;
}

/* The SHA-256 of a file's first and last 64 KiB, one after the other: the whole file twice when it is smaller
 * (gowa.py's ends_sha256). Reading 128 KiB of a file is what tells a copy of it from another file of its size. */
static int ends_sha256(const wchar_t* path, unsigned long long size, char hex[65])
{
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return 0;
    const DWORD part = size < k_ends ? (DWORD)size : k_ends;
    unsigned char* b = (unsigned char*)malloc(2 * (size_t)part + 1);
    LARGE_INTEGER at;
    at.QuadPart = (LONGLONG)(size - part);
    DWORD head = 0, tail = 0;
    int ok = b && ReadFile(f, b, part, &head, NULL) && head == part && SetFilePointerEx(f, at, NULL, FILE_BEGIN) &&
             ReadFile(f, b + part, part, &tail, NULL) && tail == part;
    CloseHandle(f);
    ok = ok && sha256_hex(b, 2 * (size_t)part, hex);
    free(b);
    return ok;
}

/* A file of the game's folder by its path in its list. */
static void file_path(wchar_t* out, size_t n, const wchar_t* dir, const char* rel)
{
    swprintf(out, n, L"%ls\\%hs", dir, rel);
    for (wchar_t* c = out; *c; c++)
        if (*c == L'/') *c = L'\\';
}

static void first_of(char* out, size_t n, const char* path)
{
    if (!out[0]) snprintf(out, n, "%s", path);
}

/* Whether dir has the game's files as 1.12 has them, and what it lacks in g. Their sizes first, and the ends only when
 * every one is there at its size: a folder that lacks some is told so without a byte of it read. A disc's update
 * (update) holds the files that come with the updates (game_file_update); a title with no list passes. */
static int files_check(const wchar_t* dir, const wchar_t* update, GameInfo* g)
{
    const TitleFiles* t = title_files(g->title_id);
    if (!t) return 1;
    g->files = t->n;
    wchar_t p[GAME_PATH_MAX + 96];
    for (int i = 0; i < t->n; i++) {
        const GameFile& f = t->files[i];
        file_path(p, sizeof p / sizeof p[0], update && game_file_update(f.path) ? update : dir, f.path);
        WIN32_FILE_ATTRIBUTE_DATA a;
        if (!GetFileAttributesExW(p, GetFileExInfoStandard, &a) || (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            g->files_missing++;
            g->bytes_missing += f.size;
            g->base_bad += !game_file_update(f.path);
            first_of(g->first_missing, sizeof g->first_missing, f.path);
        } else if (((unsigned long long)a.nFileSizeHigh << 32 | a.nFileSizeLow) != f.size) {
            g->files_wrong++;
            g->base_bad += !game_file_update(f.path);
            first_of(g->first_wrong, sizeof g->first_wrong, f.path);
        }
    }
    if (g->files_missing || g->files_wrong) return 0;
    for (int i = 0; i < t->n; i++) {
        const GameFile& f = t->files[i];
        file_path(p, sizeof p / sizeof p[0], update && game_file_update(f.path) ? update : dir, f.path);
        char hex[65];
        if (!ends_sha256(p, f.size, hex) || strcmp(hex, f.ends)) {
            g->files_wrong++;
            g->base_bad += !game_file_update(f.path);
            first_of(g->first_wrong, sizeof g->first_wrong, f.path);
        }
    }
    return !g->files_wrong;
}

/* A file's name without USRDIR/, as a player would look for it. */
static const char* name_shown(const char* path)
{
    const char* s = strrchr(path, '/');
    return s ? s + 1 : path;
}

static void bytes_text(unsigned long long b, wchar_t* out, size_t n)
{
    if (b >= 1000000000ull) swprintf(out, n, L"%.1f GB", (double)b / 1e9);
    else if (b >= 1000000ull) swprintf(out, n, L"%.0f MB", (double)b / 1e6);
    else swprintf(out, n, L"%llu bytes", b);
}

/* What to install: the updates 1.04 to 1.12, in that order, when only their files are at fault, else the game again.
 * Each brings its own files and 1.12's is the EBOOT alone: installing it again brings no archive (issue #2). A line of
 * the System tab holds about a hundred characters. */
void game_files_text(const GameInfo* g, wchar_t* out, size_t n)
{
    switch (g->disc ? g->disc_lack : DISC_WHOLE) {
    case DISC_NO_UPDATE:
        swprintf(out, n, L"The disc (%hs) without its update to 1.12: install the updates 1.04 to 1.12 in RPCS3 "
                         L"(File, Install Packages, in that order), then Auto detect", g->title_id);
        return;
    case DISC_NO_GAME:
        swprintf(out, n, L"The update of %hs is in %ls, but not its disc: Browse to the disc's folder (the one holding "
                         L"PS3_GAME)", g->title_id, g->update);
        return;
    case DISC_ISO:
        swprintf(out, n, L"RPCS3 has the disc as an ISO, %ls: open it (Mount), copy all of it into a folder, and Browse "
                         L"to that folder", g->iso);
        return;
    case DISC_ENCRYPTED:
        swprintf(out, n, L"The disc's files in %ls are encrypted: a dump never decrypted. A decrypted copy is needed",
                 g->dir);
        return;
    default:
        break;
    }
    const int one = (g->files_missing ? g->files_missing : g->files_wrong) == 1;
    if (!g->base_bad && g->files_missing) {
        swprintf(out, n, L"Not every update is installed: %d %ls missing, %hs first. Install 1.04 to 1.12, in order",
                 g->files_missing, one ? L"file" : L"files", name_shown(g->first_missing));
    } else if (!g->base_bad) {
        swprintf(out, n, L"Not every update is installed: %d %ls, %hs first. Install 1.04 to 1.12, in order",
                 g->files_wrong, one ? L"file differs" : L"files differ", name_shown(g->first_wrong));
    } else if (g->files_missing && g->disc) {
        wchar_t size[32];
        bytes_text(g->bytes_missing, size, sizeof size / sizeof size[0]);
        swprintf(out, n, L"Not the whole disc: %d of the game's %d files missing (%ls), %hs first. Copy all of the "
                         L"disc again", g->files_missing, g->files, size, name_shown(g->first_missing));
    } else if (g->files_missing) {
        wchar_t size[32];
        bytes_text(g->bytes_missing, size, sizeof size / sizeof size[0]);
        swprintf(out, n, L"Not the whole game: %d of its %d files missing (%ls), %hs first. Install the game again",
                 g->files_missing, g->files, size, name_shown(g->first_missing));
    } else if (g->disc) {
        swprintf(out, n, L"%d of the disc's files %ls not as the disc has %ls, %hs first. Copy all of the disc again",
                 g->files_wrong, one ? L"is" : L"are", one ? L"it" : L"them", name_shown(g->first_wrong));
    } else {
        swprintf(out, n, L"%d of the game's files %ls not as version 1.12 has %ls, %hs first. Install the game again",
                 g->files_wrong, one ? L"is" : L"are", one ? L"it" : L"them", name_shown(g->first_wrong));
    }
}

static int same_char(char a, char b)
{
    if (a == '\\') a = '/';
    if (b == '\\') b = '/';
    if (a >= 'a' && a <= 'z') a = (char)(a - 'a' + 'A');
    if (b >= 'a' && b <= 'z') b = (char)(b - 'a' + 'A');
    return a == b;
}

/* Whether path starts with name, or with whole set is name: either slash, any case. */
static int name_is(const char* path, const char* name, int whole)
{
    while (*name && *path && same_char(*path, *name)) { path++; name++; }
    return !*name && (!whole || !*path);
}

int game_file_needed(const char* title_id, const char* path)
{
    const TitleFiles* t = title_files(title_id);
    for (int i = 0; t && i < t->n; i++)
        if (name_is(path, t->files[i].path, 1)) return 1;
    return 0;
}

int game_file_update(const char* path)
{
    return name_is(path, "USRDIR/TPPS", 0) || name_is(path, "USRDIR/PATCHES", 1);
}

/* --- discs ---------------------------------------------------------------------- */
/* A disc runs from two folders: the disc's PS3_GAME (the base game, CATEGORY DG) and its update to 1.12, which RPCS3
 * installs in <RPCS3>\dev_hdd0\game\<id> (CATEGORY GD, with GOWA.SELF and the TPPS archives). Each is found from the
 * other: the update's RPCS3 names the disc's folder in its games.yml, and a disc's update is in the dev_hdd0 of an
 * RPCS3 known (game_note_rpcs3, the usual places, one beside the disc's folder) or one a search saw
 * (docs/notes/2026-11-17-eu-support-plan.md, phase 3). */

static SRWLOCK s_seen_lock = SRWLOCK_INIT;
static std::vector<std::wstring> s_rpcs3;                                     /* RPCS3 folders noted */
static std::vector<std::pair<std::string, std::wstring>> s_discs, s_updates;   /* what probes saw: id, folder */

static void seen_add(std::vector<std::pair<std::string, std::wstring>>& v, const char* id, const wchar_t* dir)
{
    AcquireSRWLockExclusive(&s_seen_lock);
    int there = 0;
    for (const auto& x : v) there |= x.first == id && !_wcsicmp(x.second.c_str(), dir);
    if (!there) v.push_back({ id, dir });
    ReleaseSRWLockExclusive(&s_seen_lock);
}

static std::vector<std::wstring> seen_of(const std::vector<std::pair<std::string, std::wstring>>& v, const char* id)
{
    std::vector<std::wstring> out;
    AcquireSRWLockShared(&s_seen_lock);
    for (const auto& x : v)
        if (x.first == id) out.push_back(x.second);
    ReleaseSRWLockShared(&s_seen_lock);
    return out;
}

static int has_rpcs3(const wchar_t* dir)
{
    wchar_t p[GAME_PATH_MAX + 32];
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\rpcs3.exe", dir);
    return is_file(p);
}

void game_note_rpcs3(const wchar_t* root)
{
    if (!root || !*root) return;
    wchar_t r[GAME_PATH_MAX];
    folder_copy(r, GAME_PATH_MAX, root);
    AcquireSRWLockExclusive(&s_seen_lock);
    int there = 0;
    for (const std::wstring& x : s_rpcs3) there |= !_wcsicmp(x.c_str(), r);
    if (!there) s_rpcs3.push_back(r);
    ReleaseSRWLockExclusive(&s_seen_lock);
}

/* The RPCS3 folders to look in: those noted, the usual places, and the ones beside a folder: an RPCS3 in it, or in a
 * folder next to it or next to one of the three above it (E:\PS3\Games\<disc> finds E:\PS3\rpcs3). */
static std::vector<std::wstring> rpcs3_roots(const wchar_t* beside)
{
    std::vector<std::wstring> out;
    auto add = [&out](const std::wstring& r) {
        for (const std::wstring& x : out)
            if (!_wcsicmp(x.c_str(), r.c_str())) return;
        out.push_back(r);
    };
    AcquireSRWLockShared(&s_seen_lock);
    const std::vector<std::wstring> noted = s_rpcs3;
    ReleaseSRWLockShared(&s_seen_lock);
    for (const std::wstring& r : noted) add(r);
    static const wchar_t* const vars[] = { L"LOCALAPPDATA", L"APPDATA", L"USERPROFILE" };
    for (const wchar_t* v : vars) {
        wchar_t e[GAME_PATH_MAX], p[GAME_PATH_MAX + 16];
        if (!GetEnvironmentVariableW(v, e, GAME_PATH_MAX)) continue;
        swprintf(p, sizeof p / sizeof p[0], L"%ls\\rpcs3", e);
        if (has_rpcs3(p)) add(p);
    }
    if (beside && *beside) {
        std::wstring up(beside);
        for (int level = 0; level < 4; level++) {
            const size_t s = up.find_last_of(L'\\');
            if (s == std::wstring::npos || s < 2) break;
            up.resize(s);
            if (has_rpcs3(up.c_str())) add(up);
            WIN32_FIND_DATAW fd;
            HANDLE h = FindFirstFileExW((up + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchLimitToDirectories, NULL, 0);
            if (h == INVALID_HANDLE_VALUE) continue;
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
                const std::wstring c = up + L"\\" + fd.cFileName;
                if (has_rpcs3(c.c_str())) add(c);
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        }
    }
    return out;
}

/* Where RPCS3's games.yml (config\games.yml; games.yml in older versions) says a title's disc is, as a folder of this PC
 * without the closing backslash. 1 = it names one. */
static int games_yml_dir(const wchar_t* root, const char* id, wchar_t* out, size_t n)
{
    static const wchar_t* const where[] = { L"\\config\\games.yml", L"\\games.yml" };
    const size_t idn = strlen(id);
    for (const wchar_t* w : where) {
        wchar_t p[GAME_PATH_MAX + 32];
        swprintf(p, sizeof p / sizeof p[0], L"%ls%ls", root, w);
        HANDLE h = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) continue;
        std::string d(1 << 20, '\0');
        DWORD got = 0;
        const BOOL ok = ReadFile(h, &d[0], (DWORD)d.size(), &got, NULL);
        CloseHandle(h);
        if (!ok) continue;
        d.resize(got);
        size_t pos = 0;
        while (pos < d.size()) {
            size_t end = d.find('\n', pos);
            if (end == std::string::npos) end = d.size();
            std::string line = d.substr(pos, end - pos);
            pos = end + 1;
            size_t a = line.find_first_not_of(" \t");
            if (a == std::string::npos || line.compare(a, idn, id) || line.size() <= a + idn || line[a + idn] != ':')
                continue;
            std::string v = line.substr(a + idn + 1);
            while (!v.empty() && (v.back() == '\r' || v.back() == ' ' || v.back() == '\t')) v.pop_back();
            const size_t b = v.find_first_not_of(" \t");
            v = b == std::string::npos ? "" : v.substr(b);
            if (v.size() >= 2 && (v[0] == '"' || v[0] == '\'') && v.back() == v[0]) v = v.substr(1, v.size() - 2);
            if (v.empty()) continue;
            if (!MultiByteToWideChar(CP_UTF8, 0, v.c_str(), -1, out, (int)n)) continue;
            for (wchar_t* c = out; *c; c++)
                if (*c == L'/') *c = L'\\';
            folder_copy(out, n, std::wstring(out).c_str());
            return 1;
        }
    }
    return 0;
}

/* Whether a folder is that title's disc (PS3_GAME, CATEGORY DG) or its update (CATEGORY GD, and the game's executable
 * in USRDIR: a GD folder without it is the game's data, <id>-DATA). */
static int is_disc_of(const wchar_t* dir, const char* id)
{
    GameInfo t;
    return game_read_info(dir, &t) && !strcmp(t.title_id, id) && !strcmp(t.category, "DG");
}

static int is_update_of(const wchar_t* dir, const char* id)
{
    GameInfo t;
    if (!game_read_info(dir, &t) || strcmp(t.title_id, id) || strcmp(t.category, "GD")) return 0;
    wchar_t p[GAME_PATH_MAX + 32];
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\USRDIR\\GOWA.SELF", dir);
    if (is_file(p)) return 1;
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\USRDIR\\GOWA.elf", dir);
    return is_file(p);
}

/* Whether a disc's archives are encrypted: GAME.PSARC, which every release has, does not start with PSAR, as the files
 * of a dump that was never decrypted do. */
static int disc_encrypted(const wchar_t* disc)
{
    wchar_t p[GAME_PATH_MAX + 32];
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\USRDIR\\GAME.PSARC", disc);
    HANDLE h = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    char b[4] = { 0 };
    DWORD got = 0;
    const BOOL ok = ReadFile(h, b, 4, &got, NULL);
    CloseHandle(h);
    return ok && got == 4 && memcmp(b, "PSAR", 4) != 0;
}

/* The disc's update: the one given, or in an RPCS3 known or beside the disc, or one a search saw. Of several (one
 * RPCS3 with an older update), the first at the port's version, else the first. */
static int find_update(const char* id, const wchar_t* disc, const wchar_t* given, wchar_t* out, size_t n)
{
    if (given && *given && is_update_of(given, id)) { folder_copy(out, n, given); return 1; }
    std::vector<std::wstring> found;
    /* A disc copied outside RPCS3: its update is usually Update\<id> beside the disc folder. */
    {
        std::wstring base(disc);
        for (int up = 0; up < 4; up++) {
            const size_t slash = base.find_last_of(L'\\');
            if (slash == std::wstring::npos) break;
            base.resize(slash);
            wchar_t cand[GAME_PATH_MAX + 64];
            swprintf(cand, sizeof cand / sizeof cand[0], L"%ls\\Update\\%hs", base.c_str(), id);
            if (is_update_of(cand, id)) found.push_back(cand);
            swprintf(cand, sizeof cand / sizeof cand[0], L"%ls\\%hs", base.c_str(), id);
            if (is_update_of(cand, id)) found.push_back(cand);
        }
    }
    for (const std::wstring& r : rpcs3_roots(disc)) {
        wchar_t p[GAME_PATH_MAX + 64];
        swprintf(p, sizeof p / sizeof p[0], L"%ls\\dev_hdd0\\game\\%hs", r.c_str(), id);
        if (is_update_of(p, id)) found.push_back(p);
    }
    for (const std::wstring& u : seen_of(s_updates, id))
        if (is_update_of(u.c_str(), id)) found.push_back(u);
    if (found.empty()) return 0;
    for (const std::wstring& u : found) {
        GameInfo g;
        if (game_read_info(u.c_str(), &g) && !strcmp(g.app_ver, k_app_ver)) { folder_copy(out, n, u.c_str()); return 1; }
    }
    folder_copy(out, n, found[0].c_str());
    return 1;
}

/* The update's disc: from its RPCS3's games.yml (or another RPCS3's), or one a search saw. An ISO there goes in iso. */
static int find_disc(const char* id, const wchar_t* update, wchar_t* out, size_t n, wchar_t* iso, size_t isn)
{
    std::vector<std::wstring> roots;
    std::wstring up(update);
    for (int i = 0; i < 3; i++) {
        const size_t s = up.find_last_of(L'\\');
        if (s == std::wstring::npos) break;
        up.resize(s);
    }
    roots.push_back(up);   /* <RPCS3>\dev_hdd0\game\<id>: three folders up */
    for (const std::wstring& r : rpcs3_roots(NULL)) roots.push_back(r);
    for (const std::wstring& r : roots) {
        wchar_t v[GAME_PATH_MAX];
        if (!games_yml_dir(r.c_str(), id, v, GAME_PATH_MAX)) continue;
        const size_t k = wcslen(v);
        if (k > 4 && !_wcsicmp(v + k - 4, L".iso")) {
            if (is_file(v)) swprintf(iso, isn, L"%ls", v);
            continue;
        }
        wchar_t p[GAME_PATH_MAX + 16];
        swprintf(p, sizeof p / sizeof p[0], L"%ls\\PS3_GAME", v);
        if (is_disc_of(p, id)) { folder_copy(out, n, p); return 1; }
        if (is_disc_of(v, id)) { folder_copy(out, n, v); return 1; }
    }
    for (const std::wstring& d : seen_of(s_discs, id))
        if (is_disc_of(d.c_str(), id)) { folder_copy(out, n, d.c_str()); return 1; }
    return 0;
}

/* A disc title's folder, the disc's PS3_GAME or its update, with the other found. */
static GameState disc_probe(const wchar_t* update_given, GameInfo* g)
{
    const char* id = g->title_id;
    g->disc = 1;
    wchar_t disc[GAME_PATH_MAX] = L"", upd[GAME_PATH_MAX] = L"";
    if (!strcmp(g->category, "DG")) {
        folder_copy(disc, GAME_PATH_MAX, g->dir);
        seen_add(s_discs, id, disc);
        find_update(id, disc, update_given, upd, GAME_PATH_MAX);
    } else if (!strcmp(g->category, "GD")) {
        if (!is_update_of(g->dir, id)) return g->state = GAME_NONE;   /* the game's data, not its update */
        folder_copy(upd, GAME_PATH_MAX, g->dir);
        seen_add(s_updates, id, upd);
        std::wstring root(upd);   /* <RPCS3>\dev_hdd0\game\<id>: three folders up */
        for (int i = 0; i < 3; i++) {
            const size_t s = root.find_last_of(L'\\');
            if (s != std::wstring::npos) root.resize(s);
        }
        if (has_rpcs3(root.c_str())) game_note_rpcs3(root.c_str());
        find_disc(id, upd, disc, GAME_PATH_MAX, g->iso, GAME_PATH_MAX);
    } else {
        return g->state = GAME_OTHER_TITLE;
    }
    swprintf(g->update, GAME_PATH_MAX, L"%ls", upd);
    if (!disc[0]) {   /* the update alone: dir stays the update's, for the line that says so */
        g->disc_lack = g->iso[0] ? DISC_ISO : DISC_NO_GAME;
        return g->state = GAME_INCOMPLETE;
    }
    swprintf(g->dir, GAME_PATH_MAX, L"%ls", disc);
    if (disc_encrypted(disc)) { g->disc_lack = DISC_ENCRYPTED; return g->state = GAME_INCOMPLETE; }
    if (!upd[0]) { g->disc_lack = DISC_NO_UPDATE; return g->state = GAME_INCOMPLETE; }
    GameInfo u;
    if (game_read_info(upd, &u)) snprintf(g->app_ver, sizeof g->app_ver, "%s", u.app_ver);
    if (strcmp(g->app_ver, k_app_ver)) {
        g->other_version = 1;
        return g->state = GAME_OTHER_BUILD;
    }
    if (!files_check(g->dir, upd, g)) return g->state = GAME_INCOMPLETE;
    wchar_t elf[GAME_PATH_MAX + 32];
    swprintf(elf, sizeof elf / sizeof elf[0], L"%ls\\USRDIR\\GOWA.elf", upd);
    if (!is_file(elf)) return g->state = GAME_NO_ELF;
    return g->state = game_elf_ok(elf) ? GAME_OK : GAME_OTHER_BUILD;
}

GameState game_probe(const wchar_t* dir, GameInfo* g)
{
    return game_probe_pair(dir, NULL, g);
}

GameState game_probe_pair(const wchar_t* dir, const wchar_t* update, GameInfo* g)
{
    memset(g, 0, sizeof *g);
    folder_copy(g->dir, GAME_PATH_MAX, dir);
    wchar_t p[GAME_PATH_MAX + 32], elf[GAME_PATH_MAX + 32];
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\PARAM.SFO", g->dir);
    if (!sfo_read(p, g)) return g->state = GAME_NONE;
    const GameTitle* t = game_title_find(g->title_id);
    if (!t || !t->enabled) return g->state = GAME_OTHER_TITLE;
    if (t->kind == GAME_KIND_DISC) return disc_probe(update, g);
    /* The game's data folder, <id>-DATA beside the game in RPCS3, has this title in its PARAM.SFO and neither
     * GOWA.elf nor GOWA.SELF: it is not the game, and the search goes on past it. */
    swprintf(elf, sizeof elf / sizeof elf[0], L"%ls\\USRDIR\\GOWA.elf", g->dir);
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\USRDIR\\GOWA.SELF", g->dir);
    const int has_elf = is_file(elf);
    if (!has_elf && !is_file(p)) return g->state = GAME_NONE;
    /* Another version first: its files are another version's too, and the version is what to tell. */
    if (strcmp(g->app_ver, k_app_ver)) {
        g->other_version = 1;
        return g->state = GAME_OTHER_BUILD;
    }
    /* Then the whole game, before there is anything to decrypt: GOWA.SELF and a license alone are not it. */
    if (!files_check(g->dir, NULL, g)) return g->state = GAME_INCOMPLETE;
    if (!has_elf) return g->state = GAME_NO_ELF;
    return g->state = game_elf_ok(elf) ? GAME_OK : GAME_OTHER_BUILD;
}

int game_read_info(const wchar_t* dir, GameInfo* g)
{
    memset(g, 0, sizeof *g);
    folder_copy(g->dir, GAME_PATH_MAX, dir);
    wchar_t p[GAME_PATH_MAX + 32];
    swprintf(p, sizeof p / sizeof p[0], L"%ls\\PARAM.SFO", g->dir);
    return sfo_read(p, g);
}

GameState game_locate(const wchar_t* path, GameInfo* g)
{
    wchar_t base[GAME_PATH_MAX];
    folder_copy(base, GAME_PATH_MAX, path);
    /* USRDIR, or anything in it picked by mistake: the folder above. */
    const wchar_t* end = wcsrchr(base, L'\\');
    if (end && !_wcsicmp(end + 1, L"USRDIR")) base[end - base] = 0;
    /* The folder, its PS3_GAME, and for each title the launcher takes <id>, game\<id> and dev_hdd0\game\<id>. */
    std::wstring under[2 + 3 * GAME_TITLES];
    size_t nu = 0;
    under[nu++] = L"";
    under[nu++] = L"\\PS3_GAME";
    for (int i = 0; i < GAME_TITLES; i++) {
        if (!k_game_titles[i].enabled) continue;
        wchar_t id[16];
        swprintf(id, sizeof id / sizeof id[0], L"%hs", k_game_titles[i].id);
        under[nu++] = std::wstring(L"\\") + id;
        under[nu++] = std::wstring(L"\\game\\") + id;
        under[nu++] = std::wstring(L"\\dev_hdd0\\game\\") + id;
    }
    memset(g, 0, sizeof *g);
    for (size_t i = 0; i < nu; i++) {
        wchar_t p[GAME_PATH_MAX + 32];
        swprintf(p, sizeof p / sizeof p[0], L"%ls%ls", base, under[i].c_str());
        GameInfo t;
        game_probe(p, &t);
        if (game_rank(&t) > game_rank(g)) *g = t;
        if (g->state == GAME_OK) break;
    }
    return g->state;
}

const GameTitle* game_title_of_folder(const wchar_t* dir)
{
    GameInfo g;
    if (!dir || !*dir || !game_read_info(dir, &g)) return game_title_default();
    const GameTitle* t = game_title_find(g.title_id);
    return t ? t : game_title_default();
}

const GameTitle* game_title_of_vfs(const char* vfs)
{
    if (!vfs || !*vfs) return game_title_default();
    wchar_t w[GAME_PATH_MAX];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, vfs, -1, w, GAME_PATH_MAX - 16) &&
        !MultiByteToWideChar(CP_ACP, 0, vfs, -1, w, GAME_PATH_MAX - 16))
        return game_title_default();
    wcscat_s(w, GAME_PATH_MAX, L"\\PS3_GAME");
    return game_title_of_folder(w);
}

/* --- the search ------------------------------------------------------------- */
static int has_word(const wchar_t* name, const wchar_t* word)
{
    wchar_t low[260];
    size_t i = 0;
    for (; name[i] && i < 259; i++) low[i] = towlower(name[i]);
    low[i] = 0;
    return wcsstr(low, word) != NULL;
}

/* Folders worth looking in before the rest. */
static int sounds_like_games(const wchar_t* name)
{
    static const wchar_t* const words[] = {
        L"rpcs3", L"ps3", L"playstation", L"dev_hdd0", L"game", L"emul", L"npua", L"god of war", L"gow", L"rom",
    };
    for (size_t i = 0; i < sizeof words / sizeof words[0]; i++)
        if (has_word(name, words[i])) return 1;
    return 0;
}

/* Folders no game is in, and that take long to walk. */
static int not_worth_it(const wchar_t* name)
{
    static const wchar_t* const names[] = {
        L"Windows", L"Windows.old", L"$Recycle.Bin", L"System Volume Information", L"Recovery", L"PerfLogs",
        L"ProgramData", L"AppData", L"$WinREAgent", L"Config.Msi", L"MSOCache", L"node_modules", L".git", L".svn",
        L"__pycache__", L".venv", L"WindowsApps", L"WinSxS",
    };
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
        if (!_wcsicmp(name, names[i])) return 1;
    return 0;
}

struct Dir { std::wstring path; int depth; };

static void keep_best(GameSearch* s, GameInfo* best, const GameInfo& g, std::set<std::wstring>& seen)
{
    /* Another title counts only if it is this game: another release of it. A disc and its update are one copy,
     * whichever of the two the walk came to first: each probe pairs them. */
    if (g.state == GAME_OTHER_TITLE && !strstr(g.title, "Ascension")) return;
    std::wstring key = std::wstring(g.dir) + L"|" + g.update;
    for (wchar_t& c : key) c = c == L'/' ? L'\\' : towlower(c);
    if (g.state >= GAME_OTHER_TITLE && seen.insert(key).second) s->copies++;
    if (game_rank(&g) > game_rank(best)) *best = g;
}

static int state_rank(GameState s, int other_version)
{
    switch (s) {
    case GAME_OK: return 6;
    case GAME_NO_ELF: return 5;
    case GAME_OTHER_BUILD: return other_version ? 3 : 4;
    case GAME_INCOMPLETE: return 2;
    case GAME_OTHER_TITLE: return 1;
    default: return 0;
    }
}

int game_rank(const GameInfo* g)
{
    return state_rank(g->state, g->other_version);
}

GameState game_search(GameSearch* s, GameInfo* best)
{
    memset(best, 0, sizeof *best);
    s->copies = 0;
    std::set<std::wstring> seen;
    const ULONGLONG until = GetTickCount64() + (ULONGLONG)(s->seconds > 0 ? s->seconds : 60) * 1000u;
    std::deque<Dir> hot, cold;
    if (s->under) {
        hot.push_back({ s->under, 0 });
    } else {
        /* The places RPCS3 keeps its files when it is installed for one user. */
        static const wchar_t* const vars[] = { L"LOCALAPPDATA", L"APPDATA", L"USERPROFILE" };
        for (size_t i = 0; i < sizeof vars / sizeof vars[0]; i++) {
            wchar_t v[GAME_PATH_MAX], p[GAME_PATH_MAX + 16];
            if (!GetEnvironmentVariableW(vars[i], v, GAME_PATH_MAX)) continue;
            swprintf(p, sizeof p / sizeof p[0], L"%ls\\rpcs3", v);
            GameInfo g;
            if (game_locate(p, &g) > GAME_NONE) keep_best(s, best, g, seen);
        }
        /* Every disk, the system's last: games are mostly on another one. */
        wchar_t sys[MAX_PATH] = L"C";
        GetSystemWindowsDirectoryW(sys, MAX_PATH);
        const DWORD drives = GetLogicalDrives();
        for (int i = 0; i < 26; i++) {
            if (!(drives & (1u << i))) continue;
            wchar_t root[4] = { (wchar_t)(L'A' + i), L':', L'\\', 0 };
            const UINT type = GetDriveTypeW(root);
            if (type != DRIVE_FIXED && type != DRIVE_REMOVABLE) continue;
            if (towupper(root[0]) == towupper(sys[0])) cold.push_back({ root, 0 });
            else cold.push_front({ root, 0 });
        }
    }
    const int max_depth = s->under ? 12 : 9;
    const GameState enough = s->good_enough ? (GameState)s->good_enough : GAME_OK;
    while (game_rank(best) < state_rank(enough, 0) && (!hot.empty() || !cold.empty())) {
        if ((s->stop && *s->stop) || GetTickCount64() >= until) break;
        Dir d;
        if (!hot.empty()) { d = std::move(hot.back()); hot.pop_back(); }
        else { d = std::move(cold.front()); cold.pop_front(); }
        if (s->now) s->now(d.path.c_str());
        std::wstring pat = d.path;
        if (pat.empty() || pat.back() != L'\\') pat += L'\\';
        const std::wstring prefix = pat;
        pat += L'*';
        if (pat.size() >= GAME_PATH_MAX - 40) continue;
        WIN32_FIND_DATAW fd;
        HANDLE f = FindFirstFileExW(pat.c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, NULL,
                                    FIND_FIRST_EX_LARGE_FETCH);
        if (f == INVALID_HANDLE_VALUE) continue;
        std::deque<Dir> found_hot, found_cold;
        int game_here = 0;
        do {
            const wchar_t* n = fd.cFileName;
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (!_wcsicmp(n, L"PARAM.SFO")) game_here = 1;
                else if (!_wcsicmp(n, L"rpcs3.exe")) game_note_rpcs3(d.path.c_str());   /* where a disc's update is */
                continue;
            }
            if (!wcscmp(n, L".") || !wcscmp(n, L"..")) continue;
            /* A link leads to a folder found on its own way, or round in a circle. */
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            if (d.depth >= max_depth || not_worth_it(n)) continue;
            if (sounds_like_games(n) || is_title_dir(n) || !_wcsicmp(n, L"PS3_GAME"))
                found_hot.push_back({ prefix + n, d.depth + 1 });
            else
                found_cold.push_back({ prefix + n, d.depth + 1 });
        } while (FindNextFileW(f, &fd));
        FindClose(f);
        if (game_here) {
            GameInfo g;
            if (game_probe(d.path.c_str(), &g) > GAME_NONE) keep_best(s, best, g, seen);
            continue;   /* nothing of interest under a game */
        }
        for (Dir& x : found_hot) hot.push_back(std::move(x));
        for (Dir& x : found_cold) cold.push_back(std::move(x));
    }
    return best->state;
}

/* --- links ------------------------------------------------------------------- */
/* What FSCTL_GET_REPARSE_POINT and FSCTL_SET_REPARSE_POINT take: REPARSE_DATA_BUFFER of the
 * driver kit, which the SDK's headers leave out. */
struct Reparse {
    ULONG tag;
    USHORT data_len, reserved;
    USHORT sub_off, sub_len, print_off, print_len;
    union {
        struct { ULONG flags; WCHAR link_path[1]; };   /* IO_REPARSE_TAG_SYMLINK */
        WCHAR mount_path[1];                           /* IO_REPARSE_TAG_MOUNT_POINT */
    };
};
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT 0xA0000003L
#endif
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000CL
#endif
enum { REPARSE_MAX = 16 * 1024 };

int game_link_target(const wchar_t* link, wchar_t* out, size_t n)
{
    if (n) out[0] = 0;
    const DWORD a = GetFileAttributesW(link);
    if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_REPARSE_POINT)) return 0;
    HANDLE h = CreateFileW(link, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                           FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    alignas(8) unsigned char buf[REPARSE_MAX];
    DWORD got = 0;
    const BOOL ok = DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, buf, sizeof buf, &got, NULL);
    CloseHandle(h);
    if (!ok) return 0;
    const Reparse* r = (const Reparse*)buf;
    const WCHAR* base = r->tag == IO_REPARSE_TAG_MOUNT_POINT ? r->mount_path :
                        r->tag == IO_REPARSE_TAG_SYMLINK ? r->link_path : NULL;
    if (!base) return 0;
    /* The name to print, else the one the system uses: \??\D:\Games */
    const WCHAR* s = r->print_len ? base + r->print_off / 2 : base + r->sub_off / 2;
    size_t len = (r->print_len ? r->print_len : r->sub_len) / 2;
    if (len >= 4 && !wcsncmp(s, L"\\??\\", 4)) { s += 4; len -= 4; }
    if (len >= n) return 0;
    wmemcpy(out, s, len);
    out[len] = 0;
    return 1;
}

static int same_folder(const wchar_t* a, const wchar_t* b)
{
    wchar_t x[GAME_PATH_MAX], y[GAME_PATH_MAX];
    folder_copy(x, GAME_PATH_MAX, a);
    folder_copy(y, GAME_PATH_MAX, b);
    for (wchar_t* p = x; *p; p++) if (*p == L'/') *p = L'\\';
    for (wchar_t* p = y; *p; p++) if (*p == L'/') *p = L'\\';
    return !_wcsicmp(x, y);
}

static void say(wchar_t* err, size_t n, const wchar_t* what, const wchar_t* path)
{
    if (err && n) swprintf(err, n, L"%ls: %ls", what, path);
}

int game_link_possible(const wchar_t* link, const wchar_t* target, wchar_t* err, size_t n)
{
    if (same_folder(link, target)) return 1;
    const DWORD a = GetFileAttributesW(link);
    if (a == INVALID_FILE_ATTRIBUTES || (a & FILE_ATTRIBUTE_REPARSE_POINT)) return 1;
    if (!(a & FILE_ATTRIBUTE_DIRECTORY)) { say(err, n, L"A file is in the way", link); return 0; }
    wchar_t pat[GAME_PATH_MAX + 8];
    swprintf(pat, sizeof pat / sizeof pat[0], L"%ls\\*", link);
    WIN32_FIND_DATAW fd;
    HANDLE f = FindFirstFileW(pat, &fd);
    int empty = 1;
    if (f != INVALID_HANDLE_VALUE) {
        do { if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..")) empty = 0; } while (empty && FindNextFileW(f, &fd));
        FindClose(f);
    }
    if (!empty) say(err, n, L"A real folder with files is in the way", link);
    return empty;
}

static void make_parents(const wchar_t* link)
{
    wchar_t p[GAME_PATH_MAX];
    folder_copy(p, GAME_PATH_MAX, link);
    for (wchar_t* q = p; *q; q++) {
        if (*q != L'\\' || q == p || q[-1] == L':') continue;
        *q = 0;
        CreateDirectoryW(p, NULL);
        *q = L'\\';
    }
}

static int make_junction(const wchar_t* link, const wchar_t* target, wchar_t* err, size_t n)
{
    wchar_t full[GAME_PATH_MAX];
    if (!GetFullPathNameW(target, GAME_PATH_MAX, full, NULL)) { say(err, n, L"Not a folder", target); return 0; }
    wchar_t clean[GAME_PATH_MAX];
    folder_copy(clean, GAME_PATH_MAX, full);
    if (!wcsncmp(clean, L"\\\\", 2)) { say(err, n, L"A link cannot lead to a network folder", clean); return 0; }
    make_parents(link);
    if (!CreateDirectoryW(link, NULL)) { say(err, n, L"Cannot make the link", link); return 0; }
    HANDLE h = CreateFileW(link, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                           FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        RemoveDirectoryW(link);
        say(err, n, L"Cannot make the link", link);
        return 0;
    }
    alignas(8) unsigned char buf[REPARSE_MAX];
    memset(buf, 0, sizeof buf);
    Reparse* r = (Reparse*)buf;
    const size_t len = wcslen(clean);
    const USHORT sub = (USHORT)((len + 4) * 2), print = (USHORT)(len * 2);
    r->tag = IO_REPARSE_TAG_MOUNT_POINT;
    r->sub_off = 0;
    r->sub_len = sub;
    r->print_off = (USHORT)(sub + 2);
    r->print_len = print;
    wmemcpy(r->mount_path, L"\\??\\", 4);
    wmemcpy(r->mount_path + 4, clean, len);
    wmemcpy(r->mount_path + len + 5, clean, len);
    r->data_len = (USHORT)(8 + sub + 2 + print + 2);
    DWORD got = 0;
    const BOOL ok = DeviceIoControl(h, FSCTL_SET_REPARSE_POINT, buf, 8u + r->data_len, NULL, 0, &got, NULL);
    CloseHandle(h);
    if (!ok) {
        RemoveDirectoryW(link);
        say(err, n, L"Cannot make the link", link);
    }
    return ok;
}

int game_link(const wchar_t* link, const wchar_t* target, wchar_t* err, size_t n)
{
    if (same_folder(link, target)) return 1;
    wchar_t now[GAME_PATH_MAX];
    if (game_link_target(link, now, GAME_PATH_MAX) && same_folder(now, target)) return 1;
    if (!game_link_possible(link, target, err, n)) return 0;
    /* A link, or an empty folder: RemoveDirectory takes the link itself, never what it leads to. */
    if (GetFileAttributesW(link) != INVALID_FILE_ATTRIBUTES && !RemoveDirectoryW(link)) {
        say(err, n, L"Cannot replace the link", link);
        return 0;
    }
    return make_junction(link, target, err, n);
}

/* Ascension-R --find-game [folder]: what Auto detect (every disk) or Browse (under the folder) comes to, a line each,
 * for a report or a test. 0 = a copy the launcher can take. */
extern "C" int gowa_find_game_cli(const char* under)
{
    static const char* const names[] = { "none", "other_title", "incomplete", "no_elf", "other_build", "ok" };
    static wchar_t u[GAME_PATH_MAX];
    u[0] = 0;
    if (under && *under) MultiByteToWideChar(CP_UTF8, 0, under, -1, u, GAME_PATH_MAX);
    volatile long stop = 0;
    GameSearch s = { u[0] ? u : NULL, &stop, NULL, 180, 0, 0 };
    static GameInfo g;
    game_search(&s, &g);
    printf("[find] state %s\n[find] title %s %s, %s\n[find] disc %d lacks %d\n", names[g.state],
           g.title_id[0] ? g.title_id : "-", g.app_ver[0] ? g.app_ver : "-", g.category[0] ? g.category : "-", g.disc,
           (int)g.disc_lack);
    printf("[find] dir %ls\n[find] update %ls\n[find] copies %d\n", g.dir[0] ? g.dir : L"-", g.update[0] ? g.update : L"-",
           s.copies);
    if (g.state == GAME_INCOMPLETE) {
        wchar_t t[600];
        game_files_text(&g, t, sizeof t / sizeof t[0]);
        printf("[find] says %ls\n", t);
    }
    return g.state >= GAME_NO_ELF ? 0 : 1;
}

#endif /* _WIN32 */
