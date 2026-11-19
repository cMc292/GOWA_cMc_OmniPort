param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    [string]$Repo = "cMc292/GOWA_cMc_OmniPort",
    [switch]$Draft,
    [switch]$NoPush
)

$ErrorActionPreference = "Stop"
if ($Version -notmatch '^v\d+\.\d+\.\d+([.-][0-9A-Za-z.-]+)?$') {
    throw "Version invalida: $Version (ejemplo: v0.1.5)"
}

$Full = Split-Path $PSScriptRoot -Parent
$Python = Join-Path $Full "tools\python\python.exe"
$Packager = Join-Path $Full "scripts\make_portable.py"
$Zip = Join-Path $Full ("..\dist\GOWA_cMc_OmniPort " + $Version + ".zip")
$Zip = [IO.Path]::GetFullPath($Zip)

if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    throw "GitHub CLI (gh) no esta instalado. Instala gh y ejecuta: gh auth login"
}
& gh auth status *> $null
if ($LASTEXITCODE -ne 0) {
    throw "gh no esta autenticado. Ejecuta: gh auth login"
}

Push-Location $Full
try {
    $dirty = (& git status --porcelain) -join "`n"
    if ($dirty.Trim().Length -ne 0) {
        throw "El arbol tiene cambios sin commit. Commitea antes de publicar.`n$dirty"
    }

    & $Python $Packager --version $Version
    if ($LASTEXITCODE -ne 0) { throw "El empaquetador fallo" }
    if (-not (Test-Path -LiteralPath $Zip)) { throw "No se creo el ZIP: $Zip" }

    $tag = $Version
    $existing = (& git tag --list $tag)
    if ($existing) { throw "El tag $tag ya existe. Usa otra version." }

    & git tag $tag
    if ($LASTEXITCODE -ne 0) { throw "No se pudo crear el tag $tag" }

    if (-not $NoPush) {
        & git push origin $tag
        if ($LASTEXITCODE -ne 0) { throw "No se pudo subir el tag. Configura origin a https://github.com/$Repo.git" }
    }

    $ghArgs = @("release", "create", $tag, $Zip, "--repo", $Repo,
              "--title", "GOWA_cMc_OmniPort $Version", "--generate-notes")
    if ($Draft) { $ghArgs += "--draft" }
    & gh @ghArgs
    if ($LASTEXITCODE -ne 0) { throw "gh release create fallo" }

    Write-Host ""
    Write-Host "Publicado: https://github.com/$Repo/releases/tag/$tag"
    Write-Host "ZIP:       $Zip"
}
finally {
    Pop-Location
}
