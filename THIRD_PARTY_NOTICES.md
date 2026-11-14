# Avisos de terceros — GOWA_cMc_OmniPort

Este archivo cubre el **paquete de ingeniería `v0.2.0`** (parches, `source_integrated`, informes y
evidencia) y el repositorio. El **paquete portable prebuilt** (`v0.1.0-alpha.10`, con binarios y
librerías) lleva además FFmpeg, MinGit, Python, MSVC y DXC; sus textos están en su propia release y en
`SOURCE_OFFER.txt` de ese paquete.

---

## 1. Ascension-R — MIT

Base del port (motor de recompilación estática). Copyright (c) 2026 **Ryan Bagas**.

```
MIT License

Copyright (c) 2026 Ryan Bagas

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## 2. ps3recomp — MIT

Toolkit y runtime (lifters PPU/SPU, HLE del sistema, RSX → Direct3D 12). Copyright (c) 2026 **Ned Heller /
sp00nznet** y colaboradores. El release aplica parches sobre su rama `gowa` (`09fece8`).

```
MIT License

Copyright (c) 2026 Ned Heller / sp00nznet

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## 3. zlib 1.3.1 — licencia zlib

Se distribuyen sin cambios en `source_integrated\third_party\zlib\` los ficheros que necesita el motor PSARC:
`compress.c`, `deflate.c`, `deflate.h`, `trees.c`, `trees.h`. Copyright (C) 1995-2024 **Jean-loup Gailly y
Mark Adler**. No se modifican; solo se compilan con el resto del port.

```
zlib License

Copyright (C) 1995-2024 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied warranty.  In no event will the
authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the
   original software. If you use this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being
   the original software.
3. This notice may not be removed or altered from any source distribution.
```

## 4. Fuentes Cinzel y Silkscreen — SIL Open Font License 1.1

El launcher compila dentro del ejecutable **Cinzel** (display serif) y **Silkscreen** (navegación). En este
paquete **no se distribuyen los ficheros de fuente** (van como recursos del `.exe` en el árbol de la base), así
que aquí no hay obligación de acompañar los textos; si distribuyes el ejecutable compilado, incluye:

- **Cinzel** — Copyright (c) Natanael Gama, SIL OFL 1.1 (`third_party/cinzel/OFL.txt` en Ascension-R).
- **Silkscreen** — Copyright 2001 The Silkscreen Project Authors, SIL OFL 1.1
  (`third_party/silkscreen/OFL.txt` en Ascension-R).
- Texto de la licencia: <https://openfontlicense.org/open-font-license-official-text/>

## 5. Componentes que este paquete NO distribuye

- **FFmpeg** (LGPL): lo aporta el árbol de la base para las cinemáticas; este paquete no incluye DLL ni
  ejecutables de FFmpeg. Si construyes y distribuyes un binario con FFmpeg, aplican LGPL y la oferta de
  fuente de ese build.
- **MinGit, Python, capstone, MSVC y DXC**: solo estaban en el paquete portable prebuilt
  (`v0.1.0-alpha.10`), no aquí.
- **RPCS3** (GPL-2.0): herramienta externa para descifrar tu copia; no forma parte del port.

## 6. Material del juego incluido por decisión del autor

No es una licencia, es un aviso legal. El paquete `v0.2.0` incluye:

- `assets\launcher-bg.jpg` y `assets\launcher-logo.png`: **arte de *God of War: Ascension* y su logo**, de
  Santa Monica Studio / Sony Interactive Entertainment LLC. **No** están cubiertos por la MIT; se incluyen
  asumiendo el riesgo y se retiran borrando `assets\` (la HOME vuelve al fondo procedural y al wordmark de
  texto, sin tocar código).
- `report\evidencia\*.png`: capturas del launcher y del juego, con el mismo aviso.

El **código traducido del juego** que genera `gowa.py build` tampoco se distribuye: se genera en tu PC y no
está cubierto por la MIT.

## 7. Base de conocimiento (`knowledge\`)

`knowledge\index.sqlite` contiene **solo metadatos** (rutas, tamaños y SHA-256) de las releases de
referencia usadas para comparar: no incluye su código ni sus datos. Los informes de `knowledge\reports\`
resumen diferencias y decisiones; no reproducen código de terceros. Nada de eso concede licencia alguna
sobre esos proyectos.

---

*God of War* es una marca de **Sony Interactive Entertainment LLC**. Este proyecto es de aficionados, sin
ánimo de lucro, y **no está afiliado ni respaldado** por Sony.
