# Automatización BCUS98232 — actualizar el port cuando Ascension-R cambie

Port: **cMc-OmniPort** · Motor: **Ascension-R** (Ryan Bagas) + **ps3recomp** (Ned Heller / sp00nznet)

Este directorio convierte una release nueva del port original en una release de **esta** versión
(disco US `BCUS98232` v1.12) sin rehacer el trabajo a mano.

## Piezas

| Pieza | Qué es |
|---|---|
| `../MinGit/` | Git portable incluido en el paquete (no requiere instalación ni admin) |
| `_src/GOWA-cMc/` | Checkout git del upstream, con la rama **`bcus`** encima |
| `Update-BCUS.ps1` | Trae la release nueva, la convierte, compila y prueba |
| `Promote-BCUS.ps1` | Activa `build.next` (la build nueva) y guarda la anterior |
| `Rollback-BCUS.ps1` | Vuelve a `build.prev` |
| `state.json` | Base convertida, última ejecución, si el smoke pasó |
| `reports/update-*.log` | Informe de cada ejecución |

Los lanzadores de doble clic están en la raíz del proyecto: `Update-BCUS.exe`,
`Promote-BCUS.exe`, `Rollback-BCUS.exe`.

## Cómo está montado git

```
origin (rigaz29/Ascension-R)  ──►  bcus  ──►  build
                                     ▲
                     311c22a "Port to the US disc release (BCUS98232) 1.12"
                     base: f21d70c (snapshot del que salió esta copia)
```

`bcus` = upstream + **un commit** con la adaptación BCUS:

- `scripts/gowa.py`: `GOWA_TITLE_ID` (por defecto NPUA80918) y `doctor` sin exigir git.
- `port/game_find.cpp`, `port/launcher.cpp`: id de título, carpetas y textos BCUS98232.
- `port/main.cpp`: título de ventana con los créditos.
- `port/app.ico`: icono de esta versión.
- `port/game_files.inc`: hecho desde la copia BCUS (`game-files`).
- `CREDITOS-BCUS.md`: créditos, licencias y aviso legal.

El submódulo `ps3recomp` es un repo propio en el commit que fija cada upstream
(hoy `88ac02a4`, rama `gowa`).

## Qué hace `Update-BCUS.exe`

1. `git fetch` del upstream y de las etiquetas.
2. Destino: **última release etiquetada** (`v*`); si no hay etiquetas, `origin/main`.
   Si esa release no es más nueva que nuestra base, termina sin tocar nada.
3. `bcus-next` desde el destino + `cherry-pick` de los commits BCUS.
4. `ps3recomp` al commit que pide ese upstream.
5. **Puerta ELF**: el `GOWA.elf` de la copia debe seguir siendo el de 1.12
   (`4bf75c89…`). Si no, aborta: el lift del port no valdría.
6. Regenera `port/game_files.inc` desde `E:\GOWA-BCUS98232` y lo commitea si cambió.
7. `gowa.py doctor` debe decir `all good`.
8. `lift` + `build` en **`build.next`** (la build que juegas no se toca).
9. **Smoke test** de 30 s: exige `entry OPD 0x008283F8`, `backend init OK` y
   `libavcodec`, y que no haya líneas `ERROR/FATAL` nuevas.
10. Si todo pasa, adelanta `bcus` y deja `build.next` listo. Si algo falla, `bcus`
    no se mueve y la build actual sigue intacta (código de salida 2/3/4).

## Opciones

```
Update-BCUS.ps1                  # solo si hay release nueva
Update-BCUS.ps1 -Force           # aunque no haya release nueva
Update-BCUS.ps1 -Ref origin/main -Force
Update-BCUS.ps1 -NoBuild         # hasta doctor, sin compilar
Update-BCUS.ps1 -DryRun          # como -NoBuild y sin commitear game_files.inc
Promote-BCUS.ps1 [-Force]        # activa build.next (exige que el juego esté cerrado)
Rollback-BCUS.ps1 [-Yes] [-Purge]  # vuelve a build.prev; pide confirmación salvo -Yes
```

Los `.cmd` de la raíz llaman a estos scripts con comillas normales y aceptan argumentos
(`Promote-BCUS.exe -Force`, `Rollback-BCUS.exe -Yes`). Pausan al final para poder leer el
resultado; define `BCUS_NOPAUSE=1` para usarlos desde un script.

Accesos directos en el escritorio: **jugar**, **launcher** y **Promote build** (este último llama
a `Promote-BCUS.exe`, con el icono del port).

## Tarea programada

`BCUS98232 - update port` — todos los días a las 04:00 (Windows Task Scheduler).
Solo actúa si hay release nueva; deja el informe en `reports/` y `build.next` para promover.

## Límites

- Si upstream **reescribe las líneas** que toca el parche BCUS, el `cherry-pick` conflictúa:
  el script aborta sin cambiar nada. Hay que ajustar el commit BCUS a mano y volver a lanzar.
- El **smoke test prueba que arranca** (30 s), no que vaya igual de fino: por eso promover es manual.
- `FSR` y `SDL3` no están instalados; los avisos del build son normales.
- El binario contiene código del juego traducido: no se distribuye. Solo para tu copia.
