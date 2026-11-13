# Créditos — GOWA_cMc_OmniPort

**God of War: Ascension [BCUS98232] — port nativo a Windows x64 por recompilación estática.**
Adaptación a la versión de **disco US `BCUS98232` v1.12** por **cMc-OmniPort (cMc292)**.

Versión: **v0.1.0-alpha.8** · Base: **Ascension-R v0.1.0-alpha.8** (`e2d5d31`) · Rama del port: `bcus` (`5faccc9`)

---

## 1. Port original (motor de recompilación estática)

| Proyecto | Autoría | Licencia |
|---|---|---|
| **Ascension-R** — port nativo de *God of War: Ascension* (PS3, NPUA80918 1.12) por recompilación estática | **Ryan Bagas** (GitHub [`rigaz29`](https://github.com/rigaz29/Ascension-R)) | MIT — «Copyright (c) 2026 Ryan Bagas» |
| **ps3recomp** — toolkit y runtime: lifters PPU/SPU, HLE del sistema, RSX → Direct3D 12 | **Ned Heller** ([`sp00nznet`](https://github.com/sp00nznet/ps3recomp)) y colaboradores | MIT |
| **`rigaz29/ps3recomp`, rama `gowa`** — fork del runtime que usa este port | Ryan Bagas | MIT |

El crédito del motor y del port original es de **Ryan Bagas (Ascension-R)** y de **Ned Heller / sp00nznet y
colaboradores (ps3recomp)**.

## 2. Port a BCUS98232 (este paquete)

**cMc-OmniPort (cMc292)** — adaptación del port a la versión de **disco US `BCUS98232` v1.12** y empaquetado
portable para Windows.

Trabajo realizado:

- Verificación por SHA-256 de que el `GOWA.elf` de `BCUS98232` 1.12 es **el mismo ejecutable** que el de
  `NPUA80918` 1.12 (`4bf75c89…183462b5`): permite reutilizar el lift del port original sin volver a levantar el juego.
- Ruta de descifrado del SELF **NPDRM de parche** mediante el **KLIC del parche 1.12**, inyectado por *stdin* en
  `rpcs3.exe --decrypt` (RPCS3 no lo descifra sin licencia).
- Vista fusionada **disco + parches 1.04–1.12** (36,8 GB sin duplicar archivos, con hardlinks).
- Adaptación del pipeline de Ascension-R: título por entorno en `gowa.py`, `game_files.inc` regenerado desde la copia
  de disco, compilación desde una unión sin corchetes (`[BCUS98232]` rompe el `file(GLOB)` de CMake).
- Identidad `BCUS98232` en `port/game_find.cpp` y `port/launcher.cpp`; título de ventana con los créditos.
- Integración de **FFmpeg** (cinemáticas y audio comprimido) y arranque verificado: entry OPD `0x8283F8`,
  `libavcodec 62.28.103`, `cellVdec` decodificando, WASAPI 48 kHz, 41 000–88 000 draws por sesión.
- **Interfaz nueva del launcher** (estilo *SpartanPort*, paleta de *Ascension*, textos en español): 10 páginas
  (HOME, GRÁFICOS, PANTALLA, CONTROLES, PARCHES, CHEATS, SISTEMA, AVANZADO, LOG, TROFEOS), navegación inferior,
  panel de ajustes con widgets reales, fuente **Cinzel** (OFL) incrustada, **visor de log** y **galería de trofeos**
  con nombres y descripciones del `TROPHY.TRP` en el idioma del juego y progreso leído del `TROPUSR.DAT`.
  La interfaz clásica sigue disponible con `[launcher] ui = classic`.
- Corrección de las ventanas del launcher y del juego cuando se lanzan con la consola oculta.
- **Lanzadores nativos `.exe`** (sin consola al jugar, icono y créditos embebidos) que calculan sus rutas desde su
  propia carpeta.
- **Empaquetado portable e independiente**: FFmpeg, MinGit y Python 3.12 embebidos, y **modo de actualización**
  (`Update-BCUS` / `Promote-BCUS` / `Rollback-BCUS`) dentro del paquete.
- Renombrado del producto: **`GOWA-cMc.exe`**, ajustes en **`build/gowa-cmc.ini`**, marca **GOWA-CMC** en el launcher.

Estos créditos aparecen también en el **título de la ventana** del juego y del launcher:

```
GOW Ascension [BCUS98232] - port: cMc-OmniPort | Ascension-R: Ryan Bagas + ps3recomp: sp00nznet
```

## 3. Herramientas y bibliotecas de terceros

| Componente | Uso | Licencia |
|---|---|---|
| **FFmpeg** (build shared `win64-lgpl-shared` de [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds), n8.1) | decodificación de vídeo y audio | LGPL 2.1+ (DLLs compartidas + `LICENSE.txt`) |
| **RPCS3** | herramienta externa para descifrar el `GOWA.SELF`; **no** forma parte del ejecutable | GPL-2.0 |
| **MinGit** (Git for Windows, portable) | git del modo de actualización | GPL-2.0 (con sus licencias incluidas) |
| **Python 3.12** embebido + **capstone** | scripts del modo de actualización | PSF / BSD |
| **DirectX Shader Compiler (DXC)**, Windows SDK | compilación de shaders | UIUC/NCSA y términos del SDK |
| **zlib**, **Zstandard**, **SMAA**, **AMD FSR 1/3.1**, **Cinzel** (OFL), **Silkscreen** (OFL) | componentes del árbol de Ascension-R | ver `THIRD_PARTY_NOTICES.md` |

## 4. Reconocimientos

- **MaChInEgUn3** — [God-of-War-Ascension-Recompiled](https://github.com/MaChInEgUn3/God-of-War-Ascension-Recompiled)
  y **arnaldolima1** — [gow-ascension-pc](https://github.com/arnaldolima1/gow-ascension-pc): proyectos hermanos
  sobre ps3recomp que fueron referencia.
- Comunidad de **RPCS3**: sus utilidades de descifrado y sus notas sobre el formato SELF/PKG hicieron posible el paso
  del KLIC.

## 5. Aviso legal

- Este paquete **no contiene ni distribuye archivos del juego**. El ejecutable contiene código traducido del juego y
  es una **obra derivada**: se ofrece para uso de quien posee una copia legítima y **no** queda cubierto por la
  licencia MIT del código propio.
- *God of War* es una marca de **Sony Interactive Entertainment LLC / Santa Monica Studio**. Este proyecto no está
  afiliado ni respaldado por ellos.
- Se mantienen los avisos de copyright de los proyectos de origen (MIT de Ascension-R y ps3recomp, LGPL de FFmpeg y
  las licencias del resto de componentes).

## 6. Cómo citar esta versión

> «God of War: Ascension [BCUS98232] (PC Static Recompilation)». Port a `BCUS98232` por **cMc-OmniPort (cMc292)**,
> sobre **Ascension-R** de **Ryan Bagas** y **ps3recomp** de **Ned Heller (sp00nznet)** y colaboradores.
