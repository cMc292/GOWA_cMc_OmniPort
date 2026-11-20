# GOWA_cMc_OmniPort v0.1.6

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.6** · juego base **1.12**

## Cambio principal

- **Se eliminó la ventana CMD al abrir el launcher y el juego.**
- El ejecutable usa ahora el subsistema de Windows (GUI), no el de consola.
- El comportamiento del juego y del launcher es el mismo de `v0.1.5`.

No se recompiló el gameplay: se cambió el subsistema del ejecutable
conocido y verificado. El full y los datos del juego no se tocaron.

## Contenido

Incluye las mismas mejoras de `v0.1.5`:

- launcher cMc de 11 páginas;
- HOME con estado real de la partida;
- SISTEMA con selección de disco/Update;
- detección de `BCUS98232 1.12`;
- `vfs` al disco y `hdd0` al Update;
- precompilación de pixel shaders con DXC;
- panel GC y F10;
- vibrance y contraste;
- mandos Xbox y PlayStation;
- mods y guardados;
- ultrawide;
- runtime VC, SDL3, FFmpeg y DXC incluidos;
- configuración limpia, sin rutas absolutas ni archivos del juego;`n- carpeta `mods` incluida con instrucciones para instalar mods.

## Requisitos

- Windows 10/11 x64.
- GPU compatible con Direct3D 12.
- Tu copia legítima de **God of War: Ascension BCUS98232 1.12**.
- Tu `GOWA.elf` descifrado, si tu copia lo necesita.
- El paquete no incluye el juego.

## Instalación

1. Descomprime el ZIP en una carpeta con permisos de escritura.
2. Ejecuta `GOWA-cMc.exe`.
3. En **SISTEMA**, elige la carpeta que contiene `PS3_GAME`.
   No elijas `Update\BCUS98232`.
4. Espera la compilación inicial de shaders.
5. Cuando HOME muestre `Pixel shaders: listos`, pulsa **PLAY**.

## Verificación

```text
Archivo: GOWA_cMc_OmniPort v0.1.6.zip
Tamaño:  126,4 MB
SHA-256: 246070F268032FF04757F00F9B6615F3C40E16C6799F7CB8CEBAAD91369887C6

Ejecutable:
SHA-256: 009E86ECF9CE1C5DE3BC1F761D9202F412A1CC5529308F4DB8903F766F04CB4A
Subsistema PE: 2 (Windows GUI)
```

## Validación

- Sesión larga confirmada por el usuario: partida larga sin problemas.

## Limitaciones conocidas

- La primera ejecución compila shaders y puede tardar varios minutos.
- El ultrawide está implementado, pero falta validarlo en un monitor
  ultrawide real.
- El paquete no incluye el juego ni los archivos de la copia del usuario.
- El ejecutable contiene código traducido del juego y es una obra derivada.

## Créditos

- **Ascension-R** — Ryan Bagas y colaboradores.
- **ps3recomp** — Ned Heller / sp00nznet y colaboradores.
- **Port BCUS98232 y launcher cMc** — cMc-OmniPort.
- FFmpeg, SDL3, DXC, Cinzel y runtime de Microsoft.

Repositorio y versiones:
<https://github.com/cMc292/GOWA_cMc_OmniPort>
