$ErrorActionPreference = "Stop"

$sdkRoot = "$env:LOCALAPPDATA\Android\Sdk"
$adb = "$sdkRoot\platform-tools\adb.exe"
$apk = ".\build-android\DigitalCalendarAndroid-debug.apk"

& $adb install -r $apk
