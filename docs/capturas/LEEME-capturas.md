# Capturas del repositorio

Esta carpeta tiene el banner del README (`banner.png`) y la imagen de previsualización social
(`social-preview.png`). **Ambas son de diseño propio** (fondo oscuro, acento carmesí, chips dorados): no contienen
arte del juego.

## Si quieres añadir capturas del launcher

Hazlas **con el fondo procedural, sin el arte de portada**. El launcher dibuja un fondo procedural cuando no existe
`assets\launcher-bg.jpg` junto al paquete; con ese fondo, la captura es solo tuya y se puede publicar sin problema.

1. En la carpeta del paquete, quita (o renombra) `assets\launcher-bg.jpg`, `assets\launcher-bg.png` y
   `assets\launcher-logo.png` si existen.
2. Abre `Launcher GOWA-cMc.exe` y ve a las páginas que quieras: **HOME**, **GRÁFICOS**, **TROFEOS**, **LOG**.
3. Captura con `Win+Mayús+S` o `Alt+ImprPant` y guarda los `.png` aquí, por ejemplo
   `launcher-home.png`, `launcher-graficos.png`, `launcher-trofeos.png`.
4. Añádelas al README con:

```markdown
<p align="center">
  <img src="docs/capturas/launcher-home.png" alt="Launcher: HOME" width="49%">
  <img src="docs/capturas/launcher-graficos.png" alt="Launcher: GRÁFICOS" width="49%">
</p>
```

> No subas capturas con la portada del juego, el logo de *God of War*, los iconos de trofeo del `TROPHY.TRP` ni
> fotogramas del juego: son material de Sony / Santa Monica Studio y este repositorio es solo documentación y
> herramientas.
