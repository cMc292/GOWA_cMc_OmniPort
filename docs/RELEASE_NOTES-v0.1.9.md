# GOWA_cMc_OmniPort v0.1.9

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.9** · juego base **1.12**

## Cambio principal

- El paquete usa **exactamente** el ejecutable del full:
  `work\0.2.0-full\build\GOWA-cMc.exe`.
- La consola se comporta así:
  - si falta `GOWA.elf`, se muestra para el descifrado;
  - si `GOWA.elf` ya existe, se oculta al arrancar;
  - al terminar de generar `GOWA.elf`, se oculta.
- El paquete **no incluye `GOWA.elf`** ni archivos del juego.
- Puede aparecer un parpadeo inicial de consola: la ocultación se hace desde
  la aplicación. Una build GUI real eliminaría ese parpadeo.

## Primera ejecución

1. Descomprime el ZIP.
2. Ejecuta `GOWA-cMc.exe`.
3. En **SISTEMA**, elige la carpeta que contiene `PS3_GAME`.
4. El launcher localiza el Update `Update\BCUS98232` y usa su `GOWA.SELF`.
5. Descifra con RPCS3 y genera `GOWA.elf`.
6. Al terminar, la consola se oculta.
7. Cuando HOME muestre `GOWA.elf listo`, pulsa **PLAY**.

Si ya tienes tu propio `GOWA.elf`, usa **USAR GOWA.ELF...**.

## Requisitos

- Windows 10/11 x64 y GPU con Direct3D 12.
- Copia legítima de **BCUS98232 1.12** con su Update.
- RPCS3 para el descifrado inicial, o tu `GOWA.elf` propio.
- El paquete no incluye el juego ni licencias.

## Contenido

- Launcher cMc de 11 páginas y textos en 6 idiomas.
- Detección de disco y Update; `vfs`/`hdd0` correctos.
- Precompilación de pixel shaders con DXC.
- Panel GC y F10, vibrance y contraste.
- Mandos Xbox y PlayStation, mods, guardados y ultrawide.
- Runtime VC, SDL3, FFmpeg y DXC incluidos.
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
