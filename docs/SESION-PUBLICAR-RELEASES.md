# Sesión especial: publicar releases de GOWA_cMc_OmniPort

**Espacio de trabajo:** `C:\deepseek harness\Port de God of War Ascension BCUS98232`
**Repositorio:** <https://github.com/cMc292/GOWA_cMc_OmniPort>
**Rama:** `master`
**Release actual:** `v0.1.6`
**Paquete:** `work\dist\GOWA_cMc_OmniPort v0.1.6.zip`

Esta sesión es únicamente para publicar releases. No se usa para programar,
recompilar ni modificar el full.

---

## 1. Prompt para la nueva sesión

Copiar todo esto al crear la sesión:

```text
Eres la sesión de publicación de releases de GOWA_cMc_OmniPort.

Espacio de trabajo:
C:\deepseek harness\Port de God of War Ascension BCUS98232

Repositorio:
https://github.com/cMc292/GOWA_cMc_OmniPort

Reglas:
- No recompilar el full.
- No borrar ni mover datos del juego.
- No crear carpetas de trabajo fuera del proyecto.
- No publicar nada sin verificar el ZIP.
- No guardar tokens ni credenciales.
- Antes de publicar, leer docs/SESION-PUBLICAR-RELEASES.md.

Tarea inicial: comprobar el ZIP v0.1.6, verificar el hash del ejecutable,
comprobar git/gh y preparar el release. No ejecutar el release hasta
confirmación del usuario.
```

---

## 2. Estado del paquete

```text
work\dist\GOWA_cMc_OmniPort v0.1.6.zip
```

```text
Tamaño: 126,4 MB
Entradas: 26
Ejecutable: 0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01
```

El ZIP ya contiene el ejecutable bueno. El ejecutable del `build` actual
(`5FA6…`) no se publica.

---

## 3. Requisitos

- GitHub CLI instalado y autenticado:

```powershell
gh auth login
gh auth status
```

- Repositorio configurado:

```powershell
git -C "work\0.2.0-full" remote -v
```

Debe mostrar:

```text
origin https://github.com/cMc292/GOWA_cMc_OmniPort.git
```

- Árbol limpio:

```powershell
git -C "work\0.2.0-full" status --short
```

---

## 4. Verificación antes de publicar

### 4.1 ZIP y ejecutable

```powershell
Get-FileHash `
  "work\dist\GOWA_cMc_OmniPort v0.1.6\GOWA-cMc.exe" `
  -Algorithm SHA256
```

Debe ser:

```text
0703FD9F28D6F697F20EB395A816C0FA5607003A98D1D9311932039AB1BB5B01
```

Si la carpeta no existe, extraer el ZIP en un directorio temporal o
regenerarla con el empaquetador. No usar el exe `5FA6…`.

### 4.2 Contenido

Debe incluir:

```text
GOWA-cMc.exe
GOWA.elf
SDL3.dll
FFmpeg
dxcompiler.dll
dxil.dll
runtime VC
README.md
LEEME-PRIMERO.txt
LICENSE
CREDITOS.md
THIRD-PARTY.txt
THIRD_PARTY_NOTICES.md
```

Sin:

```text
vfs con rutas absolutas
gamedata con rutas absolutas
logs
d3d12_shader_cache.bin*
```

### 4.3 Prueba final

Si aún no se ha hecho:

1. Extraer en una carpeta vacía.
2. Ejecutar `GOWA-cMc.exe`.
3. Elegir la carpeta del disco que contiene `PS3_GAME`.
4. Esperar la compilación de shaders.
5. Llegar al menú.
6. Guardar y cerrar.
7. Volver a abrir.

No publicar si falla.

---

## 5. Publicación

### Opción A: script local

```powershell
work\0.2.0-full\scripts\publish_github.ps1 -Version v0.1.6
```

El script crea el tag, lo sube y crea el release con el ZIP.

### Opción B: comandos manuales

```powershell
git -C "work\0.2.0-full" push -u origin master
git -C "work\0.2.0-full" tag v0.1.6
git -C "work\0.2.0-full" push origin v0.1.6
gh release create v0.1.6 `
  "work\dist\GOWA_cMc_OmniPort v0.1.6.zip" `
  --repo cMc292/GOWA_cMc_OmniPort `
  --title "GOWA_cMc_OmniPort v0.1.6" `
  --generate-notes
```

### Opción C: GitHub Actions

Requiere un runner self-hosted:

```text
self-hosted, windows, gowa
GOWA_FULL_ROOT=C:\deepseek harness\Port de God of War Ascension BCUS98232\work\0.2.0-full
```

Después:

```text
Actions -> Release portable -> Run workflow
Version: v0.1.6
```

---

## 6. Checksum del release

Crear `SHA256SUMS.txt`:

```powershell
Get-FileHash `
  "work\dist\GOWA_cMc_OmniPort v0.1.6.zip" `
  -Algorithm SHA256 |
  ForEach-Object { "$($_.Hash)  GOWA_cMc_OmniPort v0.1.6.zip" } |
  Set-Content -LiteralPath "work\dist\SHA256SUMS.txt" -Encoding ASCII
```

Adjuntar `SHA256SUMS.txt` al release.

---

## 7. Decisión legal pendiente

El ZIP contiene `GOWA.elf`, un archivo derivado del juego. Antes de publicar:

- **A:** publicar tal cual, con `GOWA.elf`;
- **B:** publicar sin `GOWA.elf` y exigir el ELF del usuario.

La opción B requiere cambiar el README y el flujo de arranque. No cambiar
esto sin decisión explícita del autor.

---

## 8. No hacer

- No ejecutar `make_portable.py` con el exe `5FA6…`.
- No usar `build/GOWA-cMc.exe` como release.
- No copiar el juego dentro del ZIP.
- No borrar `build/GOWA-cMc.known-good.exe`.
- No borrar el `0.2.0-portable` mientras sea la referencia del ejecutable.
- No publicar desde una sesión que no haya leído este documento.
