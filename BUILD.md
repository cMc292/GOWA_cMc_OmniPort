# GOWA 0.2.0 full — build autocontenido

Esta carpeta es un proyecto independiente. No necesita el paquete alpha.15 ni
`C:\deepseek harness\GOWA-cMc\Salida`.

## Dependencias locales

```text
GOWA.elf
recompiled/
spu/
ps3recomp/
_src/ffmpeg/                                        <- FFmpeg del repo`n_src/sdl3/SDL3-3.4.20/                           <- SDL3 para mandos PS4/5
tools/python/python.exe
local.json
```

`local.json` apunta a la FFmpeg local y al DXC local. Solo `game_dir` sale fuera:
es tu copia legítima de `BCUS98232` 1.12.

## Regla obligatoria de FFmpeg

El build exige un FFmpeg completo (la versión que traiga Ascension-R), con:

```text
include/libavcodec/avcodec.h
lib/avcodec.lib
bin/avcodec-62.dll
bin/ffmpeg.exe
```

`scripts/gowa.py build` comprueba `ffmpeg.exe -version` y aborta si falta
FFmpeg o está incompleto. No se fija una versión concreta.

## Compilar

Desde esta carpeta:

```powershell
$env:GOWA_ELF = "$PWD\GOWA.elf"
tools\python\python.exe scripts\gowa.py build --jobs 8
```

El ejecutable sale en `build\GOWA-cMc.exe`. La búsqueda de FFmpeg del ejecutable
usa `_src\ffmpeg` junto a esta carpeta, así que la cinemática funciona sin añadir
nada al PATH.

## Limpieza

No borres `_src`, `recompiled`, `spu`, `ps3recomp` ni `tools\python` mientras
quieras reconstruir. El ejecutable ya compilado también los necesita para
`GOWA.elf`/FFmpeg en tiempo de ejecución.
