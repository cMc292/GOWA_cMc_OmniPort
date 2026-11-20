# GOWA_cMc_OmniPort v0.1.6

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.6** · juego base **1.12**

## Publicación coherente

Esta `v0.1.6` sustituye al paquete antiguo con el mismo número. Es la
publicación siguiente a `v0.1.5` y cumple las dos reglas nuevas:

1. incluye **`portable.txt`**;
2. incluye una carpeta **`mods` funcional**.

## Contenido

- Launcher traducido a **6 idiomas**.
- Cambio de idioma del launcher al pulsar **GUARDAR**, sin reiniciar.
- Idioma del juego limitado a los idiomas reales del disco:
  English, French, Portuguese (Brazil) y Spanish (Latin America).
- Ejecutable exacto del full, sin parche PE.
- Todas las DLL de FFmpeg del build.
- **`portable.txt`**: configuración, logs y cachés junto al ejecutable.
- **`mods` funcional**, con `skip-intro` ya convertido:
  ```text
  mods\skip-intro\mod.ini
  mods\skip-intro\archive\INTRO\BOOT_BCES01741.TXT
  mods\skip-intro\archive\INTRO\BOOT_BCUS98232.TXT
  ```
- **No incluye `GOWA.elf`**: se genera en la primera ejecución.
- **No incluye el caché de shaders**: se genera en la primera ejecución.

## Primera ejecución

1. Extrae el ZIP en una **ruta corta** (`C:\GOWA`, el Escritorio).
   No lo ejecutes desde `work\dist`: la ruta profunda puede bloquear la carga.
2. Ejecuta `GOWA-cMc.exe`.
3. Elige la carpeta que contiene `PS3_GAME`.
4. El launcher busca el Update y genera `GOWA.elf` con RPCS3.
5. Aparecerá **COMPILANDO SHADERS**:
   ```text
   49.996 shaders
   Puede tardar varios minutos
   ```
6. Cuando termine, el juego arranca.
7. En los siguientes arranques ya usa el caché generado.

La primera compilación es normal y no es un cuelgue.

## Mods

El paquete incluye `kit\python` y `kit\scripts\modpack.py` para empaquetar los mods en el primer arranque. Probado en el propio paquete:

```text
archive  skip-intro  2  0  0  -
TPPS6.PSARC: 2 entries
```
`n## Mods

`mods` está activo por defecto. `skip-intro` salta las pantallas de aviso y
logos; el juego pasa directamente a la pantalla de carga. Se puede desactivar
en **MODS**, y se pueden añadir más mods en formato `mod.ini` + `archive\` o
`USRDIR\`.

## Requisitos

- Windows 10/11 x64 y GPU con Direct3D 12.
- Copia legítima de **BCUS98232 1.12** con su Update.
- RPCS3 para el descifrado inicial, o tu `GOWA.elf` propio.
- El paquete no incluye el juego ni licencias.

## Verificación

```text
Archivo: GOWA_cMc_OmniPort v0.1.6.zip
portable.txt: SÍ
mods\skip-intro: SÍ
GOWA.elf: NO
Caché de shaders: NO
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
