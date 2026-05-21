param(
    [Alias("ApkPath")]
    [string]$Apk = ".\build-android\android-build\build\outputs\apk\debug\android-build-debug.apk",
    [string]$NdkVersion = "27.2.12479018"
)

$ErrorActionPreference = "Stop"

$androidSdkRoot = if ($env:ANDROID_HOME) {
    $env:ANDROID_HOME
} elseif ($env:ANDROID_SDK_ROOT) {
    $env:ANDROID_SDK_ROOT
} else {
    "$env:LOCALAPPDATA\Android\Sdk"
}
$readelf = Join-Path $androidSdkRoot "ndk\$NdkVersion\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-readelf.exe"
$apkName = [System.IO.Path]::GetFileNameWithoutExtension($Apk)
$extractDir = ".\build-android\check-16kb\$apkName"

if (!(Test-Path $Apk)) {
    throw "APK not found: $Apk"
}
if (!(Test-Path $readelf)) {
    throw "llvm-readelf not found: $readelf"
}

if (Test-Path $extractDir) {
    Remove-Item -LiteralPath $extractDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $extractDir | Out-Null

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory((Resolve-Path $Apk), (Resolve-Path $extractDir))

$failed = @()
Get-ChildItem "$extractDir\lib" -Recurse -Filter "*.so" | ForEach-Object {
    $output = & $readelf -l "$($_.FullName)"
    $loadLines = $output | Where-Object { $_ -match '^\s*LOAD\s' }
    $badLines = $loadLines | Where-Object { $_ -notmatch '0x4000\s*$' }
    if ($badLines.Count -gt 0) {
        $failed += $_.FullName
        Write-Host "[4KB or mixed] $($_.FullName)"
        $badLines | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "[16KB OK] $($_.FullName)"
    }
}

if ($failed.Count -gt 0) {
    throw "$($failed.Count) native libraries are not 16KB aligned."
}

Write-Host "All native libraries are 16KB aligned."
