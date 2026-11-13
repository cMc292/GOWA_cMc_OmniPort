# Alcance de la licencia y avisos de terceros

El archivo [`LICENSE`](LICENSE) es la licencia **MIT** del código propio de este repositorio: el lanzador nativo,
los scripts del modo de actualización, la adaptación a la versión de disco `BCUS98232`, la documentación y el
empaquetado portable. Copyright (c) 2026 **cMc292 / cMc-OmniPort**.

## Qué NO cubre la licencia

- ***God of War: Ascension*** ni ningún archivo hecho a partir de él: pertenecen a **Sony Interactive Entertainment
  LLC / Santa Monica Studio**.
- El **código traducido del juego** (el ejecutable del port que va dentro del paquete y las fuentes que genera el
  lift): es una **obra derivada** del juego y no se licencia aquí. Se ofrece para usarse con una copia legítima y
  sigue perteneciendo a sus titulares.
- Los **componentes de terceros**, que mantienen sus propias licencias (FFmpeg LGPL, MinGit GPL-2.0, Python PSF,
  capstone BSD, DXC, zlib, Zstandard, SMAA, AMD FSR, Cinzel y Silkscreen OFL, runtime de Visual C++). El detalle está
  en [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

## Código de origen (MIT) que este proyecto usa y acredita

| Proyecto | Autoría | Licencia |
|---|---|---|
| **Ascension-R** — port nativo de *God of War: Ascension* (PS3) por recompilación estática | **Ryan Bagas** ([`rigaz29`](https://github.com/rigaz29/Ascension-R)) | MIT — «Copyright (c) 2026 Ryan Bagas» |
| **ps3recomp** — toolkit y runtime (lifters PPU/SPU, HLE, RSX → D3D12) | **Ned Heller / sp00nznet** y colaboradores ([`sp00nznet`](https://github.com/sp00nznet/ps3recomp)) | MIT — «Copyright (c) 2026 Ned Heller / sp00nznet» |
| **`rigaz29/ps3recomp`, rama `gowa`** — fork del runtime | Ryan Bagas | MIT |

Este repositorio **no** contiene el árbol fuente de esos proyectos (vive en el paquete, como asset de la release, y
en sus propios repositorios). Se conservan sus avisos de copyright conforme a la licencia MIT.

## Marcas

*God of War* es una marca de **Sony Interactive Entertainment LLC**. Este proyecto es de aficionados, sin ánimo de
lucro, y **no está afiliado ni respaldado** por Sony.
