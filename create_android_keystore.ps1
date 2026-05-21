param(
    [string]$KeystorePath = ".\keystore\digitalcalendar-release.jks",
    [string]$Alias = "digitalcalendar",
    [string]$CommonName = "Digital Calendar",
    [string]$OrganizationalUnit = "Personal",
    [string]$Organization = "nanamitm",
    [string]$Country = "JP",
    [int]$ValidityDays = 10950
)

$ErrorActionPreference = "Stop"

$keytool = "C:\Program Files\Android\Android Studio\jbr\bin\keytool.exe"
if (!(Test-Path $keytool)) {
    throw "keytool not found: $keytool"
}

$keystoreFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($KeystorePath)
$keystoreDir = Split-Path -Parent $keystoreFullPath
New-Item -ItemType Directory -Force -Path $keystoreDir | Out-Null

if (Test-Path $keystoreFullPath) {
    throw "Keystore already exists: $keystoreFullPath"
}

$storePassword = Read-Host "Keystore password" -AsSecureString
$keyPassword = Read-Host "Key password" -AsSecureString

$storePasswordText = [System.Net.NetworkCredential]::new("", $storePassword).Password
$keyPasswordText = [System.Net.NetworkCredential]::new("", $keyPassword).Password

$distinguishedName = "CN=$CommonName, OU=$OrganizationalUnit, O=$Organization, C=$Country"

& $keytool -genkeypair `
    -v `
    -keystore $keystoreFullPath `
    -storetype JKS `
    -alias $Alias `
    -keyalg RSA `
    -keysize 2048 `
    -validity $ValidityDays `
    -dname $distinguishedName `
    -storepass $storePasswordText `
    -keypass $keyPasswordText

@"
storeFile=$KeystorePath
storePassword=$storePasswordText
keyAlias=$Alias
keyPassword=$keyPasswordText
"@ | Set-Content -Encoding utf8NoBOM -Path ".\android-signing.properties"

Write-Host "Created keystore: $keystoreFullPath"
Write-Host "Created local signing properties: android-signing.properties"
Write-Host "Keep both files private. They are ignored by .gitignore."
