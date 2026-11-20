/* New shell, F2b: SpartanPort-style pages over the palette of God of War: Ascension.
 *
 * Palette: deep charcoal #0D0F12, ancient stone #1A1E24, Spartan blood #9E1B1E for fills,
 * a lighter red #D94A46 for text (the crimson alone would not read on the dark), the gods'
 * amber #D97724 / #F2A93B for attention and the chips, and Delphi water #4A90A4 only for
 * information. The controls are the launcher's own s_opt table: every row reads and writes
 * the same section/key of gowa-cmc.ini the classic launcher does. */

static const unsigned NC_BG = 0x0D0F12;         /* la oscuridad de la prisiÃ³n */
static const unsigned NC_PANEL = 0x1A1E24;      /* piedra antigua */
static const unsigned NC_ACCENT = 0x9E1B1E;     /* la sangre de Esparta: rellenos */
static const unsigned NC_ACCENT_HI = 0xB52326;
static const unsigned NC_ACCENT_DEEP = 0x5E0B0D;
static const unsigned NC_RED = 0xD94A46;        /* el mismo rojo, legible como texto */
static const unsigned NC_GOLD = 0xD97724;       /* el fuego del caos */
static const unsigned NC_GOLD_HI = 0xF2A93B;    /* el oro de los dioses */
static const unsigned NC_INFO = 0x4A90A4;       /* el agua de Delfos: informaciÃ³n */
static const unsigned NC_TEXT = 0xEAE6DF;       /* blanco hueso */
static const unsigned NC_TEXT2 = 0x8C929A;      /* gris ceniza */
static const unsigned NC_DESC = 0x7D838B;
static const unsigned NC_LINE = 0x2A3038;
static const unsigned NC_FIELD = 0x141A20;
static const unsigned NC_OK = 0x7FB069;
static const unsigned NC_BAD = 0xE05252;
static const unsigned NC_WARN = 0xE0A030;

static const float NC_PANEL_W = 604.0f, NC_NAV_H = 64.0f;

enum { H_NEW_NAV = 9100, H_NEW_CTL = 9200, H_NEW_SAVE = 9300, H_NEW_DDO = 9400, H_NEW_ACT = 9500,
       H_NEW_TOOL = 9600, H_NEW_LOGACT = 9700, H_NEW_MODS = 9800, H_NEW_MODS_ACT = 9900, H_NEW_PREBUILD = 9920 };
static int s_new_page = -1;         /* -1: follow s_tab */
static int s_new_dd = -1;           /* the row whose list is open, or -1 */
static int s_new_dirty;             /* a control changed since the last save */
static float s_new_scroll[11];      /* per page */
static float s_new_scroll_max[11];
static D2D1_RECT_F s_new_dd_at;     /* the field the open list hangs from */
static D2D1_RECT_F s_new_row_field[64];   /* where each row draws its field, for the click */
static float s_new_vis_top, s_new_vis_bot;   /* what of the rows is really on screen */
static Opt* s_new_dd_opt;

static void toast(const char* s);   /* defined below in launcher.cpp */

static int new_ui_on(void)
{
    if (getenv("GOWA_UI_NEW")) return 1;
    if (getenv("GOWA_UI_CLASSIC")) return 0;
    /* New by default now; [launcher] ui = classic (or 0/off/old) brings the original back. */
    const char* ui = ps3_settings_str("launcher", "ui", NULL, "new");
    if (!ui || !ui[0]) return 1;
    return !(ps3_settings_ieq(ui, "classic") || ps3_settings_ieq(ui, "old") ||
             ps3_settings_ieq(ui, "0") || ps3_settings_ieq(ui, "off"));
}

static int new_page(void)
{
    if (getenv("GOWA_UI_PAGE")) {
        const int p = atoi(getenv("GOWA_UI_PAGE"));
        if (p >= 0 && p < 11) return p;
    }
    if (s_new_page >= 0) return s_new_page;
    return s_tab == T_SYSTEM ? 7 : (s_tab >= 0 && s_tab < T_SYSTEM ? (int)s_tab : 0);
}

static const wchar_t* new_page_title(int p)
{
    static const wchar_t* keys[11] = { L"nav.home", L"nav.graphics", L"nav.display", L"nav.controls",
                                       L"nav.patches", L"nav.cheats", L"nav.mods", L"nav.system",
                                       L"nav.advanced", L"nav.log", L"nav.trophies" };
    static const wchar_t* fallback[11] = { L"HOME", L"GR\x00C1" L"FICOS", L"PANTALLA", L"CONTROLES",
                                          L"PARCHES", L"CHEATS", L"MODS", L"SISTEMA", L"AVANZADO", L"LOG", L"TROFEOS" };
    const int i = p >= 0 && p < 11 ? p : 0;
    return i18n_tr(keys[i], fallback[i]);
}

static const wchar_t* new_page_sub(int p)
{
    static const wchar_t* keys[11] = {
        L"page.home.sub", L"page.graphics.sub", L"page.display.sub", L"page.controls.sub",
        L"page.patches.sub", L"page.cheats.sub", L"page.mods.sub", L"page.system.sub",
        L"page.advanced.sub", L"page.log.sub", L"page.trophies.sub"
    };
    static const wchar_t* fallback[11] = {
        L"",
        L"Ajustes de imagen. Se guardan en ascension-r.ini; el juego lee los mismos.",
        L"Ventana, sincronizaci\x00F3n y l\x00EDmite de fotogramas.",
        L"Teclado, mando y zona muerta.",
        L"Parches del juego que se aplican al arrancar.",
        L"Trampas: da\x00F1o, orbes y magia.",
        L"Mods: carpetas y .zip; orden y estado.",
        L"Rutas, audio, guardados e idioma.",
        L"Versi\x00F3n, cr\x00E9" L"ditos y herramientas del paquete.",
        L"\x00DA" L"ltimas l\x00EDneas del registro de la build m\x00E1s reciente.",
        L"Conjunto de trofeos del juego: iconos del TRP y su carpeta.",
    };
    const int i = p >= 0 && p < 11 ? p : 0;
    return i18n_tr(keys[i], fallback[i]);
}

/* The Spanish of the option table; anything not here falls back to the launcher's English. */
struct NewEs { const char* section; const char* key; const wchar_t* label; const wchar_t* desc; };
static const NewEs k_es[] = {
    { "display", "mode", L"Ventana", L"Sin bordes cubre toda la pantalla." },
    { "display", "window_size", L"Tama\x00F1o de ventana",
      L"En modo ventana. El juego dibuja a 720p; la imagen se escala a la ventana." },
    { "display", "vsync", L"VSync", L"Espera al refresco del monitor: sin tearing." },
    { "display", "frame_cap", L"L\x00EDmite de FPS",
      L"Ritmo estable en vez de la tasa m\x00E1xima que alcance el PC. Con generaci\x00F3n de "
      L"fotogramas, sin l\x00EDmite sigue al monitor." },
    { "display", "show_fps", L"Mostrar FPS",
      L"En pantalla muestra FPS, tiempo de fotograma y carga de CPU, GPU, PPU, SPU y RSX." },
    { "display", "ultrawide", L"Ultrawide",
      L"Dibuja el juego a todo el ancho de una pantalla m\x00E1s ancha. El HUD, los men\x00FAs y "
      L"las cinem\x00E1ticas siguen en 16:9. Se aplica al siguiente arranque." },
    { "display", "shader_hint", L"Indicador de shaders",
      L"Un spinner abajo a la izquierda mientras se compilan shaders nuevos, como en RPCS3." },
    { "display", "effects", L"Efectos retro",
      L"Scanlines y glow ne\x00F3n en esta ventana. El juego no cambia." },
    { "graphics", "gpu", L"Tarjeta gr\x00E1" L"fica", L"" },
    { "graphics", "vibrance", L"Vibrance del color", L"Fuerza de la vibrance en la imagen final." },
    { "graphics", "contrast", L"Contraste", L"Fuerza del contraste en la imagen final." },
    { "graphics", "shader_prebuild", L"Pixel shaders anticipados", L"Compila los pixel shaders antes de jugar." },
    { "graphics", "render_scale", L"Resoluci\x00F3n interna",
      L"A la que se dibuja la escena. El bloom, el desenfoque y las sombras quedan al tama\x00F1o del "
      L"juego. FSR 3.1 y la generaci\x00F3n de fotogramas no pasan de la ventana." },
    { "graphics", "scaling", L"Escalado",
      L"C\x00F3mo se escala la imagen a la ventana. FSR 3.1 crea detalle a partir de fotogramas "
      L"pasados, en lugar de SMAA, y FSR 1 toma el relevo cuando no hay escena." },
    { "graphics", "sharpening", L"Nitidez FSR",
      L"Paso de nitidez de FSR (RCAS). Solo con escalado FSR." },
    { "graphics", "frame_generation", L"Generaci\x00F3n de fotogramas",
      L"FSR 3.1: el doble de fotogramas en pantalla, con VSync. El juego se limita a la mitad del "
      L"refresco del monitor, 30 a 60 Hz y 60 a 120 Hz. Se crea un fotograma entre cada dos, lo que "
      L"retrasa la imagen cerca de un fotograma, y por debajo de 25 se aparta." },
    { "graphics", "antialiasing", L"Antialiasing",
      L"Suaviza bordes dentados despu\x00E9s de que el juego dibuje el fotograma. Casi no cuesta. "
      L"El escalado FSR 3.1 ya lo hace en partida." },
    { "graphics", "anisotropy", L"Filtrado anisotr\x00F3pico",
      L"Texturas m\x00E1s n\x00ED" L"tidas en \x00E1ngulo. El juego no pide ninguno." },
    { "graphics", "async_shaders", L"Compilar shaders en segundo plano",
      L"Un shader nuevo se compila mientras el juego sigue, en vez de congelarse." },
    { "graphics", "render_thread", L"Hilo de render",
      L"El trabajo de la GPU se graba y presenta en un hilo propio, junto al que lee los comandos "
      L"del juego: sobre un 10 % m\x00E1s de fotogramas donde ese hilo frena al juego." },
    { "graphics", "pipeline_wait", L"Construir pipelines primero",
      L"Al arrancar, el juego espera a que los pipelines guardados de sesiones anteriores est\x00E9n "
      L"listos, para que ninguno se construya mientras juegas: un momento, unos segundos tras "
      L"actualizar el driver." },
    { "input", "device", L"Dispositivo",
      L"Autom\x00E1tico toma un mando si lo hay; si no, el teclado y el rat\x00F3n." },
    { "input", "api", L"API del mando",
      L"Autom\x00E1tico lee el mando por XInput cuando lo tiene (Xbox y la mayor\x00EDa de mandos de "
      L"PC); si no, por SDL3 (DualShock 4, DualSense, Switch y otros). Con DS4Windows, XInput o un "
      L"mando cuenta doble." },
    { "input", "deadzone", L"Zona muerta del stick",
      L"Cu\x00E1nto tiene que moverse un stick para que cuente." },
    { "input", "enter_button", L"Bot\x00F3n de confirmar", L"El bot\x00F3n que confirma en los men\x00FAs." },
    { "input", "vibration", L"Vibraci\x00F3n", L"Vibraci\x00F3n del mando." },
    { "patches", "disable_mlaa", L"Sin MLAA",
      L"El antialiasing de PS3 por SPU, cuyo resultado este port nunca muestra: unos 5 % m\x00E1s r\x00E1pido. "
      L"El bloom se va con \x00E9l. FSR 3.1 y la generaci\x00F3n de fotogramas necesitan esto activado." },
    { "patches", "disable_depth_of_field", L"Sin profundidad de campo",
      L"El desenfoque de lo que queda fuera del foco de la c\x00E1mara." },
    { "patches", "disable_motion_blur", L"Sin motion blur",
      L"Ahorra el trabajo del desenfoque de movimiento. Con Sin MLAA activado nunca llega a pantalla. "
      L"FSR 3.1 y la generaci\x00F3n de fotogramas mantienen sus draws, para lo que se mueve." },
    { "patches", "disable_mesh_trimming", L"Sin recorte de malla",
      L"Conserva los tri\x00E1ngulos que el juego descarta por finos para su rejilla de 720p. Para "
      L"resoluci\x00F3n interna por encima de 720p: a 720p solo a\x00F1" L"ade geometr\x00ED" L"a (3 % m\x00E1s "
      L"lento en el primer nivel, 11 % en la puerta)." },
    { "patches", "disable_shadows", L"Sin sombras",
      L"Sin sombras din\x00E1micas: el mayor ahorro, unos 25 % m\x00E1s r\x00E1pido en la pelea con Megaera." },
    { "cheats", "damage_taken", L"Da\x00F1o recibido por Kratos",
      L"Cu\x00E1nto da\x00F1o le entra. Ninguno: los golpes no le quitan salud." },
    { "cheats", "damage_dealt", L"Da\x00F1o de Kratos",
      L"Cu\x00E1nto quitan sus golpes a un enemigo. Un golpe: toda su salud." },
    { "cheats", "max_orbs", L"Orbes rojos al m\x00E1ximo",
      L"Los orbes rojos se mantienen en el m\x00E1ximo que permite el juego, 1.000.000. Una partida guardada "
      L"con esto activo los conserva." },
    { "cheats", "infinite_magic", L"Magia y objetos infinitos",
      L"Los medidores de magia y objetos se quedan llenos." },
    { "cheats", "infinite_rage", L"Rabia infinita", L"El medidor de rabia se queda lleno." },
    { "cheats", "infinite_jumps", L"Saltos dobles infinitos",
      L"Kratos puede volver a saltar en el aire, tantas veces como quiera." },
    { "system", "language", L"Idioma", L"El idioma del juego: textos y voces." },
    { "system", "esc_quits", L"Esc cierra el juego",
      L"Esc no es un bot\x00F3n de PS3; desactivado lo deja libre." },
    { "system", "save_backups", L"Copias de guardado",
      L"Copias que se guardan de una partida antes de que el juego la sobrescriba." },
    { "system", "steady_audio", L"Audio estable",
      L"Ejecuta el sonido del juego en un hilo propio, para que las peleas duras no lo corten." },
    { "system", "speakers", L"Altavoces",
      L"Autom\x00E1tico sigue el dispositivo de salida; 7.1 para Windows Sonic o Dolby Atmos para "
      L"auriculares. El surround tambi\x00E9n necesita que el dispositivo lo soporte." },
    { "system", "raw_audio", L"Omitir efectos de sonido",
      L"El sonido del juego salta los efectos de Windows y de la tarjeta (ecualizadores, sonoridad, sonido virtual)." },
    { "system", "precompile_music", L"Precompilar m\x00FAsica",
      L"M\x00FAsica en la pantalla de compilaci\x00F3n de shaders en el primer arranque de una "
      L"versi\x00F3n, hecha por el programa." },
    { "launcher", "show", L"Mostrar esta ventana al iniciar",
      L"Si est\x00E1 desactivado, mant\x00E9n Shift al arrancar el juego para recuperarla." },
    { "launcher", "update_check", L"Buscar actualizaciones",
      L"Como mucho cada seis horas, pregunta a GitHub si hay una versi\x00F3n nueva y lo indica "
      L"aqu\x00ED con un enlace." },
};
/* The values of the lists, in Spanish. Anything not here keeps the launcher's own name
 * (a GPU's model, a resolution, FSR, XInput: things that read the same in both). */
static const wchar_t* new_val_es(const wchar_t* en)
{
    static const struct { const wchar_t* en; const wchar_t* es; } t[] = {
        { L"Windowed", L"Ventana" },
        { L"Borderless fullscreen", L"Sin bordes (pantalla completa)" },
        { L"Borderless", L"Sin bordes" },
        { L"Default (720p x Windows scale)", L"Autom\x00E1tico (720p x escala de Windows)" },
        { L"Off", L"Desactivado" },
        { L"On", L"Activado" },
        { L"In the title bar", L"En la barra de t\x00ED" L"tulo" },
        { L"On screen, with CPU/GPU load", L"En pantalla, con carga de CPU/GPU" },
        { L"720p, as the game draws it", L"720p, como lo dibuja el juego" },
        { L"Bilinear", L"Bilineal" },
        { L"Game", L"Del juego" },
        { L"Low", L"Baja" },
        { L"Medium", L"Media" },
        { L"High", L"Alta" },
        { L"High (AMD's)", L"Alta (AMD)" },
        { L"Auto", L"Autom\x00E1tico" },
        { L"Keyboard", L"Teclado" },
        { L"Gamepad", L"Mando" },
        { L"Discrete", L"Dedicada" },
        { L"Integrated", L"Integrada" },
        { L"Cross", L"Cruz" },
        { L"Circle", L"C\x00ED" L"rculo" },
        { L"Square", L"Cuadrado" },
        { L"Triangle", L"Tri\x00E1ngulo" },
        { L"None", L"Ninguno" },
        { L"English", L"Ingl\x00E9s" },
        { L"Spanish", L"Espa\x00F1ol" },
        { L"Portuguese (Brazil)", L"Portugu\x00E9s (Brasil)" },
        { L"French", L"Franc\x00E9s" },
        { L"German", L"Alem\x00E1n" },
        { L"Italian", L"Italiano" },
        { L"Japanese", L"Japon\x00E9s" },
        { L"Polish", L"Polaco" },
        { L"Russian", L"Ruso" },
        { L"Stereo", L"Est\x00E9reo" },
        { L"Headphones", L"Auriculares" },
        { L"Half", L"Mitad" },
        { L"Quarter", L"Cuarto" },
        { L"One hit", L"Un golpe" },
        { L"Controller, else keyboard", L"Mando; si no, teclado" },
        { L"Controller only", L"Solo mando" },
        { L"Keyboard only", L"Solo teclado" },
        { L"XInput, else SDL3", L"XInput; si no, SDL3" },
        { L"XInput only", L"Solo XInput" },
        { L"SDL3 for every controller", L"SDL3 para todos los mandos" },
        { L"24% (XInput's)", L"24 % (el de XInput)" },
        { L"Keep 3", L"Guardar 3" },
        { L"Keep 5", L"Guardar 5" },
        { L"Keep 10", L"Guardar 10" },
        { L"As Windows has the output device", L"Como lo tenga Windows" },
        { L"Windows language", L"Idioma de Windows" },
        { L"English (US)", L"Ingl\x00E9s (EE. UU.)" },
        { L"English (UK)", L"Ingl\x00E9s (Reino Unido)" },
        { L"Dutch", L"Neerland\x00E9s" },
        { L"Korean", L"Coreano" },
        { L"Chinese (traditional)", L"Chino (tradicional)" },
        { L"Chinese (simplified)", L"Chino (simplificado)" },
        { L"Turkish", L"Turco" },
        { L"Portuguese", L"Portugu\x00E9s" },
    };
    for (size_t i = 0; i < sizeof t / sizeof t[0]; i++)
        if (!wcscmp(en, t[i].en)) return t[i].es;
    return en;
}

static const NewEs* new_es(const Opt* o)
{
    for (size_t i = 0; i < sizeof k_es / sizeof k_es[0]; i++)
        if (!strcmp(o->section, k_es[i].section) && !strcmp(o->key, k_es[i].key)) return &k_es[i];
    return NULL;
}

/* How many option rows a page has; F3 gives PATCHES, CHEATS, SYSTEM and ADVANCED theirs. */
static int new_page_opts(int p, Opt** out, int max)
{
    if (p == 6) return 0;                 /* MODS is not an option list */
    int n = 0;
    if (p == T_PATCHES) {
        for (int i = 0; i < s_npopt && n < max; i++) out[n++] = &s_popt[i];
        return n;
    }
    const int tab = (p == 7) ? T_SYSTEM : p;   /* SYSTEM moved to page 7 */
    if (tab < 1 || tab > T_SYSTEM) return 0;
    for (int i = 0; i < N_OPT && n < max; i++)
        if (s_opt[i].tab == tab) out[n++] = &s_opt[i];
    return n;
}

static int new_opt_index(const Opt* o)
{
    if (!o->values) return 0;
    for (int i = 0; i < o->n; i++)
        if (ps3_settings_ieq(o->cur, o->values[i])) return i;
    return 0;
}

/* A fade of one colour, from a to b, over a rectangle. */
static void new_fade(D2D1_POINT_2F p0, D2D1_POINT_2F p1, unsigned rgb, float a0, float a1, D2D1_RECT_F area)
{
    D2D1_GRADIENT_STOP gs[2] = {
        { 0.0f, D2D1::ColorF(rgb, a0) },
        { 1.0f, D2D1::ColorF(rgb, a1) },
    };
    ID2D1GradientStopCollection* stops = NULL;
    if (FAILED(s_d->CreateGradientStopCollection(gs, 2, &stops))) return;
    const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lp = D2D1::LinearGradientBrushProperties(p0, p1);
    ID2D1LinearGradientBrush* br = NULL;
    if (SUCCEEDED(s_d->CreateLinearGradientBrush(lp, stops, &br))) {
        s_d->FillRectangle(area, br);
        br->Release();
    }
    stops->Release();
}

/* A vertical gradient of two colours, for the armour-like fills. */
static void new_grad2(D2D1_RECT_F r, unsigned c0, unsigned c1)
{
    D2D1_GRADIENT_STOP gs[2] = {
        { 0.0f, D2D1::ColorF(c0, 1.0f) },
        { 1.0f, D2D1::ColorF(c1, 1.0f) },
    };
    ID2D1GradientStopCollection* stops = NULL;
    if (FAILED(s_d->CreateGradientStopCollection(gs, 2, &stops))) return;
    const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lp =
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, r.top), D2D1::Point2F(0, r.bottom));
    ID2D1LinearGradientBrush* br = NULL;
    if (SUCCEEDED(s_d->CreateLinearGradientBrush(lp, stops, &br))) {
        s_d->FillRectangle(r, br);
        br->Release();
    }
    stops->Release();
}

/* The user's own cover art, beside the exe: assets\launcher-bg.jpg or .png. It is never shipped
 * inside the package (that art is Santa Monica Studio's); when it is missing the procedural
 * background below is used instead. */
static ID2D1Bitmap* s_new_art;
static int s_new_art_tried;

static void new_art_load(void)
{
    if (s_new_art || s_new_art_tried) return;
    s_new_art_tried = 1;
    if (!s_wic || !s_rt) return;
    wchar_t dir[GAME_PATH_MAX];
    if (!GetModuleFileNameW(NULL, dir, GAME_PATH_MAX)) return;
    if (wchar_t* e = wcsrchr(dir, L'\\')) *e = 0;
    /* The exe may sit in build: try its own folder, the package root and the repo root. */
    static const wchar_t* stems[3] = { L"\\assets\\", L"\\..\\..\\..\\assets\\", L"\\..\\assets\\" };
    static const wchar_t* files[2] = { L"launcher-bg.jpg", L"launcher-bg.png" };
    for (int b = 0; b < 3 && !s_new_art; b++)
        for (int i = 0; i < 2 && !s_new_art; i++) {
            wchar_t path[GAME_PATH_MAX];
            swprintf(path, GAME_PATH_MAX, L"%ls%ls%ls", dir, stems[b], files[i]);
            if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) continue;
            IWICBitmapDecoder* dec = NULL;
            IWICBitmapFrameDecode* frame = NULL;
            IWICFormatConverter* cv = NULL;
            if (SUCCEEDED(s_wic->CreateDecoderFromFilename(path, NULL, GENERIC_READ,
                                                           WICDecodeMetadataCacheOnDemand, &dec)) &&
                SUCCEEDED(dec->GetFrame(0, &frame)) && SUCCEEDED(s_wic->CreateFormatConverter(&cv)) &&
                SUCCEEDED(cv->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0,
                                        WICBitmapPaletteTypeMedianCut)))
                s_rt->CreateBitmapFromWicBitmap(cv, NULL, &s_new_art);
            if (cv) cv->Release();
            if (frame) frame->Release();
            if (dec) dec->Release();
        }
}

static ID2D1Bitmap* s_new_logo;
static int s_new_logo_tried;

static void new_logo_load(void)
{
    if (s_new_logo || s_new_logo_tried) return;
    s_new_logo_tried = 1;
    if (!s_wic || !s_rt) return;
    wchar_t dir[GAME_PATH_MAX];
    if (!GetModuleFileNameW(NULL, dir, GAME_PATH_MAX)) return;
    if (wchar_t* e = wcsrchr(dir, L'\\')) *e = 0;
    static const wchar_t* stems[3] = { L"\\assets\\", L"\\..\\..\\..\\assets\\", L"\\..\\assets\\" };
    for (int b = 0; b < 3 && !s_new_logo; b++) {
        wchar_t path[GAME_PATH_MAX];
        swprintf(path, GAME_PATH_MAX, L"%ls%lslauncher-logo.png", dir, stems[b]);
        if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) continue;
        IWICBitmapDecoder* dec = NULL;
        IWICBitmapFrameDecode* frame = NULL;
        IWICFormatConverter* cv = NULL;
        if (SUCCEEDED(s_wic->CreateDecoderFromFilename(path, NULL, GENERIC_READ,
                                                       WICDecodeMetadataCacheOnDemand, &dec)) &&
            SUCCEEDED(dec->GetFrame(0, &frame)) && SUCCEEDED(s_wic->CreateFormatConverter(&cv)) &&
            SUCCEEDED(cv->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0,
                                    WICBitmapPaletteTypeMedianCut)))
            s_rt->CreateBitmapFromWicBitmap(cv, NULL, &s_new_logo);
        if (cv) cv->Release();
        if (frame) frame->Release();
        if (dec) dec->Release();
    }
}

static void new_background(void)
{
    fill(R(0, 0, WIN_W, WIN_H), NC_BG);
    new_art_load();
    if (s_new_art) {
        const D2D1_SIZE_U sz = s_new_art->GetPixelSize();
        if (sz.width && sz.height) {
            const float ax = WIN_W / (float)sz.width, ay = WIN_H / (float)sz.height;
            const float scale = ax > ay ? ax : ay;   /* cover: fill the window, crop what spills */
            const float w = sz.width * scale, h = sz.height * scale;
            s_d->DrawBitmap(s_new_art, R((WIN_W - w) * 0.5f, (WIN_H - h) * 0.5f, w, h), 1.0f,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        }
        /* A wash and the scrims: the launcher's names and buttons read over any art. */
        fill(R(0, 0, WIN_W, WIN_H), NC_BG, 0.16f);
        new_fade(D2D1::Point2F(0, WIN_H - 430), D2D1::Point2F(0, WIN_H), 0x000000, 0.0f, 0.85f,
                 R(0, WIN_H - 430, WIN_W, 430));
        new_fade(D2D1::Point2F(WIN_W - 720, 0), D2D1::Point2F(WIN_W, 0), 0x000000, 0.0f, 0.40f,
                 R(WIN_W - 720, 0, 720, WIN_H));
        new_fade(D2D1::Point2F(0, 0), D2D1::Point2F(0, 170), 0x000000, 0.35f, 0.0f, R(0, 0, WIN_W, 170));
        new_fade(D2D1::Point2F(0, 0), D2D1::Point2F(300, 0), 0x000000, 0.50f, 0.0f, R(0, 0, 300, WIN_H));
    } else {
        const D2D1_GRADIENT_STOP gs[3] = {
            { 0.00f, D2D1::ColorF(0x0D0F12, 1.0f) },
            { 0.55f, D2D1::ColorF(0x141014, 1.0f) },
            { 1.00f, D2D1::ColorF(0x3A1210, 1.0f) },
        };
        ID2D1GradientStopCollection* stops = NULL;
        if (SUCCEEDED(s_d->CreateGradientStopCollection(gs, 3, &stops))) {
            const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lp = D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(WIN_W * 0.82f, 0.0f), D2D1::Point2F(WIN_W * 0.45f, WIN_H));
            ID2D1LinearGradientBrush* br = NULL;
            if (SUCCEEDED(s_d->CreateLinearGradientBrush(lp, stops, &br))) {
                s_d->FillRectangle(R(0, 0, WIN_W, WIN_H), br);
                br->Release();
            }
            stops->Release();
        }
        new_fade(D2D1::Point2F(0, 0), D2D1::Point2F(0, 190), 0x000000, 0.55f, 0.0f, R(0, 0, WIN_W, 190));
        new_fade(D2D1::Point2F(0, WIN_H - 230), D2D1::Point2F(0, WIN_H), 0x000000, 0.0f, 0.55f,
                 R(0, WIN_H - 230, WIN_W, 230));
        new_fade(D2D1::Point2F(0, 0), D2D1::Point2F(320, 0), 0x000000, 0.45f, 0.0f, R(0, 0, 320, WIN_H));
    }
    /* Embers: still, fixed spots, so a capture is stable. */
    for (int i = 0; i < 90; i++) {
        const float x = (float)((i * 197 + 43) % 1100);
        const float y = (float)((i * 331 + 71) % 700);
        const float a = 0.10f + 0.30f * (float)((i * 53) % 100) / 100.0f;
        fill(R(x, y, 2, 2), NC_GOLD_HI, a);
    }
}

/* A pill, in amber (attention) or Delphi blue (information). */
static float new_chip_tint(const wchar_t* s, float x, float y, float a, unsigned border, unsigned txt)
{
    const float w = snap(text_w(s, s_tf_small) + 26.0f);
    const D2D1_RECT_F r = R(x, y, w, 26);
    fill(r, 0x121820, 0.92f * a);
    frame(r, border, 0.55f * a, 1.0f);
    text(s, s_tf_small, r, txt, a, DWRITE_TEXT_ALIGNMENT_CENTER);
    return w;
}
static float new_chip(const wchar_t* s, float x, float y, float a)
{
    return new_chip_tint(s, x, y, a, NC_GOLD, NC_GOLD_HI);
}

static void new_nav(void)
{
    const float y = WIN_H - NC_NAV_H;
    const int page = new_page();
    fill(R(0, y, WIN_W, NC_NAV_H), 0x0A0C0F, 0.97f);
    fill(R(0, y, WIN_W, 1), 0x000000, 0.60f);
    float x = 28.0f;
    for (int i = 0; i < 11; i++) {
        const int on = (i == page);
        const wchar_t* s = new_page_title(i);
        const float w = text_w(s, s_tf_small) + 34.0f;
        text(s, s_tf_small, R(x, y + 10, w - 34.0f, 44.0f),
             on ? NC_RED : (hovered(H_NEW_NAV + i) ? NC_TEXT : NC_TEXT2));
        if (on) fill(R(x, y, w - 34.0f, 2.0f), NC_RED);
        hit(R(x, y, w, NC_NAV_H), H_NEW_NAV + i);
        x += w;
    }
    text(W("v" AR_VERSION), s_tf_small, R(WIN_W - 40, y - 22, 200, 20), NC_DESC, 1.0f,
         DWRITE_TEXT_ALIGNMENT_TRAILING);
}

/* The line that summarises the display settings. */
static void new_summary(wchar_t* out, size_t n)
{
    const char* mode = ps3_settings_str("display", "mode", NULL, "windowed");
    const char* size = ps3_settings_str("display", "window_size", NULL, "");
    const char* cap = ps3_settings_str("display", "frame_cap", NULL, "off");
    const wchar_t* m = L"Ventana";
    if (mode && !strcmp(mode, "borderless")) m = L"Sin bordes";
    else if (mode && !strcmp(mode, "fullscreen")) m = L"Pantalla completa";
    wchar_t capw[48];
    if (!cap || !cap[0] || !strcmp(cap, "off")) wcscpy(capw, L"Sin l\x00EDmite");
    else swprintf(capw, 48, L"L\x00EDmite %hs FPS", cap);
    if (size && size[0]) {
        wchar_t sizeh[24];
        MultiByteToWideChar(CP_UTF8, 0, size, -1, sizeh, 24);
        swprintf(out, n, L"%ls  \x00B7  %ls  \x00B7  %ls", sizeh, m, capw);
    } else {
        swprintf(out, n, L"%ls  \x00B7  %ls  \x00B7  Tama\x00F1o autom\x00E1tico", m, capw);
    }
}

static void new_hero(float a, int chips)
{
    const float right = WIN_W - 40.0f;
    const float wy = WIN_H - 372.0f;
    new_logo_load();
    float subW, subX;
    if (s_new_logo) {
        const D2D1_SIZE_U ls = s_new_logo->GetPixelSize();
        const float lw = 430.0f;
        const float lh = ls.width ? lw * (float)ls.height / (float)ls.width : 70.0f;
        subX = right - lw;
        subW = lw;
        s_d->DrawBitmap(s_new_logo, R(subX, wy + 70.0f - lh, lw, lh), a);
    } else {
        const wchar_t* wa = L"GOWA";
        const wchar_t* wb = L"-cMc";
        const float qa = text_w(wa, s_tf_display_xl), qb = text_w(wb, s_tf_display_xl);
        text(wa, s_tf_display_xl, R(right - qa - qb, wy, qa + 10, 80), NC_TEXT, a);
        text(wb, s_tf_display_xl, R(right - qb, wy, qb + 10, 80), NC_RED, a);
        subX = right - qa - qb;
        subW = qa + qb;
    }
    text(L"GOD OF WAR: ASCENSION \x00B7 BCUS98232", s_tf_small, R(subX, wy + 74, subW, 22), NC_TEXT2, a);

    const float pw = 340.0f, ph = 62.0f;
    const D2D1_RECT_F pb = R(right - pw, wy + 108, pw, ph);
    const int hot = hovered(H_PLAY) && a > 0.9f;
    if (a > 0.9f) {
        /* Crimson to blood red, with the gods' amber on the edge: Spartan armour, not a flat button. */
        new_grad2(pb, hot ? NC_ACCENT_HI : NC_ACCENT, NC_ACCENT_DEEP);
        frame(pb, NC_GOLD, 0.85f, 1.5f);
    } else {
        fill(pb, NC_ACCENT, a);
        frame(pb, NC_GOLD, 0.5f * a, 1.0f);
    }
    text(L"PLAY", s_tf_display_md, pb, 0xFFFFFF, a, DWRITE_TEXT_ALIGNMENT_CENTER);
    hit(pb, H_PLAY);

    if (!chips) return;

    wchar_t c1[220], c2[160], c3[160];
    if (s_game.g.title_id[0]) swprintf(c1, 220, L"\x2713 %ls \x00B7 %ls", W(s_game.g.title_id), game_version());
    else swprintf(c1, 220, L"\x2713 BCUS98232 \x00B7 1.12");
    swprintf(c2, 160, s_rc.elf_ok ? L"\x2713 GOWA.elf listo" : L"\x2717 Falta GOWA.elf");
    swprintf(c3, 160, L"\x2713 Direct3D 12");
    const float w1 = snap(text_w(c1, s_tf_small) + 26.0f);
    const float w2 = snap(text_w(c2, s_tf_small) + 26.0f);
    const float w3 = snap(text_w(c3, s_tf_small) + 26.0f);
    const float cy = wy + 186.0f;
    new_chip_tint(c3, right - w3, cy, a, NC_INFO, NC_INFO);          /* blue: this is information */
    new_chip(c2, right - w3 - 8 - w2, cy, a);                        /* amber: what the launcher needs */
    new_chip(c1, right - w3 - 8 - w2 - 8 - w1, cy, a);

    wchar_t sum[200];
    new_summary(sum, 200);
    text(sum, s_tf_ui, R(right - 560, cy + 42, 560, 24), NC_TEXT2, a, DWRITE_TEXT_ALIGNMENT_TRAILING);
    text(L"ESC cierra  \x00B7  el launcher no toca la partida  \x00B7  port: cMc-OmniPort",
         s_tf_small, R(right - 560, cy + 70, 560, 22), NC_DESC, a, DWRITE_TEXT_ALIGNMENT_TRAILING);
}

/* --- the F2 widgets ---------------------------------------------------------- */

static void new_save_button(int dirty)
{
    const D2D1_RECT_F r = R(NC_PANEL_W - 156, 32, 124, 40);
    const int hot = hovered(H_NEW_SAVE);
    if (dirty) {
        new_grad2(r, hot ? NC_ACCENT_HI : NC_ACCENT, NC_ACCENT_DEEP);
        frame(r, NC_GOLD, 0.85f, 1.5f);
    } else {
        fill(r, 0x1E1618, 1.0f);
        frame(r, NC_LINE, 0.9f, 2.0f);
    }
    text(L"GUARDAR", s_tf_ui_b, r, dirty ? 0xFFFFFF : NC_DESC, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
    hit(r, H_NEW_SAVE);
    if (dirty)
        text(L"CAMBIOS SIN GUARDAR", s_tf_small, R(NC_PANEL_W - 300, 84, 268, 20), NC_RED, 1.0f,
             DWRITE_TEXT_ALIGNMENT_TRAILING);
}

static void new_check_row(Opt* o, int rowIndex, float x, float y, float w)
{
    const int on = opt_on(*o);
    const D2D1_RECT_F box = R(x + w - 26, y + 4, 20, 20);
    const D2D1_RECT_F area = R(x + w - 52, y - 6, 56, 40);
    const int hot = hovered(H_NEW_CTL + rowIndex);
    fill(box, on ? NC_ACCENT : NC_FIELD, 1.0f);
    frame(box, on ? NC_GOLD : (hot ? NC_GOLD : 0x3A4650), on ? 0.9f : 1.0f, 2.0f);
    if (on) text(L"\x2713", s_tf_ui_b, box, 0xFFFFFF, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
    if (y >= s_new_vis_top - 6.0f && y + 40.0f <= s_new_vis_bot + 6.0f) hit(area, H_NEW_CTL + rowIndex);
}

static void new_list_row(Opt* o, int rowIndex, float x, float y, float w)
{
    const int idx = new_opt_index(o);
    const wchar_t* name = W(o->names ? o->names[idx] : o->values[idx]);
    const D2D1_RECT_F f = R(x + w - 226, y - 2, 210, 34);
    const int hot = hovered(H_NEW_CTL + rowIndex);
    fill(f, NC_FIELD, 1.0f);
    frame(f, hot ? NC_GOLD : 0x2E3842, 0.9f, 2.0f);
    text(new_val_es(name), s_tf_small, R(f.left + 10, f.top, f.right - f.left - 34, 34), NC_TEXT);
    icon(k_ic_down, f.right - 24, f.top + 9, hot ? NC_GOLD : NC_TEXT2);
    if (y >= s_new_vis_top - 6.0f && y + 40.0f <= s_new_vis_bot + 6.0f) {
        if (rowIndex >= 0 && rowIndex < 64) s_new_row_field[rowIndex] = f;
        hit(f, H_NEW_CTL + rowIndex);
    }
}

/* One option row: label, description and its control. */
static float new_row(Opt* o, int rowIndex, float x, float y, float w)
{
    const NewEs* es = new_es(o);
    text(es ? es->label : W(o->label), s_tf_ui, R(x + 12, y, w - 250, 26), NC_TEXT);
    float h = 44.0f;
    const wchar_t* d = es ? es->desc : (o->desc ? W(o->desc) : L"");
    if (d && d[0]) {
        const float dh = text_h(d, s_tf_desc, w - 250);
        text(d, s_tf_desc, R(x + 12, y + 24, w - 250, dh + 4), NC_DESC);
        h = 26.0f + dh + 16.0f;
    }
    if (o->check) new_check_row(o, rowIndex, x, y, w);
    else new_list_row(o, rowIndex, x, y, w);
    return h;
}

/* The list under an open drop-down. */
static void new_dropdown(void)
{
    if (s_new_dd < 0 || !s_new_dd_opt) return;
    const Opt* o = s_new_dd_opt;
    fill(R(0, 0, WIN_W, WIN_H), 0x000000, 0.45f);
    const float itemH = 32.0f;
    float top = s_new_dd_at.bottom + 2.0f;
    if (top + o->n * itemH > WIN_H - NC_NAV_H - 8) top = s_new_dd_at.top - o->n * itemH - 2.0f;
    if (top < 8) top = 8;
    const D2D1_RECT_F box = R(s_new_dd_at.left, top, s_new_dd_at.right - s_new_dd_at.left, o->n * itemH + 8);
    fill(box, 0x161E24, 0.99f);
    frame(box, NC_GOLD, 0.75f, 2.0f);
    for (int i = 0; i < o->n; i++) {
        const D2D1_RECT_F it = R(box.left + 4, box.top + 4 + i * itemH, box.right - box.left - 8, itemH);
        const int on = ps3_settings_ieq(o->cur, o->values[i]);
        const int hot = hovered(H_NEW_DDO + i);
        if (on) fill(it, NC_ACCENT, 0.90f);
        else if (hot) fill(it, 0x2A241C, 1.0f);
        text(new_val_es(W(o->names ? o->names[i] : o->values[i])), s_tf_small,
             R(it.left + 10, it.top, it.right - it.left - 20, itemH), on ? 0xFFFFFF : NC_TEXT);
        hit(it, H_NEW_DDO + i);
    }
}

/* --- the LOG page ------------------------------------------------------------ */

static wchar_t s_log_text[96000];
static int s_log_lines;
static wchar_t s_log_file[GAME_PATH_MAX];

static void new_log_reload(void)
{
    s_log_lines = 0;
    s_log_text[0] = 0;
    s_log_file[0] = 0;
    wchar_t dir[GAME_PATH_MAX], pat[GAME_PATH_MAX], name[300] = L"";
    if (!GetModuleFileNameW(NULL, dir, GAME_PATH_MAX)) return;
    if (wchar_t* e = wcsrchr(dir, L'\\')) *e = 0;          /* build */
    swprintf(pat, GAME_PATH_MAX, L"%ls\\logs\\*.log", dir);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        ULONGLONG best = 0;
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            const ULONGLONG t = ((ULONGLONG)fd.ftLastWriteTime.dwHighDateTime << 32) | fd.ftLastWriteTime.dwLowDateTime;
            if (t >= best) { best = t; wcscpy(name, fd.cFileName); }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    if (!name[0]) return;
    swprintf(s_log_file, GAME_PATH_MAX, L"%ls\\logs\\%ls", dir, name);
    HANDLE f = CreateFileW(s_log_file, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    LARGE_INTEGER sz;
    if (GetFileSizeEx(f, &sz)) {
        const long long keep = 96 * 1024;
        LARGE_INTEGER pos;
        pos.QuadPart = sz.QuadPart > keep ? sz.QuadPart - keep : 0;
        SetFilePointerEx(f, pos, NULL, FILE_BEGIN);
        char* buf = (char*)malloc((size_t)keep + 1);
        DWORD got = 0;
        if (buf && ReadFile(f, buf, (DWORD)keep, &got, NULL) && got) {
            buf[got] = 0;
            const int wn = MultiByteToWideChar(CP_UTF8, 0, buf, (int)got, s_log_text, 96000 - 1);
            if (wn > 0) {
                s_log_text[wn] = 0;
                int n = 1;
                for (const wchar_t* q = s_log_text; *q; q++) if (*q == L'\n') n++;
                s_log_lines = n;
            }
        }
        if (buf) free(buf);
    }
    CloseHandle(f);
}

static void new_log_page(float x, float w)
{
    const float top = 146.0f, viewH = WIN_H - NC_NAV_H - 8.0f - top;
    if (!s_log_lines) new_log_reload();
    {
        const wchar_t* base = s_log_file[0] ? wcsrchr(s_log_file, L'\\') : NULL;
        text(base ? base + 1 : L"(todav\x00ED" L"a sin registros)", s_tf_small, R(x, top - 28, w - 340, 20), NC_TEXT2);
        const D2D1_RECT_F b1 = R(x + w - 316, top - 32, 146, 28);
        const D2D1_RECT_F b2 = R(x + w - 162, top - 32, 150, 28);
        fill(b1, NC_FIELD, 1.0f); frame(b1, NC_GOLD, 0.8f, 2.0f);
        text(L"ABRIR CARPETA", s_tf_small, b1, NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
        hit(b1, H_NEW_LOGACT + 0);
        fill(b2, NC_FIELD, 1.0f); frame(b2, NC_GOLD, 0.8f, 2.0f);
        text(L"RECARGAR", s_tf_small, b2, NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
        hit(b2, H_NEW_LOGACT + 1);
    }
    const float lineH = 16.0f;
    const float total = (float)s_log_lines * lineH;
    s_new_scroll_max[8] = total > viewH ? total - viewH + 6.0f : 0.0f;
    if (s_new_scroll[8] < 0.0f) s_new_scroll[8] = 0.0f;
    if (s_new_scroll[8] > s_new_scroll_max[8]) s_new_scroll[8] = s_new_scroll_max[8];
    s_new_vis_top = top;
    s_new_vis_bot = top + viewH;
    s_d->PushAxisAlignedClip(R(0, top - 6, NC_PANEL_W, viewH + 12), D2D1_ANTIALIAS_MODE_ALIASED);
    float y = top - s_new_scroll[8];
    const wchar_t* q = s_log_text;
    while (q && *q) {
        const wchar_t* nl = q;
        while (*nl && *nl != L'\n') nl++;
        if (y > top - lineH && y < top + viewH) {
            wchar_t line[400];
            int m = (int)(nl - q);
            if (m > 399) m = 399;
            wmemcpy(line, q, (size_t)m);
            line[m] = 0;
            while (m > 0 && (line[m - 1] == L'\r' || line[m - 1] == L' ')) line[--m] = 0;
            if (m) text(line, s_tf_small, R(x, y, w - 8, lineH + 2), NC_TEXT2);
        }
        y += lineH;
        q = *nl ? nl + 1 : nl;
    }
    s_d->PopAxisAlignedClip();
    if (s_new_scroll_max[8] > 0.0f) {
        const float track = viewH, thumb = track * viewH / (total + 6.0f);
        const float at = top + (track - thumb) * (s_new_scroll[8] / s_new_scroll_max[8]);
        fill(R(NC_PANEL_W - 8, top, 3, track), 0x1E262E, 0.9f);
        fill(R(NC_PANEL_W - 8, at, 3, thumb), NC_ACCENT, 0.85f);
    }
}

/* --- the TROPHIES page ------------------------------------------------------- */

/* The trophy definitions travel in the game's TROPHY.TRP as TROPCONF.SFM, which is
 * signed but NOT encrypted: plain UTF-8 XML. The icons are PNG files in the same
 * container. Both are read here, so the page works from the game folder alone. */

static ID2D1Bitmap* s_trop_icon[48];
static int s_trop_icons, s_trop_tried, s_trop_pngs, s_trop_defs;
static wchar_t s_trop_id[48], s_trop_dir[GAME_PATH_MAX];
static char s_trop_err[96];
static wchar_t s_trop_name[64][96];
static wchar_t s_trop_detail[64][192];
static char s_trop_ttype[64];
static char s_trop_seen[64];
static unsigned char s_trop_earned[64];
static int s_trop_earned_n;
static int s_trop_usr;

static int new_utf8_w(const char* s, int n, wchar_t* out, int cap)
{
    if (n <= 0) { out[0] = 0; return 0; }
    int w = MultiByteToWideChar(CP_UTF8, 0, s, n, out, cap - 1);
    if (w < 0) w = 0;
    out[w] = 0;
    return w;
}

static const char* new_find(const char* h, size_t hn, const char* nd, size_t nn)
{
    if (nn > hn) return NULL;
    for (size_t i = 0; i + nn <= hn; i++)
        if (!memcmp(h + i, nd, nn)) return h + i;
    return NULL;
}

static void new_trophy_parse(const char* x, size_t n)
{
    size_t i = 0;
    while (i + 8 < n) {
        const char* t = new_find(x + i, n - i, "<trophy ", 8);
        if (!t) break;
        const size_t at = (size_t)(t - x);
        size_t tagEnd = at + 8;
        while (tagEnd < n && x[tagEnd] != '>') tagEnd++;
        if (tagEnd >= n) break;
        if (tagEnd > 0 && x[tagEnd - 1] == '/') {   /* a skeleton entry: id and type only */
            i = tagEnd + 1;
            continue;
        }
        const char* close = new_find(x + tagEnd, n - tagEnd, "</trophy>", 9);
        if (!close) break;
        const size_t endAt = (size_t)(close - x);
        int id = -1;
        char tt = 0;
        {
            const char* idp = new_find(x + at, tagEnd - at, "id=\"", 4);
            if (idp) id = atoi(idp + 4);
            const char* tp = new_find(x + at, tagEnd - at, "ttype=\"", 7);
            if (tp) tt = tp[7];
        }
        if (id >= 0 && id < 64) {
            if (tt) s_trop_ttype[id] = tt;
            const char* np = new_find(x + tagEnd, endAt - tagEnd, "<name>", 6);
            const char* ne = np ? new_find(np, endAt - (size_t)(np - x), "</name>", 7) : NULL;
            if (np && ne && !s_trop_name[id][0])
                new_utf8_w(np + 6, (int)(ne - (np + 6)), s_trop_name[id], 96);
            const char* dp = new_find(x + tagEnd, endAt - tagEnd, "<detail>", 8);
            const char* de = dp ? new_find(dp, endAt - (size_t)(dp - x), "</detail>", 9) : NULL;
            if (dp && de && !s_trop_detail[id][0])
                new_utf8_w(dp + 8, (int)(de - (dp + 8)), s_trop_detail[id], 192);
            if (!s_trop_seen[id]) { s_trop_seen[id] = 1; s_trop_defs++; }
        }
        i = endAt + 9;
    }
}

static ID2D1Bitmap* new_png_bitmap(const void* data, size_t n)
{
    if (!s_wic || !s_rt) return NULL;
    IWICStream* st = NULL;
    IWICBitmapDecoder* dec = NULL;
    IWICBitmapFrameDecode* fr = NULL;
    IWICFormatConverter* cv = NULL;
    ID2D1Bitmap* bm = NULL;
    if (SUCCEEDED(s_wic->CreateStream(&st)) &&
        SUCCEEDED(st->InitializeFromMemory((BYTE*)data, (DWORD)n)) &&
        SUCCEEDED(s_wic->CreateDecoderFromStream(st, NULL, WICDecodeMetadataCacheOnDemand, &dec)) &&
        SUCCEEDED(dec->GetFrame(0, &fr)) && SUCCEEDED(s_wic->CreateFormatConverter(&cv)) &&
        SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0,
                                 WICBitmapPaletteTypeMedianCut)))
        s_rt->CreateBitmapFromWicBitmap(cv, NULL, &bm);
    if (cv) cv->Release();
    if (fr) fr->Release();
    if (dec) dec->Release();
    if (st) st->Release();
    return bm;
}

static size_t new_png_size(const unsigned char* b, size_t n)
{
    if (n < 8 || b[0] != 0x89 || b[1] != 'P' || b[2] != 'N' || b[3] != 'G') return 0;
    size_t p = 8;
    while (p + 12 <= n) {
        const size_t len = ((size_t)b[p] << 24) | ((size_t)b[p + 1] << 16) | ((size_t)b[p + 2] << 8) | b[p + 3];
        const char* type = (const char*)(b + p + 4);
        p += 12 + len;
        if (p > n) return 0;
        if (!memcmp(type, "IEND", 4)) return p;
    }
    return 0;
}

static int new_trophy_lang(void)
{
    const char* l = ps3_settings_str("system", "language", NULL, "auto");
    if (!l || !l[0] || ps3_settings_ieq(l, "auto")) {
        switch (PRIMARYLANGID(GetUserDefaultUILanguage())) {
        case LANG_JAPANESE: return 0;
        case LANG_FRENCH: return 2;
        case LANG_SPANISH: return 3;
        case LANG_GERMAN: return 4;
        case LANG_ITALIAN: return 5;
        case LANG_DUTCH: return 6;
        case LANG_PORTUGUESE: return 7;
        case LANG_RUSSIAN: return 8;
        case LANG_KOREAN: return 9;
        case LANG_CHINESE: return 10;
        case LANG_POLISH: return 16;
        case LANG_TURKISH: return 19;
        default: return 1;
        }
    }
    if (ps3_settings_ieq(l, "es")) return 3;
    if (ps3_settings_ieq(l, "fr")) return 2;
    if (ps3_settings_ieq(l, "de")) return 4;
    if (ps3_settings_ieq(l, "it")) return 5;
    if (ps3_settings_ieq(l, "nl")) return 6;
    if (ps3_settings_ieq(l, "pt-br")) return 7;
    if (ps3_settings_ieq(l, "pt")) return 17;
    if (ps3_settings_ieq(l, "ru")) return 8;
    if (ps3_settings_ieq(l, "ja")) return 0;
    if (ps3_settings_ieq(l, "ko")) return 9;
    if (ps3_settings_ieq(l, "zh")) return 10;
    if (ps3_settings_ieq(l, "pl")) return 16;
    if (ps3_settings_ieq(l, "tr")) return 19;
    return 1;   /* en, en-gb, anything else English */
}

/* The TRP entry with that file name, from its big-endian table (RPCS3's TRPEntry). */
static int new_trp_entry(const unsigned char* b, size_t n, const char* name, size_t* off, size_t* len)
{
    if (n < 0x40) return 0;
    const unsigned be32 = ((unsigned)b[0x10] << 24) | ((unsigned)b[0x11] << 16) | ((unsigned)b[0x12] << 8) | b[0x13];
    const unsigned elem = ((unsigned)b[0x14] << 24) | ((unsigned)b[0x15] << 16) | ((unsigned)b[0x16] << 8) | b[0x17];
    if (!elem || elem < 0x40) return 0;
    for (unsigned i = 0; i < be32; i++) {
        const size_t e = 0x40 + (size_t)i * elem;
        if (e + 0x30 > n) break;
        if (strncmp((const char*)b + e, name, 32)) continue;
        unsigned long long o = 0, s = 0;
        for (int k = 0; k < 8; k++) o = (o << 8) | b[e + 32 + k];
        for (int k = 0; k < 8; k++) s = (s << 8) | b[e + 40 + k];
        if (o + s > n) return 0;
        *off = (size_t)o;
        *len = (size_t)s;
        return 1;
    }
    return 0;
}
/* TROPUSR.DAT (the player's trophy record): big-endian header, tables, and entry 6
 * carries the unlocked flag. Stride is the content size plus the 16-byte entry header. */
static unsigned new_be32u(const unsigned char* p)
{
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) | ((unsigned)p[2] << 8) | p[3];
}
static unsigned long long new_be64u(const unsigned char* p)
{
    unsigned long long v = 0;
    for (int i = 0; i < 8; i++) v = (v << 8) | p[i];
    return v;
}
static void new_tropusr_parse(const unsigned char* b, size_t n)
{
    if (n < 48 || new_be32u(b) != 0x818F54AD) return;
    const unsigned tables = new_be32u(b + 8);
    for (unsigned t = 0; t < tables; t++) {
        const size_t th = 48 + (size_t)t * 32;
        if (th + 32 > n) break;
        const unsigned type = new_be32u(b + th);
        const unsigned esize = new_be32u(b + th + 4);
        const unsigned count = new_be32u(b + th + 12);
        const unsigned long long off = new_be64u(b + th + 16);
        if (type != 6 || esize < 24) continue;
        const size_t stride = (size_t)esize + 16;
        for (unsigned e = 0; e < count; e++) {
            const size_t ep = (size_t)off + (size_t)e * stride;
            if (ep + 24 > n) break;
            const unsigned id = new_be32u(b + ep + 16);
            const unsigned state = new_be32u(b + ep + 20);
            if (id < 64 && state && !s_trop_earned[id]) {
                s_trop_earned[id] = 1;
                s_trop_earned_n++;
                s_trop_usr = 1;
            }
        }
        if (s_trop_earned_n) s_trop_usr = 1;
    }
    if (!s_trop_usr) s_trop_usr = 1;   /* the file was there: zero unlocked counts */
}

/* Look for it under the port's own hdd0 (gamedata/dev_hdd0/home/<usr>/trophy/<set>). */
static void new_tropusr_find(void)
{
    wchar_t repo[GAME_PATH_MAX], home[GAME_PATH_MAX], pat[GAME_PATH_MAX];
    if (!GetModuleFileNameW(NULL, repo, GAME_PATH_MAX)) return;
    if (wchar_t* e = wcsrchr(repo, L'\\')) *e = 0;
    if (wchar_t* e = wcsrchr(repo, L'\\')) *e = 0;      /* repo */
    swprintf(home, GAME_PATH_MAX, L"%ls\\gamedata\\dev_hdd0\\home", repo);
    WIN32_FIND_DATAW fd;
    swprintf(pat, GAME_PATH_MAX, L"%ls\\*", home);
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        wchar_t tro[GAME_PATH_MAX], pat2[GAME_PATH_MAX];
        swprintf(tro, GAME_PATH_MAX, L"%ls\\%ls\\trophy", home, fd.cFileName);
        swprintf(pat2, GAME_PATH_MAX, L"%ls\\*", tro);
        WIN32_FIND_DATAW fd2;
        HANDLE h2 = FindFirstFileW(pat2, &fd2);
        if (h2 == INVALID_HANDLE_VALUE) continue;
        do {
            if (!(fd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            wchar_t usr[GAME_PATH_MAX];
            swprintf(usr, GAME_PATH_MAX, L"%ls\\%ls\\TROPUSR.DAT", tro, fd2.cFileName);
            HANDLE f = CreateFileW(usr, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (f == INVALID_HANDLE_VALUE) continue;
            DWORD sz = GetFileSize(f, NULL);
            if (sz > 0 && sz < 256 * 1024) {
                unsigned char* b = (unsigned char*)malloc(sz);
                DWORD got = 0;
                if (b && ReadFile(f, b, sz, &got, NULL) && got == sz) new_tropusr_parse(b, got);
                if (b) free(b);
            }
            CloseHandle(f);
        } while (FindNextFileW(h2, &fd2));
        FindClose(h2);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static void new_trophies_load(void)
{
    if (s_trop_tried) return;
    s_trop_tried = 1;
    wchar_t repo[GAME_PATH_MAX], trops[GAME_PATH_MAX], pat[GAME_PATH_MAX];
    if (!GetModuleFileNameW(NULL, repo, GAME_PATH_MAX)) return;
    if (wchar_t* e = wcsrchr(repo, L'\\')) *e = 0;      /* build */
    if (wchar_t* e = wcsrchr(repo, L'\\')) *e = 0;      /* repo */
    swprintf(trops, GAME_PATH_MAX, L"%ls\\vfs\\PS3_GAME\\TROPDIR", repo);
    swprintf(s_trop_dir, GAME_PATH_MAX, L"%ls", trops);
    WIN32_FIND_DATAW fd;
    swprintf(pat, GAME_PATH_MAX, L"%ls\\*", trops);
    HANDLE h = FindFirstFileW(pat, &fd);
    wchar_t trp[GAME_PATH_MAX] = L"";
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            wchar_t tryp[GAME_PATH_MAX];
            swprintf(tryp, GAME_PATH_MAX, L"%ls\\%ls\\TROPHY.TRP", trops, fd.cFileName);
            if (GetFileAttributesW(tryp) != INVALID_FILE_ATTRIBUTES) {
                wcscpy(trp, tryp);
                wcscpy(s_trop_id, fd.cFileName);
                break;
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    if (!trp[0]) { strcpy(s_trop_err, "no se encontro TROPHY.TRP en el juego"); return; }
    HANDLE f = CreateFileW(trp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) { strcpy(s_trop_err, "no se pudo abrir TROPHY.TRP"); return; }
    LARGE_INTEGER sz;
    GetFileSizeEx(f, &sz);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz.QuadPart);
    DWORD got = 0;
    if (buf && ReadFile(f, buf, (DWORD)sz.QuadPart, &got, NULL) && got == sz.QuadPart) {
        /* the icons */
        size_t i = 0;
        while (i + 8 < got && s_trop_pngs < 400) {
            if (buf[i] == 0x89 && buf[i + 1] == 'P' && buf[i + 2] == 'N' && buf[i + 3] == 'G') {
                const size_t len = new_png_size(buf + i, got - i);
                if (len) {
                    s_trop_pngs++;
                    if (s_trop_icons < 48) {
                        ID2D1Bitmap* bm = new_png_bitmap(buf + i, len);
                        if (bm) s_trop_icon[s_trop_icons++] = bm;
                    }
                    i += len;
                    continue;
                }
            }
            i++;
        }
        /* The definitions: plain XML inside the TRP. The localized one is picked by the
         * game's language (TROP_XX.SFM, or TROP.SFM for English), as the PS3 does. */
        {
            const int lang = new_trophy_lang();
            char ent[40] = "";
            size_t off = 0, len = 0;
            int ok = 0;
            if (lang == 1) {
                ok = new_trp_entry(buf, got, "TROP.SFM", &off, &len);
            } else {
                snprintf(ent, sizeof ent, "TROP_%02d.SFM", lang);
                ok = new_trp_entry(buf, got, ent, &off, &len);
            }
            if (!ok) ok = new_trp_entry(buf, got, "TROP.SFM", &off, &len);
            if (!ok) ok = new_trp_entry(buf, got, "TROPCONF.SFM", &off, &len);
            if (ok) new_trophy_parse((const char*)buf + off, len);
            else new_trophy_parse((const char*)buf, got);   /* last resort */
        }
        new_tropusr_find();
    }
    if (buf) free(buf);
    CloseHandle(f);
}

static const wchar_t* new_trophy_type_name(char t)
{
    switch (t) {
    case 'P': return L"PLATINO";
    case 'G': return L"ORO";
    case 'S': return L"PLATA";
    default: return L"BRONCE";
    }
}

static unsigned new_trophy_type_color(char t)
{
    switch (t) {
    case 'P': return 0xEAF6FF;
    case 'G': return NC_GOLD_HI;
    case 'S': return NC_TEXT2;
    default: return 0xB08D57;
    }
}

static void new_trophies_page(float x, float w)
{
    const float top = 146.0f, viewH = WIN_H - NC_NAV_H - 8.0f - top;
    new_trophies_load();
    text(s_trop_id[0] ? s_trop_id : L"TROFEOS", s_tf_display_md, R(x, top - 34, w - 220, 30), NC_GOLD_HI);
    {
        wchar_t head[160];
        if (s_trop_usr)
            swprintf(head, 160, L"%d trofeos \x00B7 %d conseguidos", s_trop_defs, s_trop_earned_n);
        else
            swprintf(head, 160, L"%d trofeos \x00B7 sin registro de progreso", s_trop_defs);
        text(head, s_tf_small, R(x, top - 4, w - 220, 20), NC_TEXT2);
        const D2D1_RECT_F b = R(x + w - 190, top - 34, 180, 30);
        fill(b, NC_FIELD, 1.0f);
        frame(b, hovered(H_NEW_LOGACT + 2) ? NC_GOLD_HI : NC_GOLD, 0.8f, 2.0f);
        text(L"ABRIR CARPETA", s_tf_small, b, NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
        hit(b, H_NEW_LOGACT + 2);
    }
    if (!s_trop_defs) {
        text(s_trop_err[0] ? W(s_trop_err) : L"Sin definiciones de trofeos.", s_tf_desc, R(x, top + 20, w, 44), NC_DESC);
        return;
    }

    const float rowH = 70.0f;
    const float total = (float)s_trop_defs * rowH;
    s_new_scroll_max[10] = total > viewH ? total - viewH + 6.0f : 0.0f;
    if (s_new_scroll[10] < 0.0f) s_new_scroll[10] = 0.0f;
    if (s_new_scroll[10] > s_new_scroll_max[10]) s_new_scroll[10] = s_new_scroll_max[10];
    s_new_vis_top = top + 24.0f;
    s_new_vis_bot = top + viewH;
    s_d->PushAxisAlignedClip(R(0, top + 20, NC_PANEL_W, viewH - 20), D2D1_ANTIALIAS_MODE_ALIASED);
    float y = top + 24.0f - s_new_scroll[10];
    for (int id = 0; id < 64; id++) {
        if (!s_trop_name[id][0]) continue;
        if (y > top + viewH) break;
        const D2D1_RECT_F ib = R(x, y + 4, 44, 44);
        const int ic = (id + 1 < s_trop_icons) ? id + 1 : (id < s_trop_icons ? id : -1);
        if (ic >= 0) {
            fill(ib, 0x0B0F13, 1.0f);
            frame(ib, 0x2E3842, 0.9f, 1.0f);
            s_d->DrawBitmap(s_trop_icon[ic], ib, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        }
        text(s_trop_name[id], s_tf_ui, R(x + 56, y + 2, w - 210, 24),
             s_trop_earned[id] ? NC_GOLD_HI : NC_TEXT);
        if (s_trop_detail[id][0])
            text(s_trop_detail[id], s_tf_desc, R(x + 56, y + 24, w - 210, 40), NC_DESC);
        if (s_trop_earned[id])
            text(L"\x2713 CONSEGUIDO", s_tf_small, R(x + w - 130, y + 8, 130, 22), NC_GOLD_HI, 1.0f,
                 DWRITE_TEXT_ALIGNMENT_TRAILING);
        else
            text(new_trophy_type_name(s_trop_ttype[id]), s_tf_small, R(x + w - 120, y + 8, 120, 22),
                 new_trophy_type_color(s_trop_ttype[id]), 1.0f, DWRITE_TEXT_ALIGNMENT_TRAILING);
        y += rowH;
    }
    s_d->PopAxisAlignedClip();
    if (s_new_scroll_max[10] > 0.0f) {
        const float track = viewH - 20.0f, thumb = track * (viewH - 20.0f) / (total + 6.0f);
        const float at = top + 20.0f + (track - thumb) * (s_new_scroll[10] / s_new_scroll_max[10]);
        fill(R(NC_PANEL_W - 8, top + 20.0f, 3, track), 0x1E262E, 0.9f);
        fill(R(NC_PANEL_W - 8, at, 3, thumb), NC_ACCENT, 0.85f);
    }
}

/* --- the ADVANCED page ------------------------------------------------------- */
/* --- the ADVANCED page ------------------------------------------------------- */

/* The package root, from the exe (which sits in _src\GOWA-cMc\build). */
static void new_pkg_root(wchar_t* out, size_t n)
{
    if (!GetModuleFileNameW(NULL, out, (DWORD)n)) { out[0] = 0; return; }
    for (int i = 0; i < 4; i++) {   /* build, repo, _src, package */
        wchar_t* e = wcsrchr(out, L'\\');
        if (!e) break;
        *e = 0;
        if (i >= 3) break;
    }
}


/* MODS: the real 0.2.0 loader (mods.cpp), not the old PS3_MODS_ROOT hook. */
static void new_btn(const wchar_t* s, D2D1_RECT_F r, int id, int accent);
static ModList s_mods;
static int s_mods_ready;

static void new_mods_reload(int unpack)
{
    mods_scan(&s_mods, unpack);
    s_mods_ready = 1;
}

static void new_mods_page(float x, float w)
{
    const float top = 146.0f, viewH = WIN_H - NC_NAV_H - 8.0f - top;
    if (!s_mods_ready) new_mods_reload(0);

    {
        wchar_t head[220];
        swprintf(head, 220, L"%d mods \x00B7 %ls", (int)s_mods.mods.size(),
                 s_mods.use ? L"activos" : L"desactivados");
        text(head, s_tf_small, R(x, top - 4, w - 360, 20), NC_TEXT2);
        text(s_mods.dir.empty() ? L"Sin carpeta de mods" : s_mods.dir.c_str(),
             s_tf_desc, R(x, top + 20, w - 360, 22), NC_DESC);
    }
    {
        const float by = top - 10, bw = 92;
        new_btn(L"RECARGAR", R(x + w - 4 * bw - 18, by, bw, 28), H_NEW_MODS_ACT + 0, 0);
        new_btn(L"ABRIR", R(x + w - 3 * bw - 12, by, bw, 28), H_NEW_MODS_ACT + 1, 0);
        new_btn(L"TODO ON", R(x + w - 2 * bw - 6, by, bw, 28), H_NEW_MODS_ACT + 2, 0);
        new_btn(L"TODO OFF", R(x + w - bw, by, bw, 28), H_NEW_MODS_ACT + 3, 0);
    }

    if (s_mods.mods.empty()) {
        text(L"No hay mods. Crea la carpeta \"mods\" junto a la configuraci\x00F3n "
             L"y pon dentro una carpeta o un .zip por mod.",
             s_tf_desc, R(x, top + 62, w, 60), NC_DESC);
        return;
    }

    const float rowH = 58.0f;
    const float total = (float)s_mods.mods.size() * rowH;
    s_new_scroll_max[6] = total > viewH ? total - viewH + 6.0f : 0.0f;
    if (s_new_scroll[6] < 0.0f) s_new_scroll[6] = 0.0f;
    if (s_new_scroll[6] > s_new_scroll_max[6]) s_new_scroll[6] = s_new_scroll_max[6];
    s_new_vis_top = top + 48.0f;
    s_new_vis_bot = top + viewH;
    s_d->PushAxisAlignedClip(R(0, top + 44, NC_PANEL_W, viewH - 44), D2D1_ANTIALIAS_MODE_ALIASED);
    float y = top + 50.0f - s_new_scroll[6];
    for (int i = 0; i < (int)s_mods.mods.size(); i++) {
        const ModInfo& m = s_mods.mods[i];
        const D2D1_RECT_F r = R(x, y, w, rowH - 6.0f);
        fill(r, m.on ? 0x151C23 : 0x10151B, 1.0f);
        frame(r, m.on ? NC_LINE : 0x20252C, 0.8f, 1.0f);
        hit(r, H_NEW_MODS + i);
        wchar_t line[320];
        swprintf(line, 320, L"%hs%ls%hs", "", m.on ? L"\x2713 " : L"  ", "");
        text(line, s_tf_small, R(x + 10, y, 30, rowH - 6), m.on ? NC_OK : NC_TEXT2);
        swprintf(line, 320, L"%ls", W(m.name.c_str()));
        text(line, s_tf_ui, R(x + 44, y + 4, w - 240, 22), m.on ? NC_TEXT : NC_TEXT2);
        if (m.error[0])
            swprintf(line, 320, L"%ls", W(m.error.c_str()));
        else
            swprintf(line, 320, L"%d archivos%ls", m.files, m.zip ? L"  \x00B7  .zip" : L"");
        text(line, s_tf_desc, R(x + 44, y + 26, w - 240, 20), m.error[0] ? NC_BAD : NC_DESC);
        if (m.version[0]) {
            swprintf(line, 320, L"v%ls", W(m.version.c_str()));
            text(line, s_tf_desc, R(x + w - 180, y + 4, 170, 20), NC_DESC, 1.0f, DWRITE_TEXT_ALIGNMENT_TRAILING);
        }
        y += rowH;
    }
    s_d->PopAxisAlignedClip();
}

static void new_open(const wchar_t* path)
{
    ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
}

static void new_h3(const wchar_t* label, float x, float* y, float w)
{
    text(label, s_tf_display_md, R(x, *y, w, 30), NC_RED);
    fill(R(x, *y + 32, w, 1), NC_LINE);
    *y += 48.0f;
}

static void new_kv(const wchar_t* k, const wchar_t* v, float x, float* y, float w)
{
    text(k, s_tf_ui, R(x, *y, w * 0.45f, 24), NC_TEXT2);
    text(v, s_tf_small, R(x + w * 0.45f, *y, w * 0.55f, 24), NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_TRAILING);
    *y += 26.0f;
}

static void new_tool_row(const wchar_t* label, const wchar_t* button, int tool, float x, float* y, float w)
{
    text(label, s_tf_ui, R(x, *y + 6, w - 190, 26), NC_TEXT2);
    const D2D1_RECT_F b = R(x + w - 160, *y + 2, 150, 32);
    const int hot = hovered(H_NEW_TOOL + tool);
    fill(b, NC_FIELD, 1.0f);
    frame(b, hot ? NC_GOLD : 0x2E3842, 0.85f, 2.0f);
    text(button, s_tf_small, b, hot ? NC_GOLD_HI : NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
    hit(b, H_NEW_TOOL + tool);
    *y += 44.0f;
}

static void new_advanced(float x, float w)
{
    const float top = 146.0f, viewH = WIN_H - NC_NAV_H - 8.0f - top;
    wchar_t root[GAME_PATH_MAX], p[GAME_PATH_MAX];
    new_pkg_root(root, GAME_PATH_MAX);

    const float total = 610.0f;
    s_new_scroll_max[7] = total > viewH ? total - viewH + 6.0f : 0.0f;
    if (s_new_scroll[7] < 0.0f) s_new_scroll[7] = 0.0f;
    if (s_new_scroll[7] > s_new_scroll_max[7]) s_new_scroll[7] = s_new_scroll_max[7];

    s_new_vis_top = top;
    s_new_vis_bot = top + viewH;
    s_d->PushAxisAlignedClip(R(0, top - 6, NC_PANEL_W, viewH + 12), D2D1_ANTIALIAS_MODE_ALIASED);
    float y = top - s_new_scroll[7];

    new_h3(L"VERSI\x00D3N", x, &y, w);
    new_kv(L"Port", W("v" AR_VERSION), x, &y, w);
    new_kv(L"Juego", L"BCUS98232 \x00B7 1.12", x, &y, w);
    y += 10.0f;

    new_h3(L"CR\x00E9" L"DITOS", x, &y, w);
    {
        static const wchar_t* lines[] = {
            L"Motor y port original: Ascension-R \x2014 Ryan Bagas (MIT)",
            L"Toolkit y runtime: ps3recomp \x2014 Ned Heller / sp00nznet y colaboradores (MIT)",
            L"Port a BCUS98232: cMc-OmniPort",
            L"FFmpeg (LGPL) \x00B7 DXC \x00B7 MinGit \x00B7 Cinzel (OFL)",
            L"No se distribuyen archivos del juego.",
        };
        for (size_t i = 0; i < sizeof lines / sizeof lines[0]; i++) {
            text(lines[i], s_tf_small, R(x, y, w, 20), i == 4 ? NC_DESC : NC_TEXT2);
            y += 20.0f;
        }
    }
    y += 10.0f;

    new_h3(L"HERRAMIENTAS DEL PAQUETE", x, &y, w);
    new_tool_row(L"Buscar y compilar una versi\x00F3n nueva", L"ACTUALIZAR", 0, x, &y, w);
    new_tool_row(L"Activar la build pendiente", L"PROMOVER", 1, x, &y, w);
    new_tool_row(L"Volver a la build anterior", L"REVERTIR", 2, x, &y, w);
    text(L"Se abren en su propia ventana; la de actualizar compila en build.next.",
         s_tf_small, R(x, y, w, 20), NC_DESC);
    y += 34.0f;

    new_h3(L"RUTAS", x, &y, w);
    text(root, s_tf_small, R(x, y, w, 20), NC_TEXT2);
    y += 24.0f;
    new_tool_row(L"Carpeta de logs de la build", L"ABRIR LOGS", 3, x, &y, w);
    new_tool_row(L"Carpeta del paquete", L"ABRIR", 4, x, &y, w);

    s_d->PopAxisAlignedClip();

    if (s_new_scroll_max[7] > 0.0f) {
        const float track = viewH, thumb = track * viewH / (total + 6.0f);
        const float at = top + (track - thumb) * (s_new_scroll[7] / s_new_scroll_max[7]);
        fill(R(NC_PANEL_W - 8, top, 3, track), 0x1E262E, 0.9f);
        fill(R(NC_PANEL_W - 8, at, 3, thumb), NC_ACCENT, 0.85f);
    }
}

/* The left settings page: title, subtitle, save, the rows and the scroll. */
/* --- F3/F4: the game's folder, the decrypting and HOME's ready check ------------- */

/* A small button of this shell: filled for the one that matters, a field otherwise. */
static void new_btn(const wchar_t* s, D2D1_RECT_F r, int id, int accent)
{
    const int hot = hovered(id);
    fill(r, accent ? NC_ACCENT : NC_FIELD, 1.0f);
    frame(r, accent ? NC_GOLD : (hot ? NC_GOLD : NC_LINE), accent ? 0.85f : 0.9f, 2.0f);
    text(s, s_tf_small, r, accent ? 0xFFFFFF : (hot ? NC_TEXT : NC_TEXT2), 1.0f,
         DWRITE_TEXT_ALIGNMENT_CENTER);
    hit(r, id);
}

/* A check line: âœ“ good, â€¢ information, ! warning, âœ— bad. */
static void new_line(float x, float y, float w, int level, const wchar_t* s)
{
    const unsigned c = level == 1 ? NC_OK : level == 0 ? NC_WARN : level == -1 ? NC_BAD : NC_INFO;
    text(level == 1 ? L"\x2713" : level == 0 ? L"!" : level == -1 ? L"\x2717" : L"\x2022",
         s_tf_ui_b, R(x, y, 18, 20), c);
    text(s, s_tf_small, R(x + 22, y, w - 22, 40), level == -1 ? NC_BAD : level == 0 ? NC_GOLD_HI : NC_TEXT2);
}

static void new_progress(float x, float y, float w, float frac, int warn)
{
    fill(R(x, y, w, 8), NC_FIELD, 1.0f);
    frame(R(x, y, w, 8), NC_LINE, 0.9f, 1.0f);
    if (frac < 0.0f) frac = 0.0f;
    if (frac > 1.0f) frac = 1.0f;
    fill(R(x + 2, y + 2, (w - 4) * frac, 4), warn ? NC_BAD : NC_GOLD, 1.0f);
}

/* The one line SISTEMA gives a folder: short, and in Spanish. */
static int new_game_line(wchar_t* b, int n)
{
    const GameInfo& g = s_game.g;
    if (g.state == GAME_OK) {
        swprintf(b, n, L"%ls \x00B7 %ls", W(g.title_id), game_version());
        return 1;
    }
    if (g.state == GAME_NO_ELF) { swprintf(b, n, L"Falta GOWA.elf: descifra el juego aqu\x00ED abajo"); return -1; }
    if (g.state == GAME_INCOMPLETE) { swprintf(b, n, L"Faltan archivos del juego o no son la versi\x00F3n 1.12"); return -1; }
    if (g.state == GAME_OTHER_BUILD) { swprintf(b, n, L"No es la versi\x00F3n 1.12 de este port"); return -1; }
    if (g.state == GAME_OTHER_TITLE) { swprintf(b, n, L"Ese juego es %ls; este port es BCUS98232 1.12", W(g.title_id)); return -1; }
    if (s_find.thread) { swprintf(b, n, L"Buscando el juego\x2026"); return 2; }
    swprintf(b, n, L"Sin juego: elige la carpeta (PS3_GAME o una que la contenga)");
    return -1;
}

static int new_need_decrypt(void)
{
    return s_prep.thread != NULL || s_game.g.state == GAME_NO_ELF;
}

/* Fixed part of SISTEMA: its height, and its drawing. It does not scroll; the options do. */
static float new_setup_h(void)
{
    return new_need_decrypt() ? 302.0f : 112.0f;
}

static void new_game_setup(float x, float y, float w)
{
    wchar_t b[GAME_PATH_MAX + 128];
    const float bw = 112.0f;
    text(L"CARPETA DEL JUEGO", s_tf_small, R(x, y, w - 2 * bw - 20, 18), NC_GOLD_HI);
    new_btn(L"EXAMINAR", R(x + w - 2 * bw - 8, y - 6, bw, 28), H_GAME_BROWSE, 0);
    new_btn(s_find.thread ? L"PARAR" : L"DETECTAR", R(x + w - bw, y - 6, bw, 28), H_GAME_AUTO, 0);
    y += 28.0f;
    const wchar_t* shown = game_path_shown();
    if (!wcscmp(shown, L"Not set")) shown = L"Sin elegir";
    text(shown, s_tf_small, R(x, y, w, 20), NC_TEXT2);
    y += 24.0f;
    const int level = new_game_line(b, (int)(sizeof b / sizeof b[0]));
    new_line(x, y, w, level, b);
    y += 26.0f;
    if (s_find.thread) {
        AcquireSRWLockShared(&s_find.lock);
        swprintf(b, sizeof b / sizeof b[0], L"Buscando: %ls", s_find.now);
        ReleaseSRWLockShared(&s_find.lock);
        text(b, s_tf_small, R(x, y, w, 20), NC_TEXT2);
        y += 22.0f;
    }

    if (!new_need_decrypt()) return;

    y += 8.0f;
    text(L"DESCIFRADO", s_tf_small, R(x, y, w, 18), NC_GOLD_HI);
    y += 22.0f;
    if (s_prep.thread) {
        float f = -1.0f;
        const int l = prep_line(b, (int)(sizeof b / sizeof b[0]), &f);
        new_line(x, y, w - 100, l, b);
        new_progress(x, y + 24, w - 110, f, l == 0);
        new_btn(L"PARAR", R(x + w - 90, y - 4, 90, 28), H_PREP_STOP, 0);
        y += 48.0f;
    } else {
        text(L"RPCS3 + licencia (.rap); un SELF de parche necesita su KLIC:",
             s_tf_small, R(x, y, w, 20), NC_TEXT2);
        y += 24.0f;
        float bx = x;
        const float gap = 8.0f;
        const wchar_t* name[4] = { L"ELEGIR RPCS3...", L"DESCARGAR RPCS3", L"ELEGIR .RAP...", L"REINTENTAR" };
        const int id[4] = { H_PICK_RPCS3, H_GET_RPCS3, H_PICK_RAP, H_PREP_RETRY };
        const int shown[4] = { 1, !s_prep.rpcs3[0], 1, s_prep.err[0] && s_prep.rpcs3[0] && s_prep.rap[0] };
        for (int i = 0; i < 4; i++) {
            if (!shown[i]) continue;
            const float wb = snap(text_w(name[i], s_tf_small) + 24.0f);
            new_btn(name[i], R(bx, y, wb, 28), id[i], 0);
            bx += wb + gap;
        }
        y += 34.0f;
        float bx2 = x;
        new_btn(L"USAR GOWA.ELF...", R(bx2, y, 168, 28), H_USE_ELF, 0);
        bx2 += 176.0f;
        new_btn(L"ELEGIR KLIC...", R(bx2, y, 142, 28), H_PICK_KLIC, 0);
        y += 38.0f;
        if (s_prep.err[0]) new_line(x, y, w, -1, s_prep.err);
    }
}

/* HOME: the real pixel-shader prebuild state, not just the setting.
 * 1 built for this version, 2 on but pending, 0 off. */
static int new_pixel_shaders_state(void)
{
    if (!ps3_settings_bool("graphics", "shader_prebuild", NULL, 1)) return 0;
    const char* cache = ps3_settings_str("graphics", "shader_cache", NULL, NULL);
    if (!cache || !*cache) return 2;
    char done[GAME_PATH_MAX + 40];
    snprintf(done, sizeof done, "%s.dxil.prebuilt.done", cache);
    FILE* f = fopen(done, "rb");
    if (!f) return 2;
    char line[128] = { 0 };
    const int ok = fgets(line, sizeof line, f) != NULL;
    fclose(f);
    return ok && strstr(line, AR_VERSION) ? 1 : 2;
}

/* HOME: the ready check and the presets the old shell had, over the art. */
static void new_home_ready(void)
{
    const float x = 32.0f, y = 286.0f, w = 480.0f, h = 360.0f;
    fill(R(x, y, w, h), 0x0A0C0F, 0.86f);
    frame(R(x, y, w, h), NC_GOLD, 0.55f, 2.0f);
    text(L"ESTADO DE LA PARTIDA", s_tf_small, R(x + 18, y + 14, w - 36, 20), NC_GOLD_HI);

    wchar_t b[GAME_PATH_MAX + 128];
    const int lvl = new_game_line(b, (int)(sizeof b / sizeof b[0]));
    new_line(x + 18, y + 40, w - 36, lvl, b);

    float yy = y + 70.0f;
    swprintf(b, sizeof b / sizeof b[0], s_rc.profiles > 0 ? L"Guardados: %d perfil%ls" : L"Guardados: ninguno todav\x00ED" L"a",
             s_rc.profiles, s_rc.profiles == 1 ? L"" : L"es");
    new_line(x + 18, yy, w - 36, s_rc.profiles > 0 ? 1 : 2, b);
    yy += 26.0f;
    swprintf(b, sizeof b / sizeof b[0], L"Gr\x00E1" L"fica: %ls", gpu_chosen()[0] ? W(gpu_chosen()) : L"sin adaptador Direct3D");
    new_line(x + 18, yy, w - 36, gpu_chosen()[0] ? 1 : -1, b);
    yy += 26.0f;
    if (s_rc.pad > 0) swprintf(b, sizeof b / sizeof b[0], L"Mando: %ls", W(s_rc.pad_name));
    else swprintf(b, sizeof b / sizeof b[0], L"Sin mando: teclado y rat\x00F3n");
    new_line(x + 18, yy, w - 36, s_rc.pad > 0 ? 1 : 2, b);
    yy += 26.0f;
    int on = 0;
    for (int i = 0; i < s_npopt; i++) on += opt_on(s_popt[i]);
    swprintf(b, sizeof b / sizeof b[0], L"Parches: %d de %d activos", on, s_npopt);
    new_line(x + 18, yy, w - 36, 2, b);
    yy += 26.0f;
    {
        const int ps = new_pixel_shaders_state();
        if (ps == 1) swprintf(b, sizeof b / sizeof b[0], L"Pixel shaders: listos");
        else if (ps == 0) swprintf(b, sizeof b / sizeof b[0], L"Pixel shaders: desactivados");
        else swprintf(b, sizeof b / sizeof b[0], L"Pixel shaders: pendientes");
        new_line(x + 18, yy, w - 36, ps == 1 ? 1 : 2, b);
    }

    text(L"PRESET", s_tf_small, R(x + 18, y + 244, w - 36, 18), NC_GOLD_HI);
    const int pn = preset_now();
    static const wchar_t* name[3] = { L"Suave", L"Equilibrado", L"Fieles" };
    const float pw = (w - 36.0f - 16.0f) / 3.0f;
    for (int i = 0; i < N_PRESET && i < 3; i++) {
        new_btn(name[i], R(x + 18 + i * (pw + 8), y + 266, pw, 30), H_PRESET + i, i == pn);
    }
    if (s_new_dirty) {
        new_btn(L"GUARDAR", R(x + 18, y + 310, 130, 30), H_SAVE, 1);
        text(L"Cambios sin guardar", s_tf_small, R(x + 158, y + 310, w - 176, 30), NC_RED);
    } else if (!s_rc.elf_ok) {
        text(L"Elige la carpeta del juego en SISTEMA", s_tf_small,
             R(x + 18, y + 310, w - 36, 30), NC_TEXT2);
    } else {
        text(L"Listo para jugar", s_tf_small, R(x + 18, y + 310, w - 36, 30), NC_OK);
    }
}

static void new_left_page(int p)
{
    fill(R(0, 0, NC_PANEL_W, WIN_H), NC_PANEL, 0.99f);
    fill(R(NC_PANEL_W - 1, 0, 1, WIN_H), 0x000000, 0.55f);
    text(new_page_title(p), s_tf_display, R(32, 30, NC_PANEL_W - 200, 58), NC_TEXT);
    text(new_page_sub(p), s_tf_small, R(34, 88, NC_PANEL_W - 68, 22), NC_TEXT2);
    fill(R(32, 124, NC_PANEL_W - 64, 1), NC_LINE);
    if (p != 6 && p != 8 && p != 9 && p != 10) new_save_button(s_new_dirty);
    if (p == 1) new_btn(L"SHADERS", R(NC_PANEL_W - 300, 32, 124, 40), H_NEW_PREBUILD, 0);
    if (p == T_PATCHES) {
        int on = 0;
        for (int i = 0; i < s_npopt; i++) on += opt_on(s_popt[i]);
        wchar_t b[64];
        swprintf(b, 64, L"%d de %d parches activos", on, s_npopt);
        text(b, s_tf_small, R(34, 108, NC_PANEL_W - 68, 20), NC_GOLD_HI);
    }

    if (p == 6) {
        new_mods_page(32.0f, NC_PANEL_W - 72.0f);
        return;
    }
    if (p == 8) {
        new_advanced(32.0f, NC_PANEL_W - 72.0f);
        return;
    }
    if (p == 9) {
        new_log_page(32.0f, NC_PANEL_W - 72.0f);
        return;
    }
    if (p == 10) {
        new_trophies_page(32.0f, NC_PANEL_W - 72.0f);
        return;
    }
    Opt* rows[64];
    const int n = new_page_opts(p, rows, 64);
    const float setupH = (p == 7) ? new_setup_h() : 0.0f;
    const float x = 32.0f, w = NC_PANEL_W - 72.0f;
    const float top = 146.0f + (setupH > 0.0f ? setupH + 14.0f : 0.0f);
    const float viewH = WIN_H - NC_NAV_H - 8.0f - top;
    if (p == 7) new_game_setup(32.0f, 146.0f, NC_PANEL_W - 72.0f);

    if (n == 0) {
        /* F3 pages: the title and the list of what they will hold. */
        static const wchar_t* place[8][6] = {
            { NULL, NULL, NULL, NULL, NULL, NULL },
            { NULL, NULL, NULL, NULL, NULL, NULL },
            { NULL, NULL, NULL, NULL, NULL, NULL },
            { NULL, NULL, NULL, NULL, NULL, NULL },
            { L"Sin MLAA", L"Sin profundidad de campo", L"Sin motion blur", L"Sin recorte de malla", L"Sin sombras", NULL },
            { L"Da\x00F1o recibido", L"Da\x00F1o hecho", L"Orbes m\x00E1ximos", L"Magia infinita", L"Rabia infinita", L"Saltos infinitos" },
            { L"Idioma", L"Audio estable", L"Altavoces", L"Copias de guardado", L"Rutas", L"Precompilar m\x00FAsica" },
            { L"Versi\x00F3n", L"Cr\x00E9" L"ditos", L"Rutas del paquete", L"Logs", L"Actualizar", NULL },
        };
        float y = top;
        for (int i = 0; i < 6; i++) {
            if (!place[p][i]) break;
            text(place[p][i], s_tf_ui, R(x, y, w - 60, 26), NC_TEXT2);
            text(L"\x2014", s_tf_ui, R(x + w - 86, y, 26, 26), NC_DESC, 1.0f, DWRITE_TEXT_ALIGNMENT_TRAILING);
            y += 40.0f;
        }
        text(L"F3: los controles reales de esta p\x00E1gina llegan en la siguiente fase.",
             s_tf_small, R(x, WIN_H - NC_NAV_H - 56, w, 40), NC_DESC);
        return;
    }

    float total = 0.0f;
    for (int i = 0; i < n; i++) {
        const NewEs* es = new_es(rows[i]);
        const wchar_t* d = es ? es->desc : (rows[i]->desc ? W(rows[i]->desc) : L"");
        total += d[0] ? 26.0f + text_h(d, s_tf_desc, w - 250) + 16.0f : 52.0f;
    }
    if (p == 7) total += 3.0f * 56.0f;
    s_new_scroll_max[p] = total > viewH ? total - viewH + 6.0f : 0.0f;
    if (s_new_scroll[p] < 0.0f) s_new_scroll[p] = 0.0f;
    if (s_new_scroll[p] > s_new_scroll_max[p]) s_new_scroll[p] = s_new_scroll_max[p];

    s_new_vis_top = top;
    s_new_vis_bot = top + viewH;
    s_d->PushAxisAlignedClip(R(0, top - 6, NC_PANEL_W, viewH + 12), D2D1_ANTIALIAS_MODE_ALIASED);
    float y = top - s_new_scroll[p];
    for (int i = 0; i < n; i++) {
        const float h = new_row(rows[i], i, x, y, w);
        y += h;
    }
    if (p == 7) {
        static const wchar_t* acts[3] = { L"Abrir carpeta de guardados", L"Informe de problema", L"Salir del launcher" };
        static const wchar_t* lab[3] = { L"ABRIR", L"ABRIR", L"SALIR" };
        for (int i = 0; i < 3; i++) {
            const D2D1_RECT_F b = R(x + w - 150, y + 4, 140, 32);
            text(acts[i], s_tf_ui, R(x + 12, y + 6, w - 200, 26), NC_TEXT2);
            const int hot = hovered(H_NEW_ACT + i);
            fill(b, NC_FIELD, 1.0f);
            frame(b, hot ? NC_GOLD : 0x2E3842, 0.85f, 2.0f);
            text(lab[i], s_tf_small, b, hot ? NC_GOLD_HI : NC_TEXT, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
            if (y >= s_new_vis_top - 6.0f && y + 40.0f <= s_new_vis_bot + 6.0f) hit(b, H_NEW_ACT + i);
            y += 56.0f;
        }
    }
    s_d->PopAxisAlignedClip();

    if (s_new_scroll_max[p] > 0.0f) {
        const float track = viewH, thumb = track * viewH / (total + 6.0f);
        const float at = top + (track - thumb) * (s_new_scroll[p] / s_new_scroll_max[p]);
        fill(R(NC_PANEL_W - 8, top, 3, track), 0x1E262E, 0.9f);
        fill(R(NC_PANEL_W - 8, at, 3, thumb), NC_ACCENT, 0.85f);
    }
}

static void draw_new_shell(void)
{
    new_background();
    if (new_page() != 0) fill(R(0, 0, WIN_W, WIN_H), 0x000000, 0.50f);

    if (new_page() == 0) {
        fill(R(24, 20, 60, 60), 0x141A20, 0.92f);
        frame(R(24, 20, 60, 60), NC_GOLD, 0.8f, 2.0f);
        text(L"Gc", s_tf_display_md, R(24, 20, 60, 60), NC_RED, 1.0f, DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    {
        const float y = 24.0f;
        const wchar_t* ver = W("v" AR_VERSION);
        const wchar_t* lang = L"ESPA\x00D1OL";
        const float wl = snap(text_w(lang, s_tf_small) + 26.0f);
        const float wv = snap(text_w(ver, s_tf_small) + 26.0f);
        new_chip(ver, WIN_W - 24 - wl - 10 - wv, y, 1.0f);
        new_chip(lang, WIN_W - 24 - wl, y, 1.0f);
    }

    const int page = new_page();
    if (page == 0) {
        new_hero(1.0f, 1);
        new_home_ready();
    } else {
        new_hero(0.32f, 0);
        new_left_page(page);
    }

    new_nav();
    new_dropdown();
}
