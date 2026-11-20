# GOWA_cMc_OmniPort v0.1.5

**God of War: Ascension [BCUS98232] — port nativo para Windows x64**
Motor **Ascension-R 0.2.0** · paquete **v0.1.5** · juego base **1.12**

Esta versión reúne el launcher cMc, la detección del disco y su Update, y un
paquete portable con el runtime completo para Windows.

## Mejoras y contenido de esta versión

### Launcher cMc

- Launcher de 11 páginas: HOME, GRÁFICOS, PANTALLA, CONTROLES, PARCHES,
  CHEATS, MODS, SISTEMA, AVANZADO, LOG y TROFEOS.
- HOME con estado real de la partida: juego, versión 1.12, gráfica, mando,
  parches, guardados y pixel shaders.
- SISTEMA con selección de carpeta, audio, guardados, idioma y descifrado.
- Pantalla de precarga moderna con música original.
- Panel GC de rendimiento y tecla **F10** para mostrarlo u ocultarlo.
- Vibrance y contraste de imagen.
- Soporte de mandos Xbox por XInput y PlayStation por SDL3.
- Mods por carpeta o `.zip`.
- Guardados con copias de seguridad.
- Ultrawide con HUD y menús a 16:9.

### Detección del juego

- Lee el `PARAM.SFO` del disco y del Update.
- Exige **BCUS98232 1.12**.
- Separa correctamente:
  - `vfs` → disco, carpeta que contiene `PS3_GAME`;
  - `hdd0` → Update.
- Si se elige el Update, busca el disco cercano.
- Si se elige la carpeta padre, resuelve `PS3_GAME`.
- Evita el cierre después de compilar shaders cuando faltaba `ARCHIVES`.

### Shaders y rendimiento

- Precompilación de pixel shaders con DXC.
- Resultado de la compilación completa:
  ```text
  49996 encontrados, 39597 compilados, 10399 ya presentes, 0 fallidos
  ```
- DXC incluido: `dxcompiler.dll` y `dxil.dll`.
- Runtime de Visual C++ incluido.
- SDL3 y FFmpeg incluidos.
- El paquete no necesita instalar esas dependencias por separado.

### Paquete portable

- Configuración sin rutas absolutas.
- Sin `vfs`, `gamedata`, logs ni caché de una PC ajena.
- README y LEEME generados para esta versión.
- El ejecutable de release está verificado por SHA-256.
- No incluye archivos del juego.

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
   - No elijas `Update\BCUS98232`.
4. Espera la compilación inicial de shaders.
5. Cuando HOME muestre `Pixel shaders: listos`, pulsa **PLAY**.

## Verificación

```text
Archivo: GOWA_cMc_OmniPort v0.1.5.zip
Tamaño:  126,4 MB
SHA-256: F2C7E514C8DB13888946C58FD39DD76FC536C9B1AB51004C2469188C9F2EA3E5

Ejecutable:
SHA-256: 0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01
```

## Limitaciones conocidas

- La primera ejecución compila shaders y puede tardar varios minutos.
- El ultrawide está implementado, pero falta validarlo en un monitor
  ultrawide real.
- El paquete no incluye el juego ni los archivos de la copia del usuario.
- El ejecutable contiene código traducido del juego y es una obra derivada;
  no está cubierto por la licencia MIT del proyecto.

## Créditos

- **Ascension-R** — Ryan Bagas y colaboradores.
- **ps3recomp** — Ned Heller / sp00nznet y colaboradores.
- **Port BCUS98232 y launcher cMc** — cMc-OmniPort.
- FFmpeg, SDL3, DXC, Cinzel y runtime de Microsoft.

Repositorio y versiones:
<https://github.com/cMc292/GOWA_cMc_OmniPort>

## Aviso legal

Este release no contiene archivos del juego: ni `PS3_GAME`, ni `*.psarc`, ni
`EBOOT`, ni vídeos, ni trofeos, ni claves o licencias de la consola.

El ejecutable contiene código traducido del juego, obra derivada de
*God of War: Ascension*: se ofrece para usarse con una copia legítima y la
licencia MIT del proyecto no la cubre.

*God of War* es una marca de Sony Interactive Entertainment LLC /
Santa Monica Studio. Proyecto de aficionados, sin ánimo de lucro, no afiliado
a Sony.

> **Nota:** este release todavía incluye `GOWA.elf` de trabajo. una versión futura
> lo retira y lo genera en la primera ejecución con RPCS3 y la licencia del juego.
