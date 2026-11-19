/*
 * Portable launchers for GOWA-cMc 0.2.0.
 *
 * Launcher GOWA-cMc.exe: opens the cMc launcher UI.
 * Jugar GOWA-cMc.exe:    starts the game directly with its ELF.
 *
 * Both look beside themselves, so the folder can be moved anywhere.
 */
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <stdio.h>

static void dir_of(wchar_t* out, DWORD n)
{
    if (!GetModuleFileNameW(NULL, out, n)) { out[0] = 0; return; }
    wchar_t* slash = wcsrchr(out, L'\\');
    if (slash) *slash = 0;
}

int wmain(void)
{
    wchar_t dir[MAX_PATH], exe[MAX_PATH], elf[MAX_PATH], cmd[2 * MAX_PATH + 64];
    dir_of(dir, MAX_PATH);
    swprintf(exe, MAX_PATH, L"%ls\\GOWA-cMc.exe", dir);
    swprintf(elf, MAX_PATH, L"%ls\\GOWA.elf", dir);
    if (GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(NULL, L"No se encontró GOWA-cMc.exe junto al lanzador.", L"GOWA-cMc", MB_ICONERROR);
        return 2;
    }
#ifdef GOWA_JUGAR
    if (GetFileAttributesW(elf) != INVALID_FILE_ATTRIBUTES)
        swprintf(cmd, 2 * MAX_PATH + 64, L"\"%ls\" --no-launcher \"%ls\"", exe, elf);
    else
        swprintf(cmd, 2 * MAX_PATH + 64, L"\"%ls\"", exe);
#else
    swprintf(cmd, 2 * MAX_PATH + 64, L"\"%ls\"", exe);
#endif
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof si;
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi)) {
        MessageBoxW(NULL, L"No se pudo iniciar GOWA-cMc.exe.", L"GOWA-cMc", MB_ICONERROR);
        return 3;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
