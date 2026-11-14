# Historial de versiones

Versiones publicadas del port a la **versión de disco US `BCUS98232` v1.12** de *God of War: Ascension*, por
**cMc-OmniPort (cMc292)**, sobre **Ascension-R** de Ryan Bagas y **ps3recomp** de Ned Heller (sp00nznet) y
colaboradores.

El paquete portable de cada versión está en la [página de releases](https://github.com/cMc292/GOWA_cMc_OmniPort/releases).

---

## v0.1.0-alpha.10 — 2026-11-13

**Base: Ascension-R v0.1.0-alpha.10** (`62a7122`), que incluye también los cambios de `v0.1.0-alpha.9`.

### Arreglado (motor)

- **Texturas rotas a los pocos minutos y caída del juego** (issues #9, #13 y, muy probablemente, la cinemática rota
  de #7). Cuando la memoria de vídeo se queda corta, el juego recorta los mips grandes de la textura usada hace más
  tiempo; también elegía texturas que solo tenían cargados los mips pequeños, cuyo bloque no tiene nada delante: el
  corte se pasaba del bloque, su tamaño quedaba negativo y el juego movía bloques por encima de otras texturas y del
  final de la memoria de vídeo. El port hace que ese corte no haga nada y el juego elige otra textura. Con la
  partida de #7, donde rompía una de cada tres o cuatro ejecuciones, siete ejecuciones seguidas con el arreglo
  terminaron sin rotura.
- **Texturas «mitad viejas, mitad nuevas»** (#9): el port guarda una copia de cada textura en la GPU y la vuelve a
  leer cuando el juego la cambia; si el juego escribía mientras se leía, la copia quedaba mezclada y su comprobación
  decía que estaba al día. Ahora la comprobación se hace antes y después de leer, en todos los mips, y esa textura
  se relee en su siguiente uso.

### Cambiado (motor)

- **F9 relee todas las texturas** de la memoria del juego: distingue una copia rota en el port (la arregla) de datos
  rotos en la memoria del juego (no la toca) y lo escribe en el registro con una línea `[tex]`.
- Arreglo del **parón de varios segundos tras los logos** cuando la caché de shaders aún no tenía ese shader: la luz
  de los personajes la hace la CPU hasta que el shader está compilado.
- En Linux, los shaders propios del port se compilan con **DXC**, como los del juego.
- El **build del código del juego es más rápido**: reparte según la memoria libre (5 min 32 s en el portátil de
  referencia, frente a 8 min 51 s de la alpha.8); el código generado es idéntico byte a byte.

### Port a BCUS (`bcus`)

- **Pantalla de precompilación** (lectura de archivos, compilación de shaders, errores) redibujada con el estilo del
  launcher nuevo (F7): resolución de ventana, arte opcional, fuente Cinzel y textos en español.
- **Navegación**: LOG y TROFEOS ya se abren con el ratón, y volver a pulsar la pestaña activa pliega el panel a HOME.

## v0.1.0-alpha.8 — 2026-11-13

Primera release pública del port. Base: Ascension-R v0.1.0-alpha.8 (`e2d5d31`).

- Adaptación a `BCUS98232` 1.12: verificación de que el `GOWA.elf` es el mismo que el de `NPUA80918` 1.12, vista
  fusionada disco + parches 1.04–1.12, descifrado del SELF de parche con el KLIC del parche 1.12, `game_files.inc`
  regenerado e identidad `BCUS98232` en la búsqueda del juego y el launcher.
- **Launcher nuevo** (estilo SpartanPort, en español) con 10 páginas: HOME, GRÁFICOS, PANTALLA, CONTROLES, PARCHES,
  CHEATS, SISTEMA, AVANZADO, LOG y TROFEOS; visor de log y galería de trofeos con el `TROPHY.TRP` en el idioma del
  juego y el progreso del `TROPUSR.DAT`.
- **Paquete portable**: lanzadores nativos, FFmpeg, MinGit y Python embebidos, y modo de actualización
  (`Update-BCUS` / `Promote-BCUS` / `Rollback-BCUS`).
