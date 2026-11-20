# GOWA_cMc_OmniPort — Paquete portable

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
**Versión del paquete: {{VERSION}}**

Repositorio y versiones nuevas:
<https://github.com/cMc292/GOWA_cMc_OmniPort>

> **IMPORTANTE — ESTE PAQUETE NO INCLUYE EL JUEGO.**
> No contiene `PS3_GAME`, `*.PSARC`, `EBOOT`, vídeos ni trofeos.
> Necesitas tu propia copia de **God of War: Ascension BCUS98232 v1.12**,
> con el disco y sus parches, y tu `GOWA.elf` descifrado.
> El ejecutable contiene código traducido del juego: es una obra derivada,
> no está cubierta por la licencia MIT y sigue perteneciendo a sus titulares.
> Este proyecto no está afiliado a Sony.

## Requisitos

- Windows 10/11 x64.
- GPU compatible con Direct3D 12.
- Tu copia legítima de **BCUS98232 1.12**.
- Tu `GOWA.elf` descifrado. El paquete incluye el `GOWA.elf` de trabajo
- No hace falta instalar Visual C++, FFmpeg ni DXC: van incluidos.

## Instalación

> **No ejecutes la release desde `work\dist`.** Extrae el ZIP en una ruta
> corta, por ejemplo `C:\GOWA` o el Escritorio. Windows y el juego usan
> rutas con límite de 260 caracteres; una ruta profunda puede impedir que
> se abran WAD, vídeos o parches y dejar la carga bloqueada.


1. Descomprime el ZIP en una carpeta con permisos de escritura.
2. Ejecuta `GOWA-cMc.exe`.
3. En **SISTEMA**, elige la carpeta del juego.
   - Debes elegir la carpeta que contiene **`PS3_GAME`**.
   - **No elijas `Update\BCUS98232`**: esa carpeta no tiene `ARCHIVES` y el
     juego se cerraría después de la pantalla de carga.
4. Deja que el launcher prepare el juego.
5. La primera vez se compilan los shaders. Puede tardar varios minutos.
6. Cuando HOME muestre `Pixel shaders: listos`, pulsa **PLAY**.

## Qué incluye

- `GOWA-cMc.exe` — launcher y juego.
- `GOWA.elf` — ELF descifrado, si el paquete lo incluye.
- `SDL3.dll` — mandos Xbox y PlayStation.
- FFmpeg (`avcodec`, `avutil`, `swresample`) — cinemáticas.
- `dxcompiler.dll` y `dxil.dll` — compilación de pixel shaders.
- Runtime de Visual C++ — necesario en PCs limpias.
- `ascension-r.ini` y `gowa-cmc.ini` — configuración sin rutas absolutas.
- `LICENSE`, `CREDITOS.md`, `THIRD-PARTY.txt` y
  `THIRD_PARTY_NOTICES.md`.

## Controles y funciones

- **F10** — mostrar u ocultar el panel GC de rendimiento.
- **F11** — ventana / borderless.
- **F12** — captura de pantalla.
- **F9** — releer texturas.
- **ESC** — cerrar el juego, si está activado en SISTEMA.
- Launcher cMc con páginas de gráficos, pantalla, controles, parches,
  cheats, mods, sistema, avanzado, log y trofeos.
- Guardados con copias de seguridad.
- Mods por carpetas o `.zip`.
- Ultrawide, FSR, F10, panel GC, vibrance y contraste.
- Audio estable y música de la pantalla de precarga.

## Solución de problemas

### El juego se cierra después de compilar shaders

Estás apuntando `vfs` al Update. Selecciona la carpeta del **disco**
(la que contiene `PS3_GAME`).

### No reconoce el mando PlayStation

Comprueba que `SDL3.dll` está junto a `GOWA-cMc.exe`. El paquete ya lo
incluye. Xbox usa XInput; PS4/PS5 usa SDL3.

### No compilan los pixel shaders

Comprueba que están junto al ejecutable:

```text
dxcompiler.dll
dxil.dll
```

y el runtime:

```text
MSVCP140.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
CONCRT140.dll
```

### HOME dice `Pixel shaders: pendientes`

Es normal en la primera ejecución. Deja terminar el build; después debería
mostrar `Pixel shaders: listos`.

## Legal y créditos

- Motor y port original: **Ascension-R**, Ryan Bagas (MIT).
- Toolkit y runtime: **ps3recomp**, Ned Heller / sp00nznet y colaboradores (MIT).
- Port a BCUS98232: **cMc-OmniPort**.
- FFmpeg (LGPL), DXC, MinGit, Cinzel (OFL) y runtime de Microsoft.
- **No se distribuyen archivos del juego.**

Lee `CREDITOS.md`, `LICENSE`, `THIRD-PARTY.txt` y
`THIRD_PARTY_NOTICES.md` antes de redistribuir.

## Información de la build

```text
Paquete:      {{VERSION}}
Ejecutable:   {{EXE_SHA256}}
Compilado:    {{DATE}}
Repositorio:  https://github.com/cMc292/GOWA_cMc_OmniPort
```

## Aviso legal

Este release no contiene archivos del juego: ni `PS3_GAME`, ni `*.psarc`, ni
`EBOOT`, ni vídeos, ni trofeos, ni claves o licencias de la consola.

El ejecutable contiene código traducido del juego, obra derivada de
*God of War: Ascension*: se ofrece para usarse con una copia legítima y la
licencia MIT del proyecto no la cubre.

*God of War* es una marca de Sony Interactive Entertainment LLC /
Santa Monica Studio. Proyecto de aficionados, sin ánimo de lucro, no afiliado
a Sony.

> **Nota:** si el paquete incluye GOWA.elf, ese ELF de trabajo debe retirarse
> antes de publicar para que el aviso anterior sea estrictamente literal.
