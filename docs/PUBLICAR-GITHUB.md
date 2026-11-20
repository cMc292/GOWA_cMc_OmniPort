# Publicar GOWA_cMc_OmniPort en GitHub

Repositorio: <https://github.com/cMc292/GOWA_cMc_OmniPort>

El paquete portable se genera siempre desde:

```text
work\0.2.0-full
```

## Requisitos

- Git y GitHub CLI (`gh`) instalados.
- `gh auth login` hecho en la PC que publica.
- El full compilado y con el juego probado.
- La build commiteada en git.

## Publicación local (recomendada)

Desde la raíz del proyecto:

```powershell
work\0.2.0-full\tools\python\python.exe work\0.2.0-full\scripts\make_portable.py --version v0.1.6
```

Luego:

```powershell
gh release create v0.1.6 `
  "work\dist\GOWA_cMc_OmniPort v0.1.6.zip" `
  --repo cMc292/GOWA_cMc_OmniPort `
  --title "GOWA_cMc_OmniPort v0.1.6" `
  --generate-notes
```

O usa el script:

```powershell
work\0.2.0-full\scripts\publish_github.ps1 -Version v0.1.6
```

El script:

1. comprueba que `gh` está autenticado;
2. comprueba que el árbol está limpio;
3. ejecuta el empaquetador;
4. exige que el ZIP y el ejecutable existan;
5. crea el tag y lo sube;
6. crea el release y sube el ZIP.

No guarda tokens ni contraseñas.

## GitHub Actions con runner self-hosted

El juego y el `GOWA.elf` no se pueden subir a GitHub. Por eso el workflow
`release.yml` corre en un runner **self-hosted de Windows** que ya tiene:

- el árbol completo en `GOWA_FULL_ROOT`;
- el juego BCUS98232 instalado;
- el ELF descifrado;
- el build compilado.

Etiquetas del runner:

```text
self-hosted, windows, gowa
```

Variable del runner:

```text
GOWA_FULL_ROOT=C:\deepseek harness\Port de God of War Ascension BCUS98232\work\0.2.0-full
```

Después, desde GitHub:

```text
Actions -> Release portable -> Run workflow
Version: v0.1.6
```

El workflow genera el paquete y crea el release usando `GH_TOKEN` del runner.

## README y LEEME

El empaquetador genera automáticamente:

```text
README.md
LEEME-PRIMERO.txt
BUILD-INFO.txt
```

con la versión, fecha y hash del ejecutable. No se copian los README
antiguos del paquete alpha.

## Comprobaciones antes de publicar

- `GOWA-cMc.exe` del ZIP coincide con el del full.
- El portable no tiene `vfs`, `gamedata`, `logs` ni caché de shaders.
- El portable no tiene rutas `C:\`, `D:\` o `E:\`.
- `dxcompiler.dll`, `dxil.dll` y el runtime VC están incluidos.
- El ZIP contiene `README.md`, `LEEME-PRIMERO.txt`, `BUILD-INFO.txt`,
  licencias y créditos.
- La versión del nombre, del README y del release es la misma.
