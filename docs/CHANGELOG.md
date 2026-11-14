# Historial de versiones

Versiones publicadas del port a la **versión de disco US `BCUS98232` v1.12** de *God of War: Ascension*, por
**cMc-OmniPort (cMc292)**, sobre **Ascension-R** de Ryan Bagas y **ps3recomp** de Ned Heller (sp00nznet) y
colaboradores.

El paquete portable de cada versión está en la [página de releases](https://github.com/cMc292/GOWA_cMc_OmniPort/releases).

---

## v0.2.0 — 2026-11-14

**Primera versión de la línea propia del port.** Base: **Ascension-R v0.1.0-alpha.10** @ `cc42407`;
runtime `ps3recomp` rama `gowa` @ `09fece8`. Formato: **paquete de ingeniería** (parche único + código
acumulado); el binario se compila en el PC del usuario. Changelog detallado:
`source_integrated\docs\VERSIONES.md` del paquete.

### Launcher bilingüe (español / inglés)

- Las diez páginas, los diálogos y los avisos, con **44 opciones** en los dos idiomas.
- `[launcher] language = auto | es | en` y **chip** de idioma en HOME (conmuta y persiste).
- Independiente de `[system] language`, que es el idioma del juego.

### Nuevas funciones

- **RESTAURAR**: deshacer la última partida guardada, con instantánea reversible.
- **Motor PSARC** propio (reescritura con `deflate` de **zlib 1.3.1**), verificado sobre el `GAME.PSARC` real.
- **Mods**: recetas `mod.json`, overlay en el runtime y botón **REGENERAR**; receta `skip-intro` incluida.
- **F12** guarda la captura (el nombre llevaba caracteres ilegales).
- **`PAD_NO_HOST`** (entrada solo por guion) y arreglo del **escritor de perfiles PGO**.

### Identidad y correcciones

- **Versión propia 0.2.0** en el launcher, la pantalla de carga, el `.exe` y el informe de fallo; la base se
  muestra en **AVANZADO** junto a las licencias.
- **Dos lanzadores**: `GOWA-cMc.exe` (juego directo) y `GOWA-cMc-Launcher.exe` (launcher).
- Título de ventana sin créditos; aviso si no hay **Direct3D 12**.
- Corregidos: versión del pie de HOME, READY CHECK, «ABRIR LOGS», final de página tapado, fila de la carpeta
  del juego en SISTEMA, 5 escapes hexadecimales que se comían letras («Gráfica»), diálogos de preparar en
  español y restos de español en las páginas en inglés.
- Base actualizada a **`cc42407`** (SISTEMA recupera la fila de la carpeta del juego).

### Notas

- El paquete **no incluye el binario**; se compila con `python scripts\gowa.py build` desde la base
  `cc42407` y el parche único.
- Incluye **arte de la HOME** y **capturas del juego** por decisión del autor: material del juego no cubierto
  por la MIT (ver `THIRD_PARTY_NOTICES.md`).
- Los parches tocan también el submódulo `ps3recomp` (rama `gowa`).

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
