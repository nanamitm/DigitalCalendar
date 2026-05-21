param(
    [string]$QtVersion = "6.9.2",
    [string]$QtRoot = "C:\Qt",
    [string]$CMakePath = "",
    [string]$NinjaPath = "",
    [string]$MingwBin = ""
)

$ErrorActionPreference = "Stop"

& .\build.ps1 `
    -QtVersion $QtVersion `
    -QtRoot $QtRoot `
    -CMakePath $CMakePath `
    -NinjaPath $NinjaPath `
    -MingwBin $MingwBin

$qtPrefix = Join-Path $QtRoot "$QtVersion\mingw_64"
$windeployqt = Join-Path $qtPrefix "bin\windeployqt.exe"
$exePath = ".\build\DigitalCalendar.exe"
$packageDir = ".\dist\DigitalCalendar-windows"
$zipPath = ".\dist\DigitalCalendar-windows.zip"

if (!(Test-Path $windeployqt)) {
    throw "windeployqt not found: $windeployqt"
}
if (!(Test-Path $exePath)) {
    throw "Executable not found: $exePath"
}

if (Test-Path $packageDir) {
    Remove-Item -LiteralPath $packageDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
Copy-Item -LiteralPath $exePath -Destination $packageDir -Force

& $windeployqt `
    --release `
    --no-translations `
    --no-system-d3d-compiler `
    --no-opengl-sw `
    (Join-Path $packageDir "DigitalCalendar.exe")

if (Test-Path $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
Compress-Archive -LiteralPath $packageDir -DestinationPath $zipPath -Force

Write-Host "Windows package: $zipPath"
