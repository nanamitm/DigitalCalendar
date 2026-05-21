# DigitalCalendar

Qt 6 / C++ で作成した、3か月表示のデジタルカレンダーです。Windows ではデスクトップアクセサリーとして、Android ではタブレットなどで横画面の常時表示カレンダーとして使う想定です。

## 特徴

- 当月を大きく、前月と翌月を小さく表示
- 秒付きデジタル時計
- 日付変更時に自動で Today 表示へ復帰
- 時計クリックで 12時間 / 24時間表示を切り替え
- Windows 版は右クリックメニューで常に最前面を切り替え
- Android 版は横画面固定、全画面表示、常時点灯
- Android 版は Auto / Light / Dark をアプリ内で切り替え
- Android 16 KB page size 対応
- Android APK では不要な Qt プラグインを削減済み

## フォルダー構成

```text
desktop/src/      Windows / desktop Qt Widgets app
android/src/      Android Qt Widgets app
android/package/  Android manifest, icon, Gradle template
.github/          GitHub Actions workflow
```

`build/`、`build-*`、`keystore/`、`android-signing.properties` はローカル生成物として git 管理しません。

## 必要な環境

Windows 版:

- Qt 6.9.2 MinGW 64-bit
- CMake
- Ninja
- MinGW 13.1.0

Android 版:

- Qt 6.11.1 `android_arm64_v8a`
- Android SDK platform 36
- Android Build Tools 36.0.0
- Android NDK 27.2.12479018
- Java 17

## Windows 版ビルド

標準設定では `C:\Qt\6.9.2\mingw_64` を使います。

```powershell
.\build.ps1
```

別の Qt ルートやバージョンを使う場合:

```powershell
.\build.ps1 -QtRoot C:\Qt -QtVersion 6.11.1
```

実行:

```powershell
.\run.ps1
```

配布用 ZIP を作成:

```powershell
.\package_windows.ps1
```

生成物:

```text
dist/DigitalCalendar-windows.zip
```

## Android debug APK

```powershell
.\build_android.ps1
```

生成物:

```text
build-android/DigitalCalendarAndroid-debug.apk
```

接続中の端末またはエミュレータへインストール:

```powershell
.\install_android.ps1
```

16 KB page size 対応チェック:

```powershell
.\check_android_16kb.ps1 -ApkPath .\build-android\DigitalCalendarAndroid-debug.apk
```

## Android release APK

署名用 keystore はリポジトリに含めません。ローカルで一度作成して、安全な場所に保管してください。

```powershell
.\create_android_keystore.ps1
```

生成されるローカル秘密ファイル:

```text
keystore/digitalcalendar-release.jks
android-signing.properties
```

署名付き release APK:

```powershell
.\build_android_release.ps1
```

生成物:

```text
build-android-release/DigitalCalendarAndroid-release.apk
```

## 配布

配布用 APK は GitHub Releases からダウンロードできます。

```text
DigitalCalendarAndroid-release.apk
```

GitHub Actions の artifact からも、debug APK と署名付き release APK を取得できます。
Windows 版は `DigitalCalendar-windows.zip` として artifact にアップロードされます。

## GitHub Actions

`.github/workflows/build.yml` で Android APK をビルドします。

通常の push / pull request では Windows ZIP と Android debug APK を作成し、artifact としてアップロードします。署名付き Android release APK も作りたい場合は、GitHub repository secrets に次を設定します。

```text
ANDROID_KEYSTORE_BASE64
ANDROID_STORE_PASSWORD
ANDROID_KEY_ALIAS
ANDROID_KEY_PASSWORD
```

`ANDROID_KEYSTORE_BASE64` は keystore を Base64 化した文字列です。

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes(".\keystore\digitalcalendar-release.jks"))
```

Secrets がすべて設定されている場合だけ、Actions は署名付き release APK も作成して artifact にアップロードします。

## ライセンス

MIT License
