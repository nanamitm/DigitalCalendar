param(
    [string]$QtVersion = "6.9.2",
    [string]$QtRoot = "C:\Qt",
    [string]$CMakePath = "",
    [string]$NinjaPath = "",
    [string]$MingwBin = ""
)

$ErrorActionPreference = "Stop"

$qtPrefix = Join-Path $QtRoot "$QtVersion\mingw_64"
$cmake = if ($CMakePath) { $CMakePath } elseif (Test-Path "$QtRoot\Tools\CMake_64\bin\cmake.exe") { "$QtRoot\Tools\CMake_64\bin\cmake.exe" } else { "cmake.exe" }
$ninja = if ($NinjaPath) { $NinjaPath } elseif (Test-Path "$QtRoot\Tools\Ninja\ninja.exe") { "$QtRoot\Tools\Ninja\ninja.exe" } else { "ninja.exe" }
$mingw = if ($MingwBin) { $MingwBin } elseif (Test-Path "$QtRoot\Tools\mingw1310_64\bin") { "$QtRoot\Tools\mingw1310_64\bin" } else { "" }

if (!(Test-Path "$qtPrefix\lib\cmake\Qt6\Qt6Config.cmake")) {
    throw "Qt desktop kit not found: $qtPrefix"
}

if ($mingw) {
    $env:Path = "$qtPrefix\bin;$mingw;$env:Path"
} else {
    $env:Path = "$qtPrefix\bin;$env:Path"
}

& $cmake -S . -B build -G Ninja `
    -DCMAKE_PREFIX_PATH="$qtPrefix" `
    -DCMAKE_MAKE_PROGRAM="$ninja" `
    -DCMAKE_BUILD_TYPE=Release

& $cmake --build build
