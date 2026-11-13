<#
    build-launchers.ps1 - reconstruye los lanzadores .exe del paquete.
    Necesita Visual Studio 2022 Build Tools (clang-cl y rc.exe). Copia el icono,
    compila con /MT y reparte el mismo binario con los cinco nombres.
#>
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$pkg = Split-Path -Parent (Split-Path -Parent $here)
$vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $vcvars)) { throw "No esta vcvars64.bat: $vcvars" }

Copy-Item -LiteralPath (Join-Path $pkg 'assets\icon\app.ico') -Destination (Join-Path $here 'app.ico') -Force
$cmd = 'call "' + $vcvars + '" >nul && rc.exe /nologo /fo "' + (Join-Path $here 'gowalaunch.res') + '" "' + (Join-Path $here 'gowalaunch.rc') + '" && clang-cl /nologo /O2 /MT /DUNICODE /D_UNICODE "' + (Join-Path $here 'gowalaunch.c') + '" "' + (Join-Path $here 'gowalaunch.res') + '" /link user32.lib /SUBSYSTEM:WINDOWS /OUT:"' + (Join-Path $here 'gowalaunch.exe') + '"'
cmd /c $cmd
if ($LASTEXITCODE -ne 0) { throw "la compilacion fallo ($LASTEXITCODE)" }

$names = @('Jugar GOWA-cMc.exe', 'Launcher GOWA-cMc.exe', 'Update-BCUS.exe', 'Promote-BCUS.exe', 'Rollback-BCUS.exe')
foreach ($n in $names) {
    Copy-Item -LiteralPath (Join-Path $here 'gowalaunch.exe') -Destination (Join-Path $pkg $n) -Force
    Write-Host ('  ' + $n)
}
Write-Host 'lanzadores listos'
