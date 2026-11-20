# Bitácora de código — GOWA_cMc_OmniPort

**Proyecto:** `God of War: Ascension [BCUS98232]`
**Árbol de desarrollo:** `work/0.2.0-full`
**Paquete de referencia:** `work/dist/GOWA_cMc_OmniPort v0.1.5`
**Última actualización:** 2026-11-19
**Commit del full:** `035dd22`

Esta bitácora es una base de datos de procesos funcionales, decisiones,
errores conocidos y verificaciones. No sustituye al historial de Git: lo
resume para evitar repetir fallos y para saber qué código está validado.

---

## 1. Reglas de la bitácora

- Una fila = un proceso concreto, verificable o reproducible.
- No se borran filas: se añade una nueva y la anterior se marca como
  `sustituido`.
- `verificado` significa que se ejecutó y se comprobó el resultado.
- `implementado` significa que el código existe, pero no hay prueba completa.
- `pendiente` significa que no se ha podido probar en hardware real.
- `no-publicar` significa que el binario o paquete no debe entregarse.
- Nunca se anotan tokens, contraseñas ni rutas de usuario como credenciales.

Estados permitidos:

```text
base | implementado | verificado | corregido | sustituido | pendiente | no-publicar
```

---

## 2. Identidad de binarios

| Elemento | SHA-256 | Estado |
|---|---|---|
| Ejecutable bueno de release | `0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01` | verificado por el usuario |
| Ejecutable actual del `build` | `5FA621E93304C6694A5B07C9D591DFB5CD9E790C878082DFCBA0989E95D06560` | no-publicar |
| Pin de release | `build/GOWA-cMc.known-good.exe` | hardlink al bueno |
| Paquete `v0.1.5` automático | `0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01` | mismo ejecutable del manual |

El empaquetador **no debe** copiar `build/GOWA-cMc.exe` mientras ese hash sea
`5FA6…`. Debe usar el pin `scripts/release-exe.json`.

---

## 3. Procesos funcionales

| ID | Fecha | Módulo | Proceso | Archivos | Verificación | Estado | Commit |
|---|---|---|---|---|---|---|---|
| P-001 | 2026-11-19 | Full | Consolidar el proyecto en un solo árbol autocontenido | `work/0.2.0-full` | Full compila y llega al menú | base | `0a6637d` |
| P-002 | 2026-11-19 | Selección | Separar `vfs` (disco) de `hdd0` (Update) al elegir juego | `port/launcher.cpp` | Prueba de runtime del full | sustituido por P-007 | `130173d` |
| P-003 | 2026-11-19 | UI | Aceptar layout fusionado disco+Update en HOME | `port/launcher.cpp` | Captura HOME verde | sustituido por P-004 | `db673ef` |
| P-004 | 2026-11-19 | Detección | Leer `PARAM.SFO` del disco y del Update y exigir 1.12 | `port/launcher.cpp` | Captura `BCUS98232 · 1.12` | verificado | `e67e77b` |
| P-005 | 2026-11-19 | Empaquetado | Generar portable versionado con README/LEEME | `scripts/make_portable.py` | ZIP y hashes | sustituido por P-013 | `8c652b8` |
| P-006 | 2026-11-19 | Publicación | Workflow de GitHub y guía de publicación | `.github/workflows/release.yml`, `docs/PUBLICAR-GITHUB.md` | Validación de sintaxis | implementado | `2b9f1a3` |
| P-007 | 2026-11-19 | Selección | Si se elige Update, buscar el `PS3_GAME` cercano | `port/launcher.cpp` (`find_disc_near`) | Prueba in-place sin cierre | implementado | `d5fb2c8` |
| P-008 | 2026-11-19 | UI | PLAY sin juego abre SISTEMA, no MODS | `port/launcher.cpp` | Captura de ENTER/PLAY en SISTEMA | verificado | `e64b91e` |
| P-009 | 2026-11-19 | UI | Bloquear PLAY sin juego detectado | `port/launcher.cpp` | Prueba de UI | sustituido por P-011 | `d93157f` |
| P-010 | 2026-11-19 | Empaquetado | Crear estructura vacía `vfs` y `gamedata` | `scripts/make_portable.py` | ZIP generado | implementado | `e0a1653` |
| P-011 | 2026-11-19 | UI | Permitir PLAY si hay rutas y enlaces configurados | `port/launcher.cpp` | Prueba con enlaces temporales | implementado | `0a2b685` |
| P-012 | 2026-11-19 | Selección | Resolver `PS3_GAME` o su carpeta padre con `game_locate` | `port/launcher.cpp` | Código compilado; falta prueba visual | implementado | `2a93c3c` |
| P-013 | 2026-11-19 | Empaquetado | Fijar ejecutable bueno por hash y validarlo | `scripts/make_portable.py`, `scripts/release-exe.json` | Paquete automático con `0703…` | verificado por hash | `9ccfc05` |
| P-014 | 2026-11-19 | Empaquetado | Hardlink del ejecutable bueno dentro de `build` | `build/GOWA-cMc.known-good.exe` | Hash y ZIP | verificado | `035dd22` |
| P-015 | 2026-11-19 | UI cMc | Launcher de 11 páginas, HOME, SISTEMA, MODS, TROFEOS | `port/ui_new.inc`, `port/launcher.cpp` | Capturas HOME/SISTEMA | verificado | base |
| P-016 | 2026-11-19 | Carga | Pantalla de precarga moderna y música original | `port/loading_screen.cpp`, `port/loading_screen_modern.inc` | Uso en ejecución | verificado | base |
| P-017 | 2026-11-19 | Shaders | Precompilación de pixel shaders con DXC | `ps3recomp/libs/video/rsx_d3d12_backend.c`, `port/precompile.cpp` | 49.996 encontrados, 39.597 compilados, 0 fallos | verificado | base |
| P-018 | 2026-11-19 | Rendimiento | Panel GC y F10 para mostrar/ocultar | `ps3recomp/libs/video/rsx_d3d12_backend.c` | Confirmado por el usuario | verificado | base |
| P-019 | 2026-11-19 | Imagen | Vibrance y contraste al final del pase | `ps3recomp/libs/video/rsx_d3d12_backend.c` | Confirmado por el usuario | verificado | base |
| P-020 | 2026-11-19 | Mandos | SDL3 para PS4/PS5 además de XInput | `port/input`, `SDL3.dll` | Confirmado por el usuario | verificado | base |
| P-021 | 2026-11-19 | Guardados | Copias de seguridad de partidas | `port/saves.cpp` | Uso en pruebas | implementado | base |
| P-022 | 2026-11-19 | Mods | Carga de mods por carpeta y `.zip` | `port/mods.cpp` | Prueba de mods | verificado | base |
| P-023 | 2026-11-19 | Pantalla | Ultrawide con HUD/menús a 16:9 | `ps3recomp/libs/video/rsx_d3d12_backend.c` | Implementado; sin monitor ultrawide | pendiente | base |
| P-024 | 2026-11-19 | Publicación | README/LEEME generados con versión y hash | `scripts/README-portable.template.md`, `scripts/LEEME-portable.template.txt` | Revisión del paquete | verificado | `035dd22` |
| P-025 | 2026-11-19 | QA | Sesión larga de juego | 0.1.6 | Confirmado por el usuario: partida larga sin problemas | verificado | 6cff48 |

---

## 4. Registro de fallos conocidos

| ID | Síntoma | Causa | Corrección | Estado |
|---|---|---|---|---|
| F-001 | Full/proyecto con 36 GB duplicados | `build/gamedata` era una copia del juego | Se sustituyó por enlace al Update; no volver a copiar datos del juego | corregido |
| F-002 | Cierre tras compilar shaders | `vfs` apuntaba al Update, que no tiene `ARCHIVES` | `find_disc_near` y pin del ejecutable bueno | corregido en paquete `v0.1.5` |
| F-003 | PLAY abría MODS | La UI nueva usa 7 para SISTEMA; el código usaba 6 | `s_new_page = 7` | corregido |
| F-004 | PLAY arrancaba sin juego y usaba `GOWA.elf` como VFS | Solo se comprobaba el ELF, no las rutas | Guard de PLAY con `vfs`, `hdd0` y enlaces | corregido |
| F-005 | El empaquetador automático producía un portable que no funcionaba | Copiaba `build/GOWA-cMc.exe`, que contiene la regresión `5FA6…` | Pin `0703…` en `release-exe.json` y hardlink | corregido |
| F-006 | Se creía que el paquete automático “perdía” archivos | El `0.2.0-portable` funcional tenía caché y enlaces de una ejecución previa | El caché es estado de ejecución, no payload; el paquete limpio no lo copia | aclarado |
| F-007 | El ejecutable nuevo no se comporta como el bueno | Cambios de detección/guard posteriores al `0703` | No publicar `5FA6…`; usar el pin hasta validar el nuevo | abierto |

---

## 5. Comandos reproducibles

### Compilar el full

```powershell
Push-Location "work\0.2.0-full"
tools\python\python.exe scripts\gowa.py build --jobs 8
Pop-Location
```

### Generar el portable automático

```powershell
Push-Location "work\0.2.0-full"
tools\python\python.exe scripts\make_portable.py --version v0.1.5
Pop-Location
```

El empaquetador lee `scripts/release-exe.json` y verifica el SHA-256.

### Verificar el paquete

```powershell
Get-FileHash `
  "work\dist\GOWA_cMc_OmniPort v0.1.5\GOWA-cMc.exe" `
  -Algorithm SHA256
```

Debe dar:

```text
0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01
```

### Probar sin crear copias

Añadir enlaces temporales dentro del paquete, ejecutar hasta el menú y
borrarlos después. No duplicar la carpeta del paquete y no tocar el full.

---

## 6. Invariantes que no se deben romper

1. El full se compila desde `work/0.2.0-full`.
2. El paquete publicado no contiene el juego ni rutas absolutas.
3. El paquete automático usa el ejecutable fijado en `release-exe.json`.
4. El ejecutable de release actual es `0703…`; `5FA6…` no se publica.
5. `vfs` debe apuntar al disco (`PS3_GAME`), no al Update.
6. `hdd0` debe apuntar al Update.
7. No se borran ni mueven datos del juego.
8. No se crean carpetas de trabajo fuera del proyecto.
9. Toda prueba de portable se hace in-place y se limpia después.
10. No se declara “funciona” sin ejecutar el flujo completo hasta el menú.

---

## 7. Pendientes reales

- Validar el ejecutable nuevo `5FA6…` y actualizar el pin solo si supera la
  prueba completa.
- Probar ultrawide en un monitor real.
- ~~Probar una sesión larga de estabilidad.~~ Confirmada el 2026-11-19 sin problemas.
- Verificar el flujo completo del paquete `v0.1.5` con selección de juego,
  shaders, menú, guardado y reapertura.
- Publicar el release en GitHub con `gh` o el runner self-hosted.

---

## 8. Base de datos CSV

El archivo `bitacora-codigo.csv` contiene las mismas filas con columnas
separadas por coma, listo para abrir en Excel, LibreOffice, SQLite o Python.

Esquema SQL de referencia: `bitacora-codigo.schema.sql`.

Base SQLite lista para consultar: `bitacora-codigo.sqlite`.

Columnas:

```text
id, fecha, modulo, proceso, archivos, verificacion, estado, commit_hash
```

### F-008 — Ruta de ejecución demasiado larga

Síntoma: la release se queda cargando la primera cinemática. Causa: se ejecutó desde `work\dist`, con una ruta profunda. Corrección: extraer a una ruta corta (Escritorio o `C:\GOWA`). Una prueba en el Escritorio con `v0.1.12` funciona correctamente.
