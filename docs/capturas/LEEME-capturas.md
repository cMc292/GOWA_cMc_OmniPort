# Capturas del repositorio

Esta carpeta contiene:

| Archivo | Uso |
|---|---|
| `launcher-home.png` | Captura del launcher (página **HOME**) que sale en el README. Es una captura del programa; el fondo lleva el arte de portada del juego, puesto por el dueño del repo en su copia local (`assets/launcher-bg.jpg`). |
| `banner.png` | Banner de diseño propio (fondo oscuro, acento carmesí), sin arte del juego. Disponible por si se quiere volver a un banner sin capturas. |
| `social-preview.png` | Previsualización social del repositorio (1280×640), de diseño propio. Se sube por web: *Settings → General → Social preview*. |

> El arte de portada que aparece en `launcher-home.png` es de **Santa Monica Studio / Sony** y no lo cubre la licencia
> MIT del proyecto. Se incluye como captura del programa; si en algún momento hay un problema de derechos, se
> sustituye por una captura con el **fondo procedural** (sin arte del juego) y no afecta al funcionamiento del repo.

## Cómo hacer más capturas

Con el **fondo procedural** (sin la portada) para que sean solo tuyas:

1. En la carpeta del paquete, quita (o renombra) `assets\launcher-bg.jpg`, `assets\launcher-bg.png` y
   `assets\launcher-logo.png` si existen.
2. Abre `Launcher GOWA-cMc.exe` y ve a las páginas que quieras: **HOME**, **GRÁFICOS**, **TROFEOS**, **LOG**.
3. Captura con `Win+Mayús+S` o `Alt+ImprPant` y guarda los `.png` aquí, por ejemplo
   `launcher-graficos.png`, `launcher-trofeos.png`.
4. Añádelas al README con:

```markdown
<p align="center">
  <img src="docs/capturas/launcher-graficos.png" alt="Launcher: GRÁFICOS" width="49%">
  <img src="docs/capturas/launcher-trofeos.png" alt="Launcher: TROFEOS" width="49%">
</p>
```
