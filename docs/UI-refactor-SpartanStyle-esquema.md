# Esquema de refactor de la UI — estilo SpartanPort

**Proyecto:** GOWA_cMc_OmniPort (God of War: Ascension [BCUS98232], port de cMc-OmniPort)
**Alcance:** **solo estilo y distribución/layout.** Todas las funciones actuales siguen operativas y con los mismos
ajustes. No se toca el runtime del juego, ni el esquema de `gowa-cmc.ini`, ni el modo de actualización.

---

## 0. Resumen ejecutivo

La UI actual del launcher es una barra lateral (Play, Graphics, Display, Controls, Game patches, Cheats, System) con
escena neón de fondo. Propongo sustituirla por el **lenguaje visual de SpartanPort**: una **pantalla con
arte a sangre**, **wordmark + PLAY grande a la derecha**, **chips de estado**, y **navegación inferior** con páginas
que scrollean sobre un panel semitransparente a la izquierda.

La implementación es **cosmética/estructural**: los widgets se redibujan, las páginas se reorganizan, pero cada
control sigue leyendo y escribiendo la misma clave de `gowa-cmc.ini` que hoy. Cero migración de ajustes.

Esfuerzo estimado: **9–12 días de trabajo** en 5 fases, con matriz de paridad y capturas de regresión.

---

## 1. Análisis del estilo de referencia (SpartanPort)

Revisadas sus capturas oficiales (`docs/images/releases/3.0.0/launcher-*.jpg`):

| Elemento | Cómo lo hace SpartanPort |
|---|---|
| Ventana | ~1600×1000, sin marco decorativo; arte de fondo a sangre (retrato oscuro rojo/carmesí con brasas) |
| Cabecera | Monograma arriba-izquierda; arriba-derecha **chips** «Release · 3.0.0 ▾» y «English ▾» |
| Panel de ajustes | Columna izquierda ~45 % del ancho, fondo negro translúcido, scroll fino, título de página en **serif mayúsculas**, subtítulo gris, botón **SAVE** rojo arriba a la derecha |
| Secciones | Encabezados en serif rojo mayúsculas («WINDOW», «UPSCALING», «EFFECTS»), con línea separadora |
| Filas | Etiqueta a la izquierda, control a la derecha (dropdown/checkbox/segmentado), **descripción gris debajo** |
| Columna derecha | Wordmark «SPARTANPORT» (serif, última palabra en rojo) + **PLAY** rojo macizo + chips de estado (✓ Saves found, ✓ CUSA · v01.02, ✓ GPU) + línea de resumen (resolución · fps · modo · monitor) + texto de ayuda |
| Navegación | **Barra inferior** oscura: HOME · GRAPHICS · PERFORMANCE · GAME · TROPHIES · CONTROLS · MODS · ADVANCED · LOG; activo en rojo con barra superior; versión/OS a la derecha |
| Controles | Botón secundario contorneado («Set up a controller»), segmentado con borde rojo en el activo, filas de mapeo con campo + botón de icono + «Clear input», casillas rojas |
| Tipografía | Display **serif** (Trajan/Cinzel-like) para títulos; **sans** de sistema para cuerpo |
| Color | Base casi negra; **carmesí** como único acento; texto hueso; gris apagado; dorado solo en chips |

### Nota legal (importante)

SpartanPort es **GPL-2.0**; nuestro proyecto es **MIT** (Ascension-R + ps3recomp) con arte y código propios. Por tanto:

- ✅ Se copia el **estilo visual y la distribución** (ideas de layout, jerarquía, tokens).
- ❌ **No** se copia código, fuentes, imágenes ni assets de SpartanPort (contaminaría la licencia).
- Las capturas descargadas son **referencia interna** de diseño; no se incluyen en el paquete.
- El fondo será **procedural o aportado por el usuario**, no arte del juego ni de SpartanPort.

---

## 2. Sistema visual propuesto (tokens)

### 2.1 Layout

| Token | Valor |
|---|---|
| Ventana base | 1600 × 1000 DIP (escalable; mínimo 1280 × 800) |
| Panel de ajustes | ancho 720 DIP (45 %), alto completo hasta la barra inferior |
| Columna hero | resto (880 DIP) |
| Barra de navegación | alto 84 DIP, pegada abajo, fondo `#0C0A0A` |
| Cabecera flotante | chips a 24 DIP del borde superior derecho |
| Márgenes internos | 40 DIP laterales, 32 DIP superior, 24 DIP entre secciones |
| Escala GUI | Auto / 1× / 1.5× (respeta DPI; se guarda en `[launcher] gui_scale`) |

### 2.2 Color

| Uso | Valor |
|---|---|
| Fondo base | `#0B0909` |
| Panel ajustes | `#120E0E` al 94 % (viñeta sobre el arte) |
| Acento / PLAY / activo | `#B01B1B` (hover `#C62626`, pressed `#8B0000`) |
| Acento texto (encabezados) | `#E23B3B` |
| Texto principal | `#EDE7E1` |
| Texto secundario | `#9A918A` |
| Descripciones | `#7E7770` |
| Separadores | `#FFFFFF` al 10 % |
| Chips | borde `#C9A227` al 60 %, texto `#D8B968` |
| Éxito / aviso | `#7FB069` / `#E0A030` |
| Sombra de texto sobre arte | `rgba(0,0,0,.65)` |

### 2.3 Tipografía

| Rol | Fuente | Tamaño/peso |
|---|---|---|
| Títulos de página y wordmark | Serif display: **Cinzel** (OFL, incrustada ~60 KB) o, como respaldo, `Georgia` | 34–40 DIP, mayúsculas, tracking +2 % |
| Cuerpo / etiquetas | `Segoe UI` (Windows) | 15–16 DIP regular / semibold |
| Descripciones | `Segoe UI` | 13 DIP |
| Navegación inferior | `Silkscreen` (la que ya tenemos) | 13 DIP mayúsculas, tracking +6 % |
| Botones | `Segoe UI Semibold` / PLAY en serif | 15–17 DIP |

> Alternativa sin incrustar fuentes: todo con `Segoe UI` + `Georgia` del sistema. Recomiendo incrustar **Cinzel**
> (OFL, redistribuible) para que el estilo no dependa del PC.

### 2.4 Formas y movimiento

- Rectángulos rectos; **chips** con radio 999 (píldora); sin sombras difusas (solo viñeta y glow sutil del texto).
- Hover: +6 % luminosidad y borde 1 DIP acento; Focus teclado: contorno 2 DIP `#E23B3B`.
- Transiciones 150 ms (fade/deslizamiento 8 DIP). Se desactivan con `[launcher] effects = off`.
- Brasas/partículas sutiles solo en HOME (reutiliza el sistema de partículas que ya tiene `loading_screen.cpp`).

---

## 3. Arquitectura de información (páginas)

Barra inferior con **8 pestañas** (nombres en mayúsculas como SpartanPort):

`HOME · GRAPHICS · DISPLAY · CONTROLS · PATCHES · CHEATS · SYSTEM · ADVANCED`

Chips globales arriba-derecha: **versión** («0.1.0-alpha.8 ▾» → abre ADVANCED) y **idioma** (solo si se añade i18n;
fase opcional).

Derecha (persistente en todas las páginas, como SpartanPort): arte, wordmark **GOWA-cMc**, **PLAY**, chips de estado
y línea de resumen. En páginas de ajustes el PLAY queda visible pero atenuado; el botón **SAVE** aparece en la
página.

### 3.1 Correspondencia 1:1 con lo que hay hoy

| Página nueva | Controles actuales | Claves de `gowa-cmc.ini` |
|---|---|---|
| **HOME** | Play, presets Smooth/Balanced/Faithful, Ready check, «Game found», Saves, GPU, Controller, resumen | `[display] mode`, `[graphics] render_scale`, `[display] frame_cap` (quick) |
| **GRAPHICS** | GPU, escala de render, escalado (FSR/bilinear), nitidez, generación de fotogramas, antialiasing, anisotropía, shaders asíncronos, hilo de render, espera de pipelines | `[graphics]` completas |
| **DISPLAY** | Modo, tamaño de ventana, VSync, límite de FPS, mostrar FPS, ultrawide, aviso de shaders | `[display]` completas |
| **CONTROLS** | Dispositivo, API, zona muerta, botón confirmar, vibración + hoja de teclas actual | `[input]` completas |
| **PATCHES** | 5 parches conmutables + contador «4 de 5 activos» | `[patches]` completas |
| **CHEATS** | Daño recibido/hecho, orbes máx., magia/rabia/saltos infinitos | `[cheats]` completas |
| **SYSTEM** | Idioma, ESC sale, copias de guardado, audio estable, altavoces, audio raw, precompilar música, carpeta del juego, rutas VFS/HDD0/FFmpeg, abrir logs, informe de problema | `[system]`, `[paths]`, `[launcher]` |
| **ADVANCED** | Versión, créditos (Ascension-R/Ryan Bagas, ps3recomp/sp00nznet, port/cMc-OmniPort), enlaces, comprobar actualización, **Update/Promote/Rollback** (lanzan los `.exe`), abrir carpeta del paquete | `[launcher] update_check`, `gui_scale` (nuevo) |

**Cero claves nuevas obligatorias.** `gui_scale` es opcional (default `auto`) y no afecta al juego.

### 3.2 Boceto — HOME

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ (monograma)                                   [0.1.0-alpha.8 ▾] [Español ▾]  │
│                                                                              │
│   arte de fondo a sangre (gradiente + brasas; opcional imagen del usuario)   │
│                                                                              │
│                                              GOWA-CmC                        │
│                                        (serif, «CmC» en rojo)                │
│                                        ┌────────────────────┐                │
│                                        │       PLAY         │  ← rojo        │
│                                        └────────────────────┘                │
│                        ✓ Juego BCUS98232 1.12  ✓ Saves 1  ✓ RX 6900 XT      │
│                        1280×720 · 60 FPS · Ventana · Monitor 1               │
│                        ESC cierra · Insert abre menú del port (si aplica)    │
├──────────────────────────────────────────────────────────────────────────────┤
│ HOME  GRAPHICS  DISPLAY  CONTROLS  PATCHES  CHEATS  SYSTEM  ADVANCED   vα.8  │
└──────────────────────────────────────────────────────────────────────────────┘
```

### 3.3 Boceto — GRAPHICS

```
┌───────────────────────────────┬──────────────────────────────────────────────┐
│ GRAPHICS              [SAVE]  │                                              │
│ Ajustes guardados en gowa-cmc.ini                                             │
│                               │        (arte + wordmark + PLAY atenuado)      │
│ GRAPHICS                      │                                              │
│ ────────────────────────────  │                                              │
│ Escala de render   [1× ▾]     │                                              │
│   Aumenta la resolución…      │                                              │
│ Escalado           [FSR 3 ▾]  │                                              │
│ Nitidez            [Alta ▾]   │                                              │
│ Antialiasing       [SMAA ▾]   │                                              │
│ Anisotropía        [4× ▾]     │                                              │
│ ☑ Shaders asíncronos          │                                              │
│ ☐ Generación de fotogramas    │                                              │
│ ────────────────────────────  │                                              │
│ AVANZADO                      │                                              │
│ ☑ Hilo de render              │                                              │
│ ☑ Espera de pipelines         │                                              │
└───────────────────────────────┴──────────────────────────────────────────────┘
```

---

## 4. Catálogo de componentes

| Componente | Especificación |
|---|---|
| `NavBar` | Alto 84; items 13 DIP Silkscreen mayúsculas; activo rojo + barra 2 DIP arriba; hover texto hueso; versión a la derecha |
| `PageTitle` | Serif 36 DIP mayúsculas + subtítulo 14 DIP gris; botón SAVE a la derecha (si hay cambios) |
| `SectionHeader` | Serif rojo 18 DIP mayúsculas + línea separadora 1 DIP |
| `Row` | 64–88 DIP de alto; etiqueta 16 DIP; control derecha; descripción 13 DIP gris (máx. 2 líneas) |
| `Dropdown` | Campo 220×40, fondo `#1A1515`, borde 1 DIP, chevron; menú 40 DIP/item, hover acento |
| `Checkbox` | 18×18, borde 1 DIP, relleno rojo con check blanco |
| `Segmented` | Botones contiguos, activo con borde rojo 2 DIP (presets, dispositivos) |
| `Slider` | Pista 4 DIP, thumb 14 DIP, valor a la derecha |
| `Chip` | Píldora 28 DIP alto, borde dorado 60 %, icono ✓ + texto 13 DIP |
| `ButtonPrimary` | PLAY: 480×72, rojo macizo, texto serif 26 DIP con tracking |
| `ButtonSecondary` | Contorno 1 DIP gris, hover borde acento |
| `Toast` | Abajo-centro, 3 s, fondo `#1A1010`, borde acento (guardado, error, copia hecha) |
| `Modal` | Fondo negro 70 %, caja 640 DIP, botones al pie (confirmar reinicio, créditos, salir) |
| `ScrollPanel` | Scroll fino 4 DIP, rueda y teclado (PgUp/PgDn), suave 150 ms |
| `PendingBadge` | «Cambios sin guardar» junto a SAVE; parpadeo nulo (accesibilidad) |

---

## 5. Implementación técnica

### 5.1 Estado actual

- `port/launcher.cpp` (170 KB) hace todo: ventana, Direct2D, entrada, dibujo de barra lateral y páginas, fuentes
  Silkscreen embebidas, bitmaps de iconos, escena de fondo y el wordmark de píxeles.
- Pestañas actuales: `Play, Graphics, Display, Controls, Game patches, Cheats, System` con iconos.

### 5.2 Refactor propuesto (extraer, no reescribir el mundo)

```
port/ui_theme.h        tokens (colores, tipografías, espaciados, radios)
port/ui_widgets.h/.cpp primitivas: NavBar, Row, Dropdown, Check, Slider, Segmented,
                       Chip, Buttons, Toast, Modal, ScrollPanel, PageTitle, Section
port/ui_pages.h/.cpp   cuerpos de las 8 páginas
port/ui_shell.h/.cpp   layout, columna hero, cambio de página, foco/teclado, animaciones
port/ui_art.cpp        fondo: gradiente + viñeta + brasas; imagen opcional del usuario
port/launcher.cpp      entrada y ventana D2D; enlaza shells y settings (ps3_settings)
```

- `CMakeLists.txt`: añadir los nuevos `.cpp` (los fuentes del port están listados explícitamente; es un cambio de
  ~4 líneas).
- Fuentes: incrustar **Cinzel** (OFL) en `third_party/cinzel/` + aviso en `THIRD_PARTY_NOTICES`; cuerpo con
  `Segoe UI` del sistema (respaldo `Tahoma`).
- Fondo: por defecto **procedural** (gradiente carmesí + viñeta + partículas, reutilizando `brand.cpp`/partículas de
  `loading_screen.cpp`). Opcional: si existe `assets/launcher-bg.png` junto al `.exe`, se usa como arte (decisión del
  usuario; no se distribuye arte del juego).
- Escalado: respetar DPI existente + `gui_scale` auto/1×/1.5.
- Accesibilidad: navegación con Tab/Shift+Tab, Enter/Space, foco visible, contraste ≥ 4.5:1, `effects = off`
  desactiva animaciones.
- El juego (`--no-launcher`) **no se toca**: el launcher es un proceso independiente.

### 5.3 Compatibilidad con el modo de actualización

- Todo va como **commits nuevos en la rama `bcus`** (no en `ps3recomp`, que se resetea en cada update).
- Minimizar el diff en `launcher.cpp`: el grueso vive en archivos nuevos; así un cambie de upstream en
  `launcher.cpp` conflictúa poco.
- `Update-BCUS.cmd` ya reaplica los commits BCUS; si upstream toca `launcher.cpp`, el conflicto se resuelve a mano
  una vez y el resto sigue igual.
- Capturas de regresión y matriz de paridad se guardan en `docs/ui/` del paquete.

---

## 6. Fases de trabajo

| Fase | Entregable | Criterio de aceptación | Días |
|---|---|---|---|
| **F0** | Tokens + fuentes + shell (ventana, panel, hero, navbar) funcionando con una página vacía | Abre a 1600×1000, DPI correcto, sin parpadeo, `effects` respetado | 2 |
| **F1** | Widgets completos + HOME | PLAY arranca el juego; chips de Ready check; presets; SAVE funciona | 2 |
| **F2** | GRAPHICS + DISPLAY + CONTROLS | Cada control escribe/lee su clave; valores por defecto intactos | 3 |
| **F3** | PATCHES + CHEATS + SYSTEM | Paridad exacta con la UI actual; informe de problema sigue funcionando | 2 |
| **F4** | ADVANCED: créditos, versión, rutas, abrir logs, Update/Promote/Rollback, `gui_scale` | Créditos visibles; botones lanzan los `.exe`; sin efectos en el juego | 1–2 |
| **F5** | Pulido: animaciones, toasts, foco teclado, capturas de regresión, docs | Matriz de paridad 100 %; capturas por página; sin regresiones en `--no-launcher` | 1–2 |

**Pruebas por página**

1. **Paridad de ajustes**: guardar cada valor → releer `gowa-cmc.ini` → mismo valor y misma sección. Repetir
   cargando un `.ini` antiguo (migración cero).
2. **Arranque**: PLAY abre el juego con la build activa y la misma configuración que antes.
3. **Regresión del juego**: `GOWA-cMc.exe --no-launcher` sigue arrancando; el launcher no toca su ruta.
4. **Visual**: captura de cada página a 1× y 1.5× guardada en `docs/ui/`.
5. **Accesibilidad**: recorrido completo con teclado; zoom 150 %.

---

## 7. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| Conflictos con upstream en `launcher.cpp` | Extraer a archivos nuevos; diff mínimo; conflicto se resuelve una vez |
| Licencia GPL de SpartanPort | Solo ideas de layout; ni código ni assets; fondo procedural o del usuario |
| Fuentes: el display serif puede no existir | Incrustar Cinzel (OFL) y usar `Georgia` como respaldo |
| Rendimiento Direct2D con muchas capas | Precachear bitmaps de fondo; medir arranque < 1 s; sin sombras difusas |
| Texto pequeño a 4K | `gui_scale` auto + tamaños en DIP |
| Accesibilidad / teclado | Foco visible, Tab orden lógico, atajos documentados |
| Cambiar la UI y romper un ajuste | Matriz de paridad automatizada en F2–F3 |

---

## 8. Decisiones que necesito

1. **Fuente display**: ¿incrustamos **Cinzel** (OFL, resultado idéntico en cualquier PC) o usamos **Georgia** del
   sistema (0 KB añadidos, depende del PC)?
2. **Fondo**: ¿procedural (gradiente+brasas, sin arte de terceros) o quieres aportar tú una imagen
   (`assets/launcher-bg.png`, como hiciste con el icono)?
3. **Idioma**: ¿UI solo en español, o añadimos conmutador ES/EN (el chip de idioma que tiene SpartanPort)?
4. **Alcance extra**: ¿añadimos las pestañas LOG (visor de logs) y TROPHIES (como SpartanPort) o nos quedamos en las
   8 páginas con lo que ya existe? (LOG/TROPHIES serían funciones nuevas, no solo estilo.)

Con tus respuestas a 1–4 empiezo por **F0+F1** (shell + widgets + HOME) y te enseño capturas antes de seguir.
