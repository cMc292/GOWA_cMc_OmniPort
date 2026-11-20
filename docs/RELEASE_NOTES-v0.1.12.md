# GOWA_cMc_OmniPort v0.1.12

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.12** · juego base **1.12**

## Contenido de esta versión

- Launcher traducido a **6 idiomas**: inglés, español, portugués de Brasil,
  francés, alemán e italiano.
- El idioma del launcher se cambia en **SISTEMA** y se aplica al pulsar
  **GUARDAR**, sin reiniciar.
- Idioma del juego limitado a los idiomas reales del disco:
  English, French, Portuguese (Brazil) y Spanish (Latin America).
- El paquete usa **exactamente** el ejecutable del full.
- Incluye **todas** las DLL de FFmpeg del build.
- **No incluye `GOWA.elf`**: se genera en la primera ejecución.
- **No incluye el caché de shaders**: se genera en la primera ejecución.
- La consola se oculta si `GOWA.elf` ya existe y al terminar de generarlo.
  En la primera ejecución puede verse mientras se prepara el juego.

## Primera ejecución

1. Descomprime el ZIP.
2. Ejecuta `GOWA-cMc.exe`.
3. Elige la carpeta que contiene `PS3_GAME`.
4. El launcher busca el Update y genera `GOWA.elf` con RPCS3.
5. Aparecerá **COMPILANDO SHADERS**:
   ```text
   49.996 shaders
   Puede tardar varios minutos
   ```
6. Cuando termine, el juego arranca solo.
7. En los siguientes arranques ya usa el caché generado.

La primera compilación es normal y no es un cuelgue.

## Requisitos

- Windows 10/11 x64 y GPU con Direct3D 12.
- Copia legítima de **BCUS98232 1.12** con su Update.
- RPCS3 para el descifrado inicial, o tu `GOWA.elf` propio.
- El paquete no incluye el juego ni licencias.

## Contenido

- Launcher cMc de 11 páginas.
- Detección de disco y Update; `vfs`/`hdd0` correctos.
- Pixel shaders con DXC.
- Panel GC y F10, vibrance y contraste.
- Mandos Xbox/PlayStation, mods, guardados y ultrawide.
- Runtime VC, SDL3, FFmpeg completo y DXC.
- Configuración limpia, sin rutas absolutas.

## Verificación del paquete

```text
Archivo: GOWA_cMc_OmniPort v0.1.12.zip
GOWA.elf incluido: NO
Caché de shaders incluido: NO
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
