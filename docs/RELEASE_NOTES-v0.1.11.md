# GOWA_cMc_OmniPort v0.1.11

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.11** · juego base **1.12**

## Correcciones de esta versión

- El paquete incluye **todas** las DLL de FFmpeg del build:
  `avcodec`, `avutil`, `swresample`, `avformat`, `avfilter`, `swscale` y `avdevice`.
- Incluye el **caché de shaders precompilado** del full, por lo que la primera
  carga se comporta como el full y no se queda compilando en la pantalla de
  inicio.
- El ejecutable es **exactamente** el del full:
  `work\0.2.0-full\build\GOWA-cMc.exe`.
- No se aplica ningún parche PE al ejecutable.

## Primera ejecución

1. Descomprime el ZIP.
2. Ejecuta `GOWA-cMc.exe`.
3. Elige la carpeta que contiene `PS3_GAME`.
4. El launcher usa el Update `Update\BCUS98232` y genera `GOWA.elf`.
5. La consola se oculta en cuanto existe `GOWA.elf`.
6. Pulsa **PLAY**.

El paquete **no incluye `GOWA.elf`** ni archivos del juego.

## Contenido

- Launcher cMc de 11 páginas, textos en 6 idiomas.
- Detección de disco y Update; `vfs`/`hdd0` correctos.
- Pixel shaders con DXC y caché precompilado.
- Panel GC y F10, vibrance y contraste.
- Mandos Xbox/PlayStation, mods, guardados y ultrawide.
- Runtime VC, SDL3, FFmpeg completo y DXC.
- Configuración limpia, sin rutas absolutas.

## Verificación

```text
Archivo: GOWA_cMc_OmniPort v0.1.11.zip
ZIP SHA-256: 8A2F30BB8EE466E46D3E32D4CED4F1EFABCCEADE939E4B7844D9CAEA5E7AFAE3
Exe SHA-256: A6C3C22E9C7433846773B20C56F0EA47D52BA3DE85556BAE72FD4A48BEF43601
GOWA.elf incluido: NO
```

El primer vídeo se comprobó en el paquete:

```text
[cellVdec] Open -> handle=1
[cellVdec] decoding with libavcodec
[cellVdec] StartSeq(handle=1)
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

Repositorio y versiones:
<https://github.com/cMc292/GOWA_cMc_OmniPort>
