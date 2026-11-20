# Reglas de publicación

Estas reglas son obligatorias para **todos los Releases nuevos** de
`GOWA_cMc_OmniPort`.

## 1. Estructura fija del README

El README del paquete conserva siempre esta organización y este orden:

1. Portada y título.
2. Badges de versión, plataforma, estado y licencia.
3. Descripción del port.
4. Enlace al repositorio.
5. Aviso importante: no contiene archivos del juego.
6. `## Contenido`
7. `## Estado`
8. `## Características`
9. `## Descarga e instalación`
10. `## Requisitos`
11. `## Primera ejecución`
12. `## Mods`
13. `## Controles`
14. `## Solución de problemas`
15. `## Preguntas frecuentes`
16. `## Legal`
17. `## Créditos`
18. `## Licencia`
19. `## Información de la build`

No se renombran, eliminan ni reordenan estas secciones en un Release nuevo.

## 2. Solo agregar funciones y mejoras

En cada publicación nueva:

- se actualiza `## Estado` si cambia el estado real;
- se **agregan** las funciones nuevas en `## Características`;
- se **agregan** las mejoras en la sección que corresponda;
- se **agregan** nuevos problemas y soluciones en `## Solución de problemas`;
- se **agregan** preguntas nuevas al final de `## Preguntas frecuentes`;
- se actualiza `## Información de la build` con versión, hash y fecha.

No se reescribe el README desde cero ni se cambia su estructura para una
publicación normal.

## 3. Reglas obligatorias del paquete

Cada Release debe incluir:

```text
portable.txt
mods\ (funcional)
AVISO-LEGAL.txt
```

Y no debe incluir:

```text
GOWA.elf
caché de shaders
archivos del juego
```

## 4. Aviso legal obligatorio

El texto exacto está en `scripts/AVISO-LEGAL.txt` y debe aparecer también en:

```text
README.md
LEEME-PRIMERO.txt
AVISO-LEGAL.txt
```

El empaquetador falla si falta o se altera.

## 5. Orden de publicación

Los números de versión siguen el orden publicado. No se crean versiones
intermedias fuera de ese orden ni se reutiliza un número para un paquete
diferente sin reemplazar el anterior de forma explícita.

## 6. Rutas públicas

La documentación del ZIP no menciona rutas internas del proyecto, como
`work\dist`. Solo indica:

> Extrae el ZIP en una ruta corta, por ejemplo `C:\GOWA` o el Escritorio.

## 7. Primera ejecución

La primera ejecución debe documentar:

- la elección de la carpeta que contiene `PS3_GAME`;
- la generación de `GOWA.elf`;
- la compilación inicial de shaders;
- que la compilación inicial no es un cuelgue.

## 8. Palabra clave de publicación

Cuando el usuario escriba exactamente:

```text
Audita y prepara Para publicar
```

El agente debe:

1. auditar el ZIP, el ejecutable, el aviso legal, `portable.txt`, `mods`, la ausencia de `GOWA.elf` y del caché;
2. verificar los hashes y el checksum;
3. comprobar que la documentación no tenga rutas internas;
4. terminar la respuesta con un bloque de **copia y pega** para la sección `Publicar releases`.

El bloque debe incluir versión, nombre del ZIP, SHA-256, SHA-256 del exe, tamaño, requisitos de publicación y un enlace o referencia a las notas. No debe incluir instrucciones internas ni rutas como `work\dist`.
