/*
 * GOWA_cMc_OmniPort - launcher nativo (un solo binario, copiado con varios nombres).
 *
 *   Jugar GOWA-cMc.exe       arranca el juego (--no-launcher + GOWA.elf), sin consola
 *   Launcher GOWA-cMc.exe    abre el launcher del port
 *   Update-BCUS.exe          Update-BCUS.ps1 en una consola que queda abierta
 *   Promote-BCUS.exe         Promote-BCUS.ps1
 *   Rollback-BCUS.exe        Rollback-BCUS.ps1 (con confirmacion)
 *
 * Las rutas se calculan desde la ubicacion del propio .exe: el paquete es portable.
 * Los argumentos que reciba se pasan al script o al ejecutable del juego.
 *
 * Port a BCUS98232: cMc-OmniPort.
 * Motor: Ascension-R de Ryan Bagas + ps3recomp de Ned Heller (sp00nznet) y colaboradores.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAXP 1024

static void Cat(WCHAR* dst, int cap, const WCHAR* a, const WCHAR* b)
{
    int n = (int)lstrlenW(dst);
    const WCHAR* parts[2];
    parts[0] = a;
    parts[1] = b;
    for (int i = 0; i < 2; i++) {
        const WCHAR* s = parts[i];
        while (*s && n < cap - 1) dst[n++] = *s++;
    }
    dst[n] = 0;
}

static BOOL FileExists(const WCHAR* p)
{
    DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static BOOL EndsWith(const WCHAR* s, const WCHAR* suffix)
{
    int ls = (int)lstrlenW(s), lf = (int)lstrlenW(suffix);
    if (lf > ls) return FALSE;
    return lstrcmpiW(s + ls - lf, suffix) == 0;
}

static void Error(const WCHAR* text)
{
    MessageBoxW(NULL, text, L"GOWA_cMc_OmniPort", MB_OK | MB_ICONERROR);
}

/* Everything after the program name in the command line (arguments to forward). */
static const WCHAR* ForwardedArgs(void)
{
    static WCHAR tail[MAXP];
    const WCHAR* cmd = GetCommandLineW();
    const WCHAR* p = cmd;
    if (*p == L'"') {
        p++;
        while (*p && *p != L'"') p++;
        if (*p == L'"') p++;
    } else {
        while (*p && *p != L' ') p++;
    }
    while (*p == L' ' || *p == L'\t') p++;
    lstrcpynW(tail, p, MAXP);
    return tail;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR args, int show)
{
    WCHAR self[MAXP];
    if (!GetModuleFileNameW(NULL, self, MAXP)) { Error(L"No se pudo leer la ruta del lanzador."); return 1; }

    WCHAR name[MAXP];
    lstrcpynW(name, self, MAXP);
    {
        WCHAR* slash = wcsrchr(name, L'\\');
        if (slash) *slash = 0; else name[0] = 0;
    }
    const WCHAR* base = self;
    {
        const WCHAR* slash = wcsrchr(self, L'\\');
        if (slash) base = slash + 1;
    }

    WCHAR root[MAXP];
    lstrcpynW(root, self, MAXP);
    {
        WCHAR* slash = wcsrchr(root, L'\\');
        if (slash) *slash = 0;
    }

    WCHAR repo[MAXP] = L"";
    Cat(repo, MAXP, root, L"\\_src\\GOWA-cMc");

    const BOOL isPlay = (wcsstr(base, L"Jugar") != NULL);
    const BOOL isTool = (wcsstr(base, L"Update") != NULL) ||
                        (wcsstr(base, L"Promote") != NULL) ||
                        (wcsstr(base, L"Rollback") != NULL);

    WCHAR exe[MAXP] = L"";
    Cat(exe, MAXP, repo, L"\\build\\GOWA-cMc.exe");
    if (!FileExists(exe)) {
        WCHAR msg[MAXP] = L"No esta compilado todavia:\r\n";
        Cat(msg, MAXP, exe, L"\r\n\r\nEjecuta Update-BCUS.exe para compilar una release.");
        Error(msg);
        return 1;
    }

    /* Tools: run the PowerShell script in a console that stays open. */
    if (isTool) {
        WCHAR ps1[MAXP] = L"";
        Cat(ps1, MAXP, root, L"\\tools\\bcus\\");
        Cat(ps1, MAXP, base, L"");
        /* name: <Tool>-BCUS.exe -> <Tool>-BCUS.ps1 */
        {
            WCHAR* dot = wcsrchr(ps1, L'.');
            if (dot) lstrcpynW(dot, L".ps1", 8);
        }
        WCHAR cmd[2 * MAXP] = L"";
        Cat(cmd, 2 * MAXP, L"powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \"", ps1);
        Cat(cmd, 2 * MAXP, L"\" ", ForwardedArgs());
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof si);
        si.cb = sizeof si;
        ZeroMemory(&pi, sizeof pi);
        SetCurrentDirectoryW(root);
        if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, root, &si, &pi)) {
            Error(L"No se pudo abrir la ventana de la herramienta.");
            return 1;
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }

    /* Game: environment first (the launcher and the game read these). */
    WCHAR vfs[MAXP] = L"", hdd[MAXP] = L"", elf[MAXP] = L"", ffbin[MAXP] = L"", newpath[4 * MAXP] = L"";
    Cat(vfs, MAXP, repo, L"\\vfs");
    Cat(hdd, MAXP, repo, L"\\gamedata\\dev_hdd0");
    Cat(elf, MAXP, repo, L"\\vfs\\PS3_GAME\\USRDIR\\GOWA.elf");
    Cat(ffbin, MAXP, root, L"\\_src\\ffmpeg\\ffmpeg-n8.1-latest-win64-lgpl-shared-8.1\\bin");

    SetEnvironmentVariableW(L"GOWA_TITLE_ID", L"BCUS98232");
    SetEnvironmentVariableW(L"PS3_TITLE", L"BCUS98232");
    SetEnvironmentVariableW(L"PS3_BOOT_PATH", L"/dev_hdd0/game/BCUS98232/USRDIR/GOWA.SELF");
    SetEnvironmentVariableW(L"PS3_VFS_ROOT", vfs);
    SetEnvironmentVariableW(L"PS3_HDD0_ROOT", hdd);
    SetEnvironmentVariableW(L"PS3_MODS_ROOT", L"");
    {
        WCHAR oldpath[4 * MAXP] = L"";
        GetEnvironmentVariableW(L"PATH", oldpath, 4 * MAXP);
        lstrcpynW(newpath, ffbin, 4 * MAXP);
        Cat(newpath, 4 * MAXP, L";", oldpath);
        SetEnvironmentVariableW(L"PATH", newpath);
    }

    if (isPlay && !FileExists(elf)) {
        WCHAR msg[MAXP] = L"Falta GOWA.elf (descifrado) en:\r\n";
        Cat(msg, MAXP, elf, L"\r\n\r\nEjecuta una vez, con tu copia del juego:\r\n"
                             L"python scripts\\gowa.py setup --game-dir <carpeta del juego>");
        Error(msg);
        return 1;
    }

    WCHAR cmd[2 * MAXP] = L"";
    Cat(cmd, 2 * MAXP, L"\"", exe);
    if (isPlay) {
        Cat(cmd, 2 * MAXP, L"\" --no-launcher \"", elf);
        Cat(cmd, 2 * MAXP, L"\" ", ForwardedArgs());
    } else {
        Cat(cmd, 2 * MAXP, L"\" ", ForwardedArgs());
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof si);
    si.cb = sizeof si;
    ZeroMemory(&pi, sizeof pi);
    SetCurrentDirectoryW(repo);
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, repo, &si, &pi)) {
        Error(L"No se pudo iniciar GOWA-cMc.exe.");
        return 1;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
