/* ---------------------------------------------------------------------------
 * The modern frame (F7): the precompile gate with the launcher's own look.
 * Charcoal gradient (or the player's cover art, when assets\launcher-bg.jpg is
 * there), Cinzel from the exe's resources, the Ascension palette, Spanish text.
 * Drawn at the window's resolution with GDI, so the serif is smooth instead of
 * the old 640x360 pixel canvas.
 * -----------------------------------------------------------------------*/

#include <objbase.h>
#include <wincodec.h>

static unsigned ls_rgb(unsigned c) { return RGB((c >> 16) & 255, (c >> 8) & 255, c & 255); }

/* --- fonts, Cinzel from app.rc (103) --------------------------------------- */
static wchar_t s_ls_fam[64] = L"Cinzel";
static int s_ls_fam_loaded;
static HFONT s_ls_xl, s_ls_l, s_ls_m, s_ls_b, s_ls_s, s_ls_sb;
static int s_ls_k;

static void ls_load_cinzel(void)
{
    if (s_ls_fam_loaded) return;
    s_ls_fam_loaded = 1;
    HRSRC r = FindResourceW(NULL, MAKEINTRESOURCEW(103), MAKEINTRESOURCEW(10));
    if (!r) { wcscpy(s_ls_fam, L"Georgia"); return; }
    HGLOBAL g = LoadResource(NULL, r);
    const void* p = g ? LockResource(g) : NULL;
    const DWORD n = SizeofResource(NULL, r);
    DWORD count = 0;
    if (!p || !n || !AddFontMemResourceEx((PVOID)p, n, NULL, &count) || !count) wcscpy(s_ls_fam, L"Georgia");
}

static HFONT ls_mkfont(int px, int weight, const wchar_t* fam)
{
    return CreateFontW(-px, 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH, fam);
}

static void ls_fonts(float k)
{
    const int kk = (int)(k * 1000.0f + 0.5f);
    if (kk == s_ls_k && s_ls_xl) return;
    s_ls_k = kk;
    if (s_ls_xl) DeleteObject(s_ls_xl);
    if (s_ls_l) DeleteObject(s_ls_l);
    if (s_ls_m) DeleteObject(s_ls_m);
    if (s_ls_b) DeleteObject(s_ls_b);
    if (s_ls_s) DeleteObject(s_ls_s);
    if (s_ls_sb) DeleteObject(s_ls_sb);
    ls_load_cinzel();
    s_ls_xl = ls_mkfont((int)(60.0f * k), FW_BOLD, s_ls_fam);
    s_ls_l = ls_mkfont((int)(26.0f * k), FW_BOLD, s_ls_fam);
    s_ls_m = ls_mkfont((int)(20.0f * k), FW_SEMIBOLD, s_ls_fam);
    s_ls_b = ls_mkfont((int)(15.0f * k), FW_NORMAL, L"Segoe UI");
    s_ls_sb = ls_mkfont((int)(15.0f * k), FW_SEMIBOLD, L"Segoe UI");
    s_ls_s = ls_mkfont((int)(12.5f * k), FW_NORMAL, L"Segoe UI");
}

/* --- the cover art, if the player put it beside the package ---------------- */
static std::vector<unsigned char> s_ls_art;   /* 32bpp PBGRA, top-down */
static int s_ls_aw, s_ls_ah, s_ls_art_tried;

static void ls_art_load(void)
{
    if (s_ls_art_tried) return;
    s_ls_art_tried = 1;
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    wchar_t dir[MAX_PATH];
    if (!GetModuleFileNameW(NULL, dir, MAX_PATH)) return;
    if (wchar_t* e = wcsrchr(dir, L'\\')) *e = 0;
    static const wchar_t* stems[3] = { L"\\assets\\", L"\\..\\..\\..\\assets\\", L"\\..\\assets\\" };
    static const wchar_t* files[2] = { L"launcher-bg.jpg", L"launcher-bg.png" };
    IWICImagingFactory* factory = NULL;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) || !factory)
        return;
    for (int b = 0; b < 3 && s_ls_art.empty(); b++)
        for (int i = 0; i < 2 && s_ls_art.empty(); i++) {
            wchar_t path[MAX_PATH];
            swprintf(path, MAX_PATH, L"%ls%ls%ls", dir, stems[b], files[i]);
            if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) continue;
            IWICBitmapDecoder* dec = NULL;
            IWICBitmapFrameDecode* fr = NULL;
            IWICFormatConverter* cv = NULL;
            if (SUCCEEDED(factory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &dec)) &&
                SUCCEEDED(dec->GetFrame(0, &fr)) && SUCCEEDED(factory->CreateFormatConverter(&cv)) &&
                SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0,
                                         WICBitmapPaletteTypeMedianCut))) {
                UINT w = 0, h = 0;
                if (SUCCEEDED(cv->GetSize(&w, &h)) && w && h && w < 8192 && h < 8192) {
                    std::vector<unsigned char> buf((size_t)w * h * 4);
                    const UINT stride = w * 4;
                    if (SUCCEEDED(cv->CopyPixels(NULL, stride, (UINT)buf.size(), buf.data()))) {
                        s_ls_art.swap(buf);
                        s_ls_aw = (int)w;
                        s_ls_ah = (int)h;
                    }
                }
            }
            if (cv) cv->Release();
            if (fr) fr->Release();
            if (dec) dec->Release();
        }
    factory->Release();
    if (s_ls_art.empty()) s_ls_aw = s_ls_ah = 0;
}

/* --- the background, baked once per window size ---------------------------- */
static HDC s_ls_bgdc;
static HBITMAP s_ls_bgbm;
static void* s_ls_bgbits;
static int s_ls_bgw, s_ls_bgh;

static void ls_bg_release(void)
{
    if (s_ls_bgdc) { DeleteDC(s_ls_bgdc); s_ls_bgdc = NULL; }
    if (s_ls_bgbm) { DeleteObject(s_ls_bgbm); s_ls_bgbm = NULL; }
    s_ls_bgbits = NULL;
    s_ls_bgw = s_ls_bgh = 0;
}

static float ls_clamp01(float v) { return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v; }

static void ls_bg_build(int w, int h)
{
    if (s_ls_bgdc && w == s_ls_bgw && h == s_ls_bgh) return;
    ls_bg_release();
    ls_art_load();
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;   /* top down */
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    s_ls_bgbm = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &s_ls_bgbits, NULL, 0);
    if (!s_ls_bgbm) return;
    s_ls_bgdc = CreateCompatibleDC(NULL);
    SelectObject(s_ls_bgdc, s_ls_bgbm);
    s_ls_bgw = w;
    s_ls_bgh = h;
    unsigned char* px = (unsigned char*)s_ls_bgbits;

    const int have_art = !s_ls_art.empty();
    for (int y = 0; y < h; y++) {
        const float u = (float)y / (float)(h > 1 ? h - 1 : 1);
        for (int x = 0; x < w; x++) {
            float r, g, b;
            if (have_art) {
                /* cover: fill the window, crop the spill */
                const float scale = std::max((float)w / (float)s_ls_aw, (float)h / (float)s_ls_ah);
                const float sw = (float)s_ls_aw * scale, sh = (float)s_ls_ah * scale;
                const float sx = ((float)x + 0.5f - ((float)w - sw) * 0.5f) / scale;
                const float sy = ((float)y + 0.5f - ((float)h - sh) * 0.5f) / scale;
                float fx = sx < 0.0f ? 0.0f : sx > (float)(s_ls_aw - 1) ? (float)(s_ls_aw - 1) : sx;
                float fy = sy < 0.0f ? 0.0f : sy > (float)(s_ls_ah - 1) ? (float)(s_ls_ah - 1) : sy;
                const int x0 = (int)fx, y0 = (int)fy;
                const int x1 = x0 + 1 < s_ls_aw ? x0 + 1 : x0, y1 = y0 + 1 < s_ls_ah ? y0 + 1 : y0;
                const float tx = fx - x0, ty = fy - y0;
                const unsigned char* p00 = s_ls_art.data() + ((size_t)y0 * s_ls_aw + x0) * 4;
                const unsigned char* p10 = s_ls_art.data() + ((size_t)y0 * s_ls_aw + x1) * 4;
                const unsigned char* p01 = s_ls_art.data() + ((size_t)y1 * s_ls_aw + x0) * 4;
                const unsigned char* p11 = s_ls_art.data() + ((size_t)y1 * s_ls_aw + x1) * 4;
                const float a = ((1 - tx) * (1 - ty) * p00[3] + tx * (1 - ty) * p10[3] + (1 - tx) * ty * p01[3] +
                                 tx * ty * p11[3]) / 255.0f;
                b = ((1 - tx) * (1 - ty) * p00[0] + tx * (1 - ty) * p10[0] + (1 - tx) * ty * p01[0] + tx * ty * p11[0]);
                g = ((1 - tx) * (1 - ty) * p00[1] + tx * (1 - ty) * p10[1] + (1 - tx) * ty * p01[1] + tx * ty * p11[1]);
                r = ((1 - tx) * (1 - ty) * p00[2] + tx * (1 - ty) * p10[2] + (1 - tx) * ty * p01[2] + tx * ty * p11[2]);
                /* the art is premultiplied: put it over the charcoal */
                r += 13.0f * (1 - a);
                g += 15.0f * (1 - a);
                b += 18.0f * (1 - a);
                /* wash, top fade, right fade, bottom scrim: the UI reads over any art */
                float mult = 1.0f - 0.16f;
                mult *= 1.0f - 0.35f * ls_clamp01((170.0f - (float)y) / 170.0f);
                mult *= 1.0f - 0.40f * ls_clamp01(((float)x - ((float)w - 720.0f)) / 720.0f);
                const float scrim = 0.85f * ls_clamp01((u - 0.42f) / 0.58f);
                r *= mult * (1 - scrim);
                g *= mult * (1 - scrim);
                b *= mult * (1 - scrim);
            } else {
                /* charcoal to deep crimson, a soft warm glow low on the right */
                const float t = u;
                if (t < 0.55f) {
                    const float v = t / 0.55f;
                    r = 13 + (20 - 13) * v; g = 15 + 16 * v; b = 18 + 20 * v;
                } else {
                    const float v = (t - 0.55f) / 0.45f;
                    r = 20 + (58 - 20) * v; g = 16 + 2 * v; b = 20 - 4 * v;
                }
                const float dx = ((float)x / w - 0.78f) * 1.4f, dy = ((float)y / h - 0.86f);
                const float glow = ls_clamp01(1.0f - (dx * dx + dy * dy) * 2.2f);
                r += 70.0f * glow; g += 34.0f * glow; b += 18.0f * glow;
            }
            /* vignette */
            const float vx = ((float)x / w - 0.5f) * 2.0f, vy = ((float)y / h - 0.5f) * 2.0f;
            const float d = sqrtf(vx * vx + vy * vy) / 1.4142f;
            const float dark = 1.0f - 0.38f * d * d;
            r *= dark; g *= dark; b *= dark;
            const size_t o = ((size_t)y * w + x) * 4;
            px[o + 0] = (unsigned char)(b < 0 ? 0 : b > 255 ? 255 : b);
            px[o + 1] = (unsigned char)(g < 0 ? 0 : g > 255 ? 255 : g);
            px[o + 2] = (unsigned char)(r < 0 ? 0 : r > 255 ? 255 : r);
            px[o + 3] = 255;
        }
    }
    /* still embers, as the launcher's fallback has */
    for (int i = 0; i < 260; i++) {
        unsigned hsh = (unsigned)(i * 2654435761u);
        const int x = (int)(hsh % (unsigned)w);
        hsh = (unsigned)(i * 40503u + 7u) * 2246822519u;
        const int y = (int)(hsh % (unsigned)h);
        const float a = 0.10f + 0.35f * (float)((i * 53) % 100) / 100.0f;
        const size_t o = ((size_t)y * w + x) * 4;
        px[o + 0] = (unsigned char)(px[o + 0] * (1 - a) + 0x29 * a);
        px[o + 1] = (unsigned char)(px[o + 1] * (1 - a) + 0xA9 * a);
        px[o + 2] = (unsigned char)(px[o + 2] * (1 - a) + 0xF2 * a);
    }
}

/* --- text and shapes -------------------------------------------------------- */
static int ls_text_w(HDC dc, HFONT f, const wchar_t* s)
{
    HGDIOBJ old = SelectObject(dc, f);
    SIZE sz = {};
    GetTextExtentPoint32W(dc, s, (int)wcslen(s), &sz);
    SelectObject(dc, old);
    return sz.cx;
}

static void ls_text(HDC dc, int x, int y, HFONT f, unsigned color, const wchar_t* s)
{
    HGDIOBJ old = SelectObject(dc, f);
    SetTextColor(dc, (COLORREF)ls_rgb(color));
    SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, x, y, s, (int)wcslen(s));
    SelectObject(dc, old);
}

static void ls_text_c(HDC dc, int cx, int y, HFONT f, unsigned color, const wchar_t* s)
{
    ls_text(dc, cx - ls_text_w(dc, f, s) / 2, y, f, color, s);
}

static void ls_text_r(HDC dc, int rx, int y, HFONT f, unsigned color, const wchar_t* s)
{
    ls_text(dc, rx - ls_text_w(dc, f, s), y, f, color, s);
}

static void ls_rect(HDC dc, int x0, int y0, int x1, int y1, unsigned c)
{
    RECT r = { x0, y0, x1, y1 };
    HBRUSH b = CreateSolidBrush((COLORREF)ls_rgb(c));
    FillRect(dc, &r, b);
    DeleteObject(b);
}

static void ls_frame(HDC dc, int x0, int y0, int x1, int y1, unsigned c, int t)
{
    ls_rect(dc, x0, y0, x1, y0 + t, c);
    ls_rect(dc, x0, y1 - t, x1, y1, c);
    ls_rect(dc, x0, y0, x0 + t, y1, c);
    ls_rect(dc, x1 - t, y0, x1, y1, c);
}

static void ls_hline(HDC dc, int x0, int x1, int y, unsigned c, int t)
{
    ls_rect(dc, x0, y, x1, y + t, c);
}

/* --- the frame -------------------------------------------------------------- */
static void ls_widen(const char* s, wchar_t* out, int n)
{
    if (!s || !*s) { out[0] = 0; return; }
    MultiByteToWideChar(CP_UTF8, 0, s, -1, out, n);
    out[n - 1] = 0;
}

void ls_draw_modern(HDC dc, int w, int h, const Progress& p, ULONGLONG t)
{
    if (w <= 0 || h <= 0) return;
    ls_fonts((float)w / 1280.0f < (float)h / 720.0f ? (float)w / 1280.0f : (float)h / 720.0f);
    ls_bg_build(w, h);
    if (s_ls_bgdc) BitBlt(dc, 0, 0, w, h, s_ls_bgdc, 0, 0, SRCCOPY);
    else ls_rect(dc, 0, 0, w, h, 0x0D0F12);

    const float k = (float)w / 1280.0f < (float)h / 720.0f ? (float)w / 1280.0f : (float)h / 720.0f;
    const int ox = (w - (int)(1280.0f * k)) / 2, oy = (h - (int)(720.0f * k)) / 2;
    const int X = (int)(1280.0f * k), Y = (int)(720.0f * k);
    (void)X; (void)Y;
    const unsigned cream = 0xEAE6DF, bone = 0xEDE7E1, muted = 0x8C929A, dim = 0x7D838B;
    const unsigned gold = 0xD97724, gold_hi = 0xF2A93B, blood = 0x9E1B1E, red = 0xD94A46;
    const int blink = (int)((t / 380) & 1);

    /* monogram */
    {
        const int bx = ox + (int)(48 * k), by = oy + (int)(40 * k), bs = (int)(58 * k);
        ls_rect(dc, bx, by, bx + bs, by + bs, 0x141A20);
        ls_frame(dc, bx, by, bx + bs, by + bs, gold, (int)std::max(1.0f, 2.0f * k));
        ls_text_c(dc, bx + bs / 2, by + (int)(bs * 0.18f), s_ls_m, gold_hi, L"Gc");
    }

    /* the wordmark, in Cinzel, beside the monogram */
    {
        wchar_t a[8] = L"GOWA", b[8] = L"-cMc";
        const int x = ox + (int)(120 * k), y = oy + (int)(44 * k);
        ls_text(dc, x, y, s_ls_l, bone, a);
        ls_text(dc, x + ls_text_w(dc, s_ls_l, a), y, s_ls_l, red, b);
        ls_text(dc, x, y + (int)(36 * k), s_ls_s, muted, L"GOD OF WAR: ASCENSION   \x00B7   BCUS98232");
    }

    /* the phase */
    wchar_t title[96], line[192], hint[192], right[96], pct[24], foot[192];
    title[0] = line[0] = hint[0] = right[0] = foot[0] = pct[0] = 0;
    double frac = 0.0;
    unsigned title_c = gold_hi;
    wchar_t wname[128];
    ls_widen(p.name, wname, 128);
    switch (p.phase) {
    case PH_START:
        wcscpy(title, L"PREPARANDO");
        wcscpy(hint, L"El juego arranca solo cuando esto termina.");
        break;
    case PH_READ:
        wcscpy(title, L"LEYENDO LOS ARCHIVOS DEL JUEGO");
        frac = p.n ? (double)(p.a ? p.a - 1 : 0) / p.n : 0.0;
        if (wname[0]) swprintf(line, 192, L"Archivo %u de %u   \x00B7   %ls", p.a, p.n, wname);
        else swprintf(line, 192, L"Archivo %u de %u", p.a, p.n);
        wcscpy(hint, L"Se lee una vez por versi\x00F3n. El juego arranca solo.");
        break;
    case PH_BUILD: {
        wcscpy(title, L"COMPILANDO SHADERS");
        frac = p.total ? (double)p.done / p.total : 0.0;
        if (p.total) swprintf(line, 192, L"%u de %u shaders", p.done, p.total);
        else swprintf(line, 192, L"%u programas de v\x00E9rtice encontrados", p.found);
        const double ms = (double)(GetTickCount64() - p.t_build);
        if (p.done >= 50 && p.done < p.total && ms > 3000.0) {
            const double left_s = ms / p.done * (p.total - p.done) / 1000.0;
            if (left_s < 60.0) wcscpy(right, L"Queda menos de un minuto");
            else swprintf(right, 96, L"Quedan unos %d min", (int)(left_s / 60.0 + 0.999));
        }
        wcscpy(hint, L"Se compila una vez por versi\x00F3n. El juego arranca solo.");
        break;
    }
    case PH_DONE:
        wcscpy(title, L"SHADERS LISTOS");
        frac = 1.0;
        wcscpy(line, blink ? L"Iniciando el juego..." : L"Iniciando el juego");
        wcscpy(hint, L"Un momento: el juego abre su propia ventana.");
        break;
    case PH_ERROR:
        wcscpy(title, L"NO SE PUDIERON COMPILAR LOS SHADERS");
        title_c = 0xFF6A4A;
        frac = p.total ? (double)p.done / p.total : 0.0;
        {
            wchar_t w[160];
            ls_widen(p.why[0] ? p.why : "La compilaci\x00F3n se detuvo", w, 160);
            swprintf(line, 192, L"%ls", w);
        }
        wcscpy(hint, L"El juego arranca igualmente. El pr\x00F3ximo inicio lo reintenta.");
        break;
    }
    if (!wcscmp(title, L"")) wcscpy(title, L"PREPARANDO");
    if (p.phase == PH_READ || p.phase == PH_BUILD || p.phase == PH_DONE)
        swprintf(pct, 24, L"%d%%", (int)(frac * 100.0 + 1e-9));

    ls_text_c(dc, ox + (int)(640 * k), oy + (int)(740 * k), s_ls_l, title_c, title);

    /* the panel: title inside it, then the archive, the bar and the hints */
    const int px0 = ox + (int)(140 * k), px1 = ox + (int)(1140 * k);
    const int py0 = oy + (int)(452 * k), py1 = oy + (int)(656 * k);
    ls_rect(dc, px0, py0, px1, py1, 0x141A20);
    ls_frame(dc, px0, py0, px1, py1, gold, (int)std::max(1.0f, 2.0f * k));
    ls_hline(dc, px0 + 2, px1 - 2, py0 + (int)(58 * k), 0x2A3038, (int)std::max(1.0f, 1.0f * k));
    ls_text(dc, px0 + (int)(20 * k), py0 + (int)(14 * k), s_ls_l, title_c, title);
    if (pct[0]) ls_text_r(dc, px1 - (int)(20 * k), py0 + (int)(16 * k), s_ls_m, gold_hi, pct);

    /* the line of what is being read / built */
    ls_text(dc, px0 + (int)(20 * k), py0 + (int)(68 * k), s_ls_b, cream, line);
    /* the bar */
    {
        const int bx0 = px0 + (int)(20 * k), bx1 = px1 - (int)(20 * k);
        const int by0 = py0 + (int)(100 * k), by1 = py0 + (int)(126 * k);
        ls_rect(dc, bx0, by0, bx1, by1, 0x0B0F13);
        ls_frame(dc, bx0, by0, bx1, by1, 0x2E3842, (int)std::max(1.0f, 1.0f * k));
        const int fill = (int)((bx1 - bx0 - 4) * frac + 0.5);
        if (fill > 0) {
            const int fy0 = by0 + 2, fy1 = by1 - 2;
            /* crimson to amber along the built part */
            for (int x = 0; x < fill; x++) {
                const float u = fill > 1 ? (float)x / (fill - 1) : 1.0f;
                const int r = (int)(0x9E + (0xD9 - 0x9E) * u);
                const int g = (int)(0x1B + (0x77 - 0x1B) * u);
                const int bb = (int)(0x1E + (0x24 - 0x1E) * u);
                ls_rect(dc, bx0 + 2 + x, fy0, bx0 + 3 + x, fy1, ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)bb);
            }
            ls_rect(dc, bx0 + 2, fy0, bx0 + 2 + fill, fy0 + (int)std::max(1.0f, 1.0f * k), 0xB52326);
        }
    }
    /* hint and right column */
    {
        const int hy = py0 + (int)(138 * k);
        ls_text(dc, px0 + (int)(20 * k), hy, s_ls_s, dim, hint);
        if (right[0]) ls_text_r(dc, px1 - (int)(20 * k), hy, s_ls_s, muted, right);
        swprintf(foot, 192, L"port: cMc-OmniPort");
        ls_text_r(dc, px1 - (int)(20 * k), py1 - (int)(26 * k), s_ls_s, dim, foot);
    }

    /* a thin amber line over the whole frame, as the launcher's chrome */
    ls_hline(dc, 0, w, 0, 0x2A3038, 1);
}

/* --loading-preview: the modern frame at 1280x720, as a BMP */
static int ls_write_bmp(const char* path, int w, int h, const void* bits)
{
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    const int row = w * 3, size = 54 + row * h;
    uint8_t hd[54] = { 'B', 'M' };
    memcpy(hd + 2, &size, 4);
    const int off = 54, dib = 40, planes = 1, bpp = 24;
    memcpy(hd + 10, &off, 4);
    memcpy(hd + 14, &dib, 4);
    memcpy(hd + 18, &w, 4);
    memcpy(hd + 22, &h, 4);
    memcpy(hd + 26, &planes, 2);
    memcpy(hd + 28, &bpp, 2);
    fwrite(hd, 1, 54, f);
    const unsigned char* px = (const unsigned char*)bits;
    std::vector<uint8_t> line((size_t)row);
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            const size_t o = ((size_t)y * w + x) * 4;
            line[(size_t)x * 3] = px[o];
            line[(size_t)x * 3 + 1] = px[o + 1];
            line[(size_t)x * 3 + 2] = px[o + 2];
        }
        fwrite(line.data(), 1, line.size(), f);
    }
    fclose(f);
    return 1;
}

int ls_preview_modern(const char* path, const Progress& p, ULONGLONG t)
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    const int w = 1280, h = 720;
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = NULL;
    HBITMAP bm = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!bm) return 0;
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, bm);
    ls_draw_modern(dc, w, h, p, t);
    const int ok = ls_write_bmp(path, w, h, bits);
    DeleteDC(dc);
    DeleteObject(bm);
    return ok;
}
