<div align="center">

# GOWA_cMc_OmniPort

**God of War: Ascension · BCUS98232 · Windows x64**

![Versión](https://img.shields.io/badge/version-{{VERSION}}-orange)
![Plataforma](https://img.shields.io/badge/platform-Windows%20x64-lightgrey)
![Estado](https://img.shields.io/badge/status-alpha-orange)
![Licencia](https://img.shields.io/badge/license-MIT-blue)

</div>

Port nativo de **God of War: Ascension** (PS3, BCUS98232) para Windows x64.
El código del juego se ejecuta traducido a x86-64 y el RSX se dibuja con
Direct3D 12: no hay emulador durante la partida. RPCS3 se usa solo para
descifrar tu copia.

Repositorio y versiones:
<https://github.com/cMc292/GOWA_cMc_OmniPort>

> [!IMPORTANT]
> Este paquete **no contiene archivos del juego**. Necesitas tu propia copia
> legítima de **BCUS98232 1.12** con su Update. No se incluye `GOWA.elf`:
> se genera en la primera ejecución. Consulta [Legal](#legal).

## Contenido

- [Estado](#estado)
- [Características](#características)
- [Descarga e instalación](#descarga-e-instalación)
- [Requisitos](#requisitos)
- [Primera ejecución](#primera-ejecución)
- [Mods](#mods)
- [Controles](#controles)
- [Solución de problemas](#solución-de-problemas)
- [Preguntas frecuentes](#preguntas-frecuentes)
- [Legal](#legal)
- [Créditos](#créditos)
- [Licencia](#licencia)
- [Información de la build](#información-de-la-build)

## Estado

- **Juego:** God of War: Ascension **BCUS98232 1.12**, con su Update instalado
  en RPCS3.
- **Plataforma:** Windows 10 u 11, 64 bits.
- **GPU:** compatible con Direct3D 12.
- **Online:** no soportado; el port es para un jugador.
- **Primera carga:** compila los shaders del juego; puede tardar varios minutos.
- **Paquete:** portable, sin instalador. La configuración, los logs y el caché
  quedan junto al ejecutable gracias a `portable.txt`.

## Características

- **Launcher cMc:** 11 páginas para gráficos, pantalla, controles, parches,
  cheats, mods, sistema, avanzado, log y trofeos.
- **Idiomas:** launcher en inglés, español, portugués de Brasil, francés,
  alemán e italiano; cambio en caliente al pulsar **GUARDAR**.
- **Juego:** idioma limitado a los idiomas reales del disco: English, French,
  Portuguese (Brazil) y Spanish (Latin America).
- **Imagen:** FSR, SMAA, filtrado anisotrópico, vibrance y contraste.
- **Pantalla:** ventana, borderless, ultrawide, VSync, límite de FPS y panel GC.
- **Shaders:** compilación con DXC y caché persistente entre ejecuciones.
- **Parches:** MLAA, profundidad de campo, motion blur, sombras y mesh trimming.
- **Cheats:** daño, orbes, magia, objetos, furia y dobles saltos.
- **Controles:** Xbox/XInput y PlayStation mediante SDL3; teclado y ratón.
- **Mods:** carpeta `mods` funcional; mods por carpeta o `.zip`.
- **Guardados:** copias de seguridad antes de sobrescribir.
- **Informes:** `GOWA-cMc.exe --report` genera un ZIP con los logs y tus rutas
  ocultas.

## Descarga e instalación

1. Descarga el ZIP desde la página de releases.
2. Extrae el ZIP en una **ruta corta**, por ejemplo `C:\GOWA` o el Escritorio.
   Evita rutas profundas: Windows y el juego tienen un límite práctico de
   260 caracteres que puede impedir abrir WAD o vídeos.
3. No hace falta instalar Visual C++, FFmpeg ni DXC: van incluidos.

`portable.txt` está incluido. No borres `portable.txt` si quieres que la
configuración, los logs y el caché sigan junto al ejecutable.

### RecompLauncher

El Release incluye `recomplauncher.json` con el nombre del proyecto, el
repositorio, el ejecutable y las rutas de configuración y guardados. La ficha
pública de RecompLauncher todavía no tiene URL; cuando se publique, esta
sección tendrá el badge de instalación oficial.

## Requisitos

- Windows 10 u 11, 64 bits.
- CPU con AVX2 y FMA.
- GPU compatible con Direct3D 12.
- Tu copia legítima de **BCUS98232 1.12** con el Update `1.04–1.12`.
- RPCS3 para descifrar el juego la primera vez, o tu propio `GOWA.elf`.

El paquete no incluye el juego, licencias, `PS3_GAME`, `*.psarc`, `EBOOT`,
vídeos ni trofeos.

## Primera ejecución

1. Ejecuta `GOWA-cMc.exe`.
2. En **SISTEMA**, elige la carpeta que contiene **`PS3_GAME`**.
   No elijas `Update\BCUS98232`: no tiene `ARCHIVES` y el juego se cerraría
   después de la pantalla de carga.
3. El launcher busca el Update y descifra `GOWA.SELF` con RPCS3.
4. Aparecerá **COMPILANDO SHADERS**. Deja que termine; es normal que tarde
   varios minutos la primera vez.
5. Cuando HOME muestre `GOWA.elf listo` y los shaders estén listos, pulsa
   **PLAY**.
6. En los siguientes arranques se usa el `GOWA.elf` y el caché ya generados.

Si ya tienes un `GOWA.elf`, usa **USAR GOWA.ELF...** en SISTEMA.

## Mods

El paquete incluye una carpeta `mods` funcional, con `skip-intro` ya convertido
al formato del port:

```text
mods\skip-intro\mod.ini
mods\skip-intro\archive\INTRO\BOOT_BCES01741.TXT
mods\skip-intro\archive\INTRO\BOOT_BCUS98232.TXT
```

- **MODS** en el launcher lista los mods y permite activarlos, desactivarlos y
  ordenarlos.
- Un mod puede ser una carpeta o un `.zip` dentro de `mods`.
- Formato: `mod.ini` y archivos en `archive\` o `USRDIR\`.
- Los mods son solo archivos: no se ejecuta código de un mod.
- El paquete incluye `kit\python` y `kit\scripts\modpack.py` para empaquetar los
  mods en el primer arranque.

`skip-intro` salta los avisos y logos iniciales. Se puede desactivar en MODS.

## Controles

| Tecla | Acción |
|---|---|
| `F10` | Mostrar u ocultar el panel GC |
| `F11` | Ventana / borderless |
| `F12` | Captura de pantalla |
| `F9` | Releer texturas |
| `ESC` | Cerrar el juego, si está activado en SISTEMA |

El launcher se controla con ratón y teclado; los mandos Xbox y PlayStation se
configuran en **CONTROLES**.

## Solución de problemas

### Se queda en COMPILANDO SHADERS

Es la primera compilación. No cierres el juego; cuando termine se guarda el
caché y los siguientes arranques son rápidos. Si se cierra, vuelve a abrirlo:
continúa desde donde iba.

### El juego se cierra después de la pantalla de carga

Estás apuntando al Update. Selecciona la carpeta del **disco**, la que contiene
`PS3_GAME`.

### No reconoce el mando PlayStation

Comprueba que `SDL3.dll` está junto a `GOWA-cMc.exe`. El paquete lo incluye.
Xbox usa XInput; PS4/PS5 usa SDL3.

### No compilan los pixel shaders

Comprueba que están junto al ejecutable:

```text
dxcompiler.dll
dxil.dll
```

y el runtime de Visual C++:

```text
MSVCP140.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
CONCRT140.dll
```

### Error al abrir WAD o vídeos

Extrae el paquete en una ruta más corta. No uses una carpeta con una ruta muy
profunda.

### El launcher no encuentra el juego

En SISTEMA elige manualmente la carpeta que contiene `PS3_GAME`. No elijas
`Update\BCUS98232`.

## Preguntas frecuentes

### ¿Es una versión oficial de PC?

No. *God of War: Ascension* solo salió en PlayStation 3. Este es un proyecto
de aficionados, sin ánimo de lucro y no afiliado a Sony.

### ¿Necesito el juego original?

Sí. El paquete no contiene código ni archivos del juego. Necesitas una copia
legítima de BCUS98232 1.12 y su Update.

### ¿Por qué el paquete no incluye `GOWA.elf`?

Porque es código derivado del juego. El launcher lo genera desde tu `GOWA.SELF`
con RPCS3 y tu copia legal.

### ¿RPCS3 es necesario para jugar?

No. Solo se usa para descifrar la primera vez. Después, el juego se ejecuta
nativo con Direct3D 12.

### ¿Puedo usar mods?

Sí. La carpeta `mods` está incluida y es funcional. Añade carpetas o `.zip` y
actívalos en MODS.

### ¿Por qué tarda la primera vez?

Porque compila los shaders del juego. Es un proceso único por versión; después
se usa el caché generado.

## Legal

Aviso legal

Este release no contiene archivos del juego: ni PS3_GAME, ni *.psarc, ni EBOOT, ni vídeos, ni trofeos, ni claves o licencias de la consola.

El ejecutable contiene código traducido del juego, obra derivada de God of War: Ascension: se ofrece para usarse con una copia legítima y la licencia MIT del proyecto no la cubre.

God of War es una marca de Sony Interactive Entertainment LLC / Santa Monica Studio. Proyecto de aficionados, sin ánimo de lucro, no afiliado a Sony.

## Créditos

- **Ascension-R** y **ps3recomp**: motor, toolkit y runtime.
- **FFmpeg** (LGPL), **SDL3**, **DXC**, **Cinzel** (OFL) y runtime de Microsoft.
- **Port BCUS98232**: cMc-OmniPort.

Lee `CREDITOS.md`, `LICENSE`, `THIRD-PARTY.txt` y `THIRD_PARTY_NOTICES.md`
antes de redistribuir.

## Licencia

El código del proyecto está bajo la licencia **MIT**; consulta `LICENSE`.
La licencia MIT **no cubre** el juego ni el código derivado de él.

## Información de la build

```text
Paquete:      {{VERSION}}
Ejecutable:   {{EXE_SHA256}}
Compilado:    {{DATE}}
Repositorio:  https://github.com/cMc292/GOWA_cMc_OmniPort
```
