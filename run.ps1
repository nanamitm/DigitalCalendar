$ErrorActionPreference = "Stop"

$qtPrefix = "C:\Qt\6.9.2\mingw_64"
$mingwBin = "C:\Qt\Tools\mingw1310_64\bin"

$env:Path = "$qtPrefix\bin;$mingwBin;$env:Path"

& ".\build\DigitalCalendar.exe"
