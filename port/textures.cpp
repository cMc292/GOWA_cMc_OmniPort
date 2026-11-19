/*
 * Texture mods for the D3D12 backend's hooks (textures.h, ps3recomp's rsx_tex_replace.h). The pictures are read
 * and the dump is written with Windows' own imaging (WIC).
 */
#include "textures.h"
#include "mods.h"

#include <windows.h>
#include <wincodec.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ps3emu/ps3_settings.h"
#include "rsx_tex_replace.h"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

namespace {

struct Pic {
    std::wstring path;
    int kind;     /* 1 in the texture's place, 2 over the game's own */
    int res = 0;  /* a picture in the exe (app.rc's RCDATA) in place of a file */
};

/* Button icons: Xbox. The game's button textures, the same in every release, by the button each is of, and the
 * port's own picture for it (app.rc, scripts/button_icons.ps1): the face buttons by where each sits on the pad. */
struct Builtin { uint64_t id; int res; };
const Builtin k_xbox_icons[] = {
    { 0xd8c17e20cbbe71acULL, 201 },   /* Cross, A */
    { 0x1f52688dc50e2bfbULL, 202 },   /* Circle, B */
    { 0x2133aab0625680afULL, 203 },   /* Square, X */
    { 0x660940394f9aee54ULL, 204 },   /* Triangle, Y */
    { 0xa4f295497b5454b7ULL, 205 },   /* Select's bar, View */
};

/* A texture for the dump's thread to write. */
struct DumpJob {
    std::wstring path;
    uint32_t w, h;
    std::vector<uint8_t> rgba;
};

struct State {
    std::mutex mu;
    std::unordered_map<uint64_t, Pic> pics;
    std::wstring dump;                 /* the dump's folder, or "" */
    std::unordered_set<uint64_t> dumped;
    std::unordered_set<uint64_t> bad;  /* pictures WIC could not read: said once */
    std::vector<uint8_t> out;          /* the last picture handed over */
    /* The dump is written on a thread of its own, so the game does not wait for PNGs; the frame waits only when
     * more than k_dump_queue bytes are queued. */
    std::mutex dmu;
    std::condition_variable dcv;
    std::deque<DumpJob> jobs;
    size_t queued = 0;
    bool writer = false;
};
State& st()
{
    static State* s = new State;   /* never freed: the backend may call in as the process ends */
    return *s;
}

const int k_max_dump = 50000;
const size_t k_dump_queue = 256u << 20;

IWICImagingFactory* wic(void)
{
    static thread_local bool com = false;
    if (!com) {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);   /* RPC_E_CHANGED_MODE: the thread has COM already */
        com = true;
    }
    static IWICImagingFactory* f = NULL;
    if (!f) CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f));
    return f;
}

template <class T> void release(T*& p)
{
    if (p) p->Release();
    p = NULL;
}

/* A PNG from its file, or from the exe's resource res. */
bool load_png(const std::wstring& path, int res, uint32_t* w, uint32_t* h, std::vector<uint8_t>* rgba)
{
    IWICImagingFactory* f = wic();
    if (!f) return false;
    IWICBitmapDecoder* dec = NULL;
    IWICBitmapFrameDecode* fr = NULL;
    IWICFormatConverter* cv = NULL;
    IWICStream* mem = NULL;
    bool ok = false, opened = false;
    UINT fw = 0, fh = 0;
    if (res) {
        HRSRC r = FindResourceW(NULL, MAKEINTRESOURCEW(res), MAKEINTRESOURCEW(10) /* RT_RCDATA */);
        HGLOBAL g = r ? LoadResource(NULL, r) : NULL;
        void* data = g ? LockResource(g) : NULL;
        const DWORD n = r ? SizeofResource(NULL, r) : 0;
        opened = data && n && SUCCEEDED(f->CreateStream(&mem)) &&
                 SUCCEEDED(mem->InitializeFromMemory((BYTE*)data, n)) &&
                 SUCCEEDED(f->CreateDecoderFromStream(mem, NULL, WICDecodeMetadataCacheOnDemand, &dec));
    } else {
        opened = SUCCEEDED(f->CreateDecoderFromFilename(path.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand,
                                                        &dec));
    }
    if (opened &&
        SUCCEEDED(dec->GetFrame(0, &fr)) && SUCCEEDED(fr->GetSize(&fw, &fh)) && fw && fh && fw <= 16384 && fh <= 16384 &&
        SUCCEEDED(f->CreateFormatConverter(&cv)) &&
        SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0,
                                 WICBitmapPaletteTypeCustom))) {
        rgba->resize((size_t)fw * fh * 4);
        ok = SUCCEEDED(cv->CopyPixels(NULL, fw * 4, (UINT)rgba->size(), rgba->data()));
        *w = fw;
        *h = fh;
    }
    release(cv);
    release(fr);
    release(dec);
    release(mem);
    return ok;
}

bool save_png(const std::wstring& path, uint32_t w, uint32_t h, const uint8_t* rgba)
{
    IWICImagingFactory* f = wic();
    if (!f) return false;
    std::vector<uint8_t> bgra((size_t)w * h * 4);
    for (size_t i = 0; i < (size_t)w * h; i++) {
        bgra[i * 4 + 0] = rgba[i * 4 + 2];
        bgra[i * 4 + 1] = rgba[i * 4 + 1];
        bgra[i * 4 + 2] = rgba[i * 4 + 0];
        bgra[i * 4 + 3] = rgba[i * 4 + 3];
    }
    IWICStream* stream = NULL;
    IWICBitmapEncoder* enc = NULL;
    IWICBitmapFrameEncode* fe = NULL;
    IPropertyBag2* props = NULL;
    WICPixelFormatGUID pf = GUID_WICPixelFormat32bppBGRA;
    bool ok = SUCCEEDED(f->CreateStream(&stream)) && SUCCEEDED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE)) &&
              SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, NULL, &enc)) &&
              SUCCEEDED(enc->Initialize(stream, WICBitmapEncoderNoCache)) && SUCCEEDED(enc->CreateNewFrame(&fe, &props)) &&
              SUCCEEDED(fe->Initialize(props)) && SUCCEEDED(fe->SetSize(w, h)) && SUCCEEDED(fe->SetPixelFormat(&pf)) &&
              IsEqualGUID(pf, GUID_WICPixelFormat32bppBGRA) &&
              SUCCEEDED(fe->WritePixels(h, w * 4, (UINT)bgra.size(), bgra.data())) && SUCCEEDED(fe->Commit()) &&
              SUCCEEDED(enc->Commit());
    release(props);
    release(fe);
    release(enc);
    release(stream);
    if (!ok) DeleteFileW(path.c_str());
    return ok;
}

/* A picture's id and kind from its file's name: sixteen hex digits first, then the end of the name, '_' or '.'. */
bool parse_name(const wchar_t* name, uint64_t* id, int* kind)
{
    const size_t n = wcslen(name);
    if (n < 20 || _wcsicmp(name + n - 4, L".png")) return false;
    uint64_t v = 0;
    for (int i = 0; i < 16; i++) {
        const wchar_t c = name[i];
        const int d = c >= L'0' && c <= L'9' ? c - L'0' : c >= L'a' && c <= L'f' ? c - L'a' + 10
                    : c >= L'A' && c <= L'F' ? c - L'A' + 10 : -1;
        if (d < 0) return false;
        v = v << 4 | (uint64_t)d;
    }
    if (name[16] != L'.' && name[16] != L'_') return false;
    std::wstring low(name);
    for (wchar_t& c : low) c = (wchar_t)towlower(c);
    *id = v;
    *kind = low.size() >= 12 && !low.compare(low.size() - 12, 12, L".overlay.png") ? 2 : 1;
    return true;
}

void index_dir(const std::wstring& dir, int depth, std::unordered_map<uint64_t, Pic>* out, int* found)
{
    if (depth > 16) return;
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, 0);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..")) continue;
        const std::wstring p = dir + L"\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            index_dir(p, depth + 1, out, found);
            continue;
        }
        uint64_t id;
        int kind;
        if (!parse_name(fd.cFileName, &id, &kind)) continue;
        (*found)++;
        if (!out->count(id)) out->emplace(id, Pic{ p, kind });   /* a mod above has it already: that one */
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

int has_hook(u64 id)
{
    State& s = st();
    std::lock_guard<std::mutex> lk(s.mu);
    auto it = s.pics.find(id);
    return it == s.pics.end() || s.bad.count(id) ? 0 : it->second.kind;
}

int picture_hook(u64 id, u32 w, u32 h, const u8* base, rsx_tex_picture* out)
{
    State& s = st();
    std::lock_guard<std::mutex> lk(s.mu);
    auto it = s.pics.find(id);
    if (it == s.pics.end()) return 0;
    uint32_t pw = 0, ph = 0;
    std::vector<uint8_t> png;
    if (!load_png(it->second.path, it->second.res, &pw, &ph, &png)) {
        if (s.bad.insert(id).second) {
            char u8[1200];
            WideCharToMultiByte(CP_UTF8, 0, it->second.path.c_str(), -1, u8, sizeof u8, NULL, NULL);
            fprintf(stderr, "[tex] %s: not a picture this program can read%c", u8, 10);
        }
        return 0;
    }
    if (it->second.kind == 2) {   /* over the game's own, scaled to the overlay's size */
        if (!base) return 0;
        for (uint32_t y = 0; y < ph; y++) {
            const uint8_t* brow = base + (size_t)((uint64_t)y * h / ph) * w * 4;
            for (uint32_t x = 0; x < pw; x++) {
                uint8_t* o = &png[((size_t)y * pw + x) * 4];
                const uint8_t* b = brow + (size_t)((uint64_t)x * w / pw) * 4;
                const uint32_t a = o[3];
                if (a == 0) {
                    memcpy(o, b, 4);
                } else if (a == 1) {
                    o[0] = o[1] = o[2] = o[3] = 0;
                } else {
                    for (int c = 0; c < 3; c++) o[c] = (uint8_t)((o[c] * a + b[c] * (255u - a) + 127u) / 255u);
                    o[3] = (uint8_t)(a + (b[3] * (255u - a) + 127u) / 255u);
                }
            }
        }
    }
    s.out.swap(png);
    out->w = pw;
    out->h = ph;
    out->rgba = s.out.data();
    return 1;
}

void dump_writer(void)
{
    State& s = st();
    for (;;) {
        DumpJob j;
        {
            std::unique_lock<std::mutex> lk(s.dmu);
            s.dcv.wait(lk, [&] { return !s.jobs.empty(); });
            j = std::move(s.jobs.front());
            s.jobs.pop_front();
        }
        save_png(j.path, j.w, j.h, j.rgba.data());
        {
            std::lock_guard<std::mutex> lk(s.dmu);
            s.queued -= j.rgba.size();
        }
        s.dcv.notify_all();
    }
}

void dump_hook(u64 id, u32 w, u32 h, u32 fmt, const u8* base)
{
    State& s = st();
    std::wstring p;
    {
        std::lock_guard<std::mutex> lk(s.mu);
        if (s.dump.empty() || (int)s.dumped.size() >= k_max_dump || !s.dumped.insert(id).second) return;
        wchar_t name[96];
        swprintf(name, 96, L"\\%016llx_%ux%u_%02X.png", (unsigned long long)id, w, h, fmt & 0xFFu);
        p = s.dump + name;
    }
    if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) return;   /* an earlier run's */
    DumpJob j;
    j.path = p;
    j.w = w;
    j.h = h;
    j.rgba.assign(base, base + (size_t)w * h * 4);
    std::unique_lock<std::mutex> lk(s.dmu);
    if (!s.writer) {
        std::thread(dump_writer).detach();
        s.writer = true;
    }
    s.dcv.wait(lk, [&] { return s.queued + j.rgba.size() <= k_dump_queue || s.jobs.empty(); });
    s.queued += j.rgba.size();
    s.jobs.push_back(std::move(j));
    s.dcv.notify_all();
}

std::wstring settings_dir(void)
{
    wchar_t p[MAX_PATH * 2];
    const char* s = ps3_settings_path();
    if (s && *s && MultiByteToWideChar(CP_UTF8, 0, s, -1, p, MAX_PATH * 2)) {
        for (wchar_t* c = p; *c; c++) if (*c == L'/') *c = L'\\';
        if (wchar_t* e = wcsrchr(p, L'\\')) { *e = 0; return p; }
    }
    const DWORD n = GetModuleFileNameW(NULL, p, MAX_PATH);
    if (n && n < MAX_PATH) {
        if (wchar_t* e = wcsrchr(p, L'\\')) *e = 0;
        return p;
    }
    return L".";
}

}  // namespace

int gowa_textures_xbox_icons(void)
{
    const char* v = ps3_settings_str("input", "button_icons", "GOWA_BUTTON_ICONS", "auto");
    if (v && ps3_settings_ieq(v, "xbox")) return 1;
    if (v && ps3_settings_ieq(v, "playstation")) return 0;
    /* auto: an XInput controller plugged in now. XInput is loaded as the runtime loads it, from System32. */
    typedef DWORD(WINAPI * GetState)(DWORD, void*);
    static const wchar_t* const dlls[] = { L"XInput1_4.dll", L"XInput1_3.dll", L"XInput9_1_0.dll" };
    for (const wchar_t* d : dlls) {
        HMODULE m = LoadLibraryExW(d, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!m) continue;
        GetState get = (GetState)(void*)GetProcAddress(m, "XInputGetState");
        int any = 0;
        unsigned char state[16];
        for (DWORD i = 0; get && i < 4 && !any; i++) any = get(i, state) == ERROR_SUCCESS;
        FreeLibrary(m);
        return any;
    }
    return 0;
}

std::wstring gowa_textures_dump_folder(void)
{
    if (const wchar_t* e = _wgetenv(L"GOWA_TEXTURE_DUMP")) if (*e) return e;
    return settings_dir() + L"\\textures-dump";
}

int gowa_textures_start(const ModList& l, const std::wstring& dump_dir, int xbox_icons)
{
    State& s = st();
    int found = 0, builtin = 0;
    {
        std::lock_guard<std::mutex> lk(s.mu);
        s.pics.clear();
        s.bad.clear();
        if (l.use)
            for (const ModInfo& m : l.mods)
                if (m.on && m.error.empty() && !m.textures.empty()) index_dir(m.textures, 0, &s.pics, &found);
        if (xbox_icons)   /* after the mods: a mod's picture of a button wins over the port's */
            for (const Builtin& b : k_xbox_icons)
                if (!s.pics.count(b.id)) {
                    Pic p;
                    p.path = L"Button icons: Xbox";
                    p.kind = 1;
                    p.res = b.res;
                    s.pics.emplace(b.id, p);
                    builtin++;
                }
        s.dump = dump_dir;
        if (!s.dump.empty() && !CreateDirectoryW(s.dump.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
            s.dump.clear();
    }
    if (s.pics.empty() && s.dump.empty()) {
        rsx_tex_set_hooks(NULL);
        return 0;
    }
    rsx_tex_hooks hk;
    hk.has = has_hook;
    hk.picture = picture_hook;
    hk.dump = s.dump.empty() ? NULL : dump_hook;
    rsx_tex_set_hooks(&hk);
    char u8[1200] = "";
    if (!s.dump.empty()) WideCharToMultiByte(CP_UTF8, 0, s.dump.c_str(), -1, u8, sizeof u8, NULL, NULL);
    printf("[tex] %d pictures of mods, %d of the Xbox button icons, for %d textures%s%s\n", found, builtin,
           (int)s.pics.size(), u8[0] ? "; the dump: " : "", u8);
    return (int)s.pics.size();
}
