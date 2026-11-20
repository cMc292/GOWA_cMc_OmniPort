# i18n del launcher — plan de trabajo

**Rama:** `i18n-launcher`
**Alcance:** solo la interfaz del launcher y sus mensajes. No incluye textos, voces ni assets del juego.
**Idiomas iniciales:** inglés, español, portugués de Brasil, francés, alemán e italiano.

## Estado actual

- `system.language` sigue controlando el idioma del juego.
- `launcher.language` es independiente y controla esta ventana.
- Los archivos de idioma están en `locale/<código>.lang`.
- El fallback es inglés.
- Ya están migrados:
  - títulos de las 11 páginas;
  - subtítulos de las 11 páginas;
  - etiquetas de navegación.
- El resto de textos del launcher sigue pendiente.

## Formato

Archivo UTF-8, una clave por línea:

```text
nav.home=HOME
page.graphics.sub=Ajustes de imagen...
```

Reglas:

- no se traducen nombres de archivos, claves técnicas ni logs;
- las claves no cambian al traducir;
- si falta una clave, se usa inglés;
- no se usan cadenas concatenadas para formar frases;
- las traducciones finales requieren revisión nativa.

## Fases

### Fase 1 — infraestructura (hecha)

- módulo `port/i18n.cpp`;
- selector `launcher.language`;
- archivos de los 6 idiomas;
- navegación, títulos y subtítulos;
- prueba visual en francés.

### Fase 2 — catálogo completo (en curso)`n`n- HOME completo en los 6 idiomas;`n- SISTEMA funcional y valores en los 6 idiomas;`n- GRÁFICOS completo en los 6 idiomas.`n- botones globales y errores dinámicos pendientes.`n- SISTEMA es el siguiente bloque.

- extraer todas las cadenas de `port/launcher.cpp` y `port/ui_new.inc`;
- separar etiquetas, descripciones, avisos y errores;
- asignar claves estables;
- migrar las 11 páginas;
- migrar botones, toasts y diálogos.

### Fase 3 — traducción de los 6 idiomas

- inglés: base y revisión;
- español: revisión latinoamericana/España;
- portugués de Brasil;
- francés;
- alemán;
- italiano.

### Fase 4 — QA

- capturas de las 11 páginas en cada idioma;
- pruebas de textos largos en alemán y francés;
- validación de claves faltantes;
- revisión de acentos, géneros y mayúsculas;
- prueba con `launcher.language=auto` en Windows español, inglés y portugués.

### Fase 5 — más idiomas

Después de los 6 iniciales:

- ruso;
- polaco;
- chino simplificado;
- japonés;
- coreano;
- idiomas de comunidad.

## Prueba rápida

```powershell
$env:GOWA_LAUNCHER_LANGUAGE='fr'
build\GOWA-cMc.exe
```

Captura sin abrir ventana:

```powershell
$env:GOWA_LAUNCHER_SHOT='C:\temp\i18n'
$env:GOWA_LAUNCHER_SHOT_DPI='120'
$env:GOWA_LAUNCHER_LANGUAGE='fr'
build\GOWA-cMc.exe
```

## Empaquetado

No se incluye en el release `v0.1.6` todavía. Cuando el catálogo esté completo:

- copiar `locale/` al paquete;
- verificar que no haya rutas absolutas;
- generar una versión nueva;
- probar la selección de idioma en Limpio;
- actualizar README y notas.

## Criterio de terminado

No se considera terminado hasta que:

1. todas las cadenas visibles estén en archivos de idioma;
2. no queden textos hardcodeados en la UI;
3. las 11 páginas se vean bien en los 6 idiomas;
4. no haya claves faltantes;
5. un hablante nativo haya revisado cada idioma.
