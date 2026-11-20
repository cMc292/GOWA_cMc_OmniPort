# GOWA_cMc_OmniPort v0.1.10

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.10** · juego base **1.12**

## Corrección de esta versión

- El paquete incluye **todas** las DLL de FFmpeg que usa el build:
  `avcodec`, `avutil`, `swresample`, `avformat`, `avfilter`, `swscale` y `avdevice`.
- Se corrige el primer vídeo/cinemática que fallaba en los paquetes anteriores.
- El ejecutable del paquete es exactamente el del full:
  `work\0.2.0-full\build\GOWA-cMc.exe`.
- No se aplica ningún parche PE al ejecutable.

## Consola

- La consola se oculta si `GOWA.elf` ya existe.
- Se muestra solo si falta `GOWA.elf`, para poder descifrarlo.
- Al terminar de generarlo, se oculta.
- Puede haber un parpadeo inicial; una build GUI real lo eliminaría.

## Primera ejecución

1. Descomprime el ZIP.
2. Ejecuta `GOWA-cMc.exe`.
3. Elige la carpeta que contiene `PS3_GAME`.
4. El launcher usa el Update `Update\BCUS98232` y genera `GOWA.elf`.
5. Cuando HOME muestre `GOWA.elf listo`, pulsa **PLAY**.

El paquete **no incluye `GOWA.elf`** ni archivos del juego.

## Contenido

- Launcher cMc de 11 páginas, textos en 6 idiomas.
- Detección de disco y Update; `vfs`/`hdd0` correctos.
- Pixel shaders con DXC.
- Panel GC y F10, vibrance y contraste.
- Mandos Xbox/PlayStation, mods, guardados y ultrawide.
- Runtime VC, SDL3, FFmpeg completo y DXC.
- Configuración limpia, sin rutas absolutas.

## Aviso legal

Este release no contiene archivos del juego: ni `PS3_GAME`, ni `*.psarc`, ni
`EBOOT`, ni vídeos, ni trofeos, ni claves o licencias de la consola.

El ejecutable contiene código traducido del juego, obra derivada de
*God of War: Ascension*: se ofrece para usarse con una copia legítima y la
licencia MIT del proyecto no la cubre.

*God of War* es una marca de Sony Interactive Entertainment LLC /
Santa Monica Studio. Proyecto de aficionados, sin ánimo de lucro, no afiliado
a Sony.

Repositorio y versiones:
<https://github.com/cMc292/GOWA_cMc_OmniPort>
