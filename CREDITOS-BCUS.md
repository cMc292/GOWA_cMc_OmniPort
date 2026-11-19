# Créditos — God of War: Ascension [BCUS98232] (PC Static Recompilation)

Versión del port: **v0.1.0-alpha.10** · Base Ascension-R: **v0.1.0-alpha.10** · BCUS98232 v1.12 · Fecha: 2026-11-13

---

## 1. Port original (motor de recompilación estática)

| Proyecto | Autoría | Licencia |
|---|---|---|
| **Ascension-R** — port nativo de *God of War: Ascension* (PS3) por recompilación estática | **Ryan Bagas** (GitHub [`rigaz29`](https://github.com/rigaz29/Ascension-R)) | MIT — «Copyright (c) 2026 Ryan Bagas» |
| **ps3recomp** — toolkit y runtime: lifters PPU/SPU, HLE del sistema, RSX → Direct3D 12 | **Ned Heller** ([`sp00nznet`](https://github.com/sp00nznet/ps3recomp)) y colaboradores | MIT |
| **`rigaz29/ps3recomp`, rama `gowa`** — fork del que este port toma el runtime | Ryan Bagas | MIT |

El crédito del motor y del port original es, por tanto, de **Ryan Bagas (Ascension-R)** y de
**Ned Heller / sp00nznet y colaboradores (ps3recomp)**.

## 2. Port a BCUS98232 (esta versión)

**cMc-OmniPort (cMc292)** — adaptación del port a la versión de **disco US `BCUS98232` v1.12**.

Trabajo realizado en esta versión:

- Verificación por SHA-256 de que el `GOWA.elf` de BCUS98232 1.12 es **el mismo ejecutable** que el de NPUA80918
  1.12, lo que permite reutilizar el lift sin recompilar el juego.
- Ruta de descifrado del SELF NPDRM de parche mediante el **KLIC del parche 1.12**, inyectado por stdin en
  `rpcs3.exe --decrypt` (RPCS3 no lo descifra sin licencia).
- Vista fusionada **disco + parches 1.04–1.12** (con hardlinks: 36,8 GB sin gastar espacio).
- Adaptación del pipeline: título por entorno en `gowa.py`, compilación desde una unión sin corchetes (el
  `[BCUS98232]` rompe el `file(GLOB)` de CMake), `game_files.inc` regenerado para la copia de disco.
- Identidad BCUS98232 en `port/game_find.cpp` y `port/launcher.cpp`.
- Integración de **FFmpeg** (cinemáticas y audio comprimido) y arranque verificado: entry OPD `0x8283F8`,
  `libavcodec 62.28.103`, `cellVdec` decodificando, WASAPI 48 kHz, 49k–88k draws por sesión.
- **Interfaz nueva del launcher** (estilo SpartanPort, paleta de *Ascension*, en español): 10 páginas, visor de
  log y galería de trofeos con nombres y descripciones del `TROPHY.TRP` y progreso del `TROPUSR.DAT`.
- **Pantalla de precompilación** (lectura de archivos y shaders) con el estilo del launcher nuevo (F7), y
  navegación corregida: LOG y TROFEOS se abren con el ratón y la pestaña activa pliega a HOME.
- **Base Ascension-R v0.1.0-alpha.9 + v0.1.0-alpha.10** integrada: arreglo del streaming de texturas (corte de mips)
  y de la copia de texturas en la GPU, relectura completa con F9 (`[tex]`), parón tras los logos (vertex pass),
  DXC en Linux y build del código del juego más rápido.
- Lanzadores **nativos .exe**, empaquetado portable con FFmpeg, MinGit y Python incluidos, y modo de actualización
  (`Update-BCUS` / `Promote-BCUS` / `Rollback-BCUS`).

## 3. Herramientas y bibliotecas de terceros

- **FFmpeg** (LGPL) — build shared `win64-lgpl-shared` de [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds); decodificación de vídeo y audio.
- **RPCS3** (GPL-2.0) — usado como herramienta externa para descifrar el `GOWA.SELF`; no forma parte del ejecutable.
- **DirectX Shader Compiler (DXC)** y **Windows SDK** de Microsoft — compilación de shaders.
- **MinGit** (Git for Windows, GPL-2.0) — git portable incluido para el modo de actualización.
- **Python 3.12 embebido** (PSF) y **capstone** (BSD) — scripts del modo de actualización.
- **Runtime de Visual C++** (msvcp140 / vcruntime140) — redistribuible de Visual Studio 2022.
- **zlib**, **Zstandard**, **SMAA**, **AMD FSR 1/3.1**, **Cinzel** (OFL) y **Silkscreen** (OFL) — ver
  `THIRD_PARTY_NOTICES.md`.

## 4. Reconocimientos

- **MaChInEgUn3** — [`God-of-War-Ascension-Recompiled`](https://github.com/MaChInEgUn3/God-of-War-Ascension-Recompiled)
  y **arnaldolima1** — [`gow-ascension-pc`](https://github.com/arnaldolima1/gow-ascension-pc): proyectos hermanos
  sobre ps3recomp que fueron referencia.
- Comunidad de **RPCS3**: sus utilidades de descifrado y sus notas sobre el formato SELF/PKG hicieron posible el
  paso del KLIC.

## 5. Aviso legal

- Este paquete **no contiene ni distribuye archivos del juego**. El ejecutable contiene código traducido del juego:
  es una obra derivada y se ofrece para uso de quien posee una copia legítima; la licencia MIT del código propio
  no la cubre.
- *God of War* es una marca de Sony Interactive Entertainment LLC / Santa Monica Studio. Este proyecto no está
  afiliado ni respaldado por ellos.
- Se mantienen los avisos de copyright de los proyectos de origen (MIT de Ascension-R y ps3recomp, LGPL de FFmpeg,
  y las licencias del resto de componentes).

## 6. Cómo citar esta versión

> «God of War: Ascension [BCUS98232] (PC Static Recompilation)». Port a BCUS98232 por **cMc-OmniPort (cMc292)**,
> sobre **Ascension-R** de **Ryan Bagas** y **ps3recomp** de **Ned Heller (sp00nznet)** y colaboradores.
