# Android port plan (Qt 6.10.2)

## Goals
- Keep **comics early** (CBZ/CBR via libarchive).
- Rebuild all format dependencies for Android.
- Defer desktop-only services (libsecret keychain, speech-dispatcher/espeak).

## Dependency matrix
### Format dependencies
- **CBZ/CBR**: libarchive (CBR support depends on libarchive’s RAR support).
- **PDF**: QtPdf module (Qt 6.10.2 Android kit).
- **DJVU**: DjVuLibre tools (`ddjvu`, `djvused`, `djvutxt`) or a future in-process API.
- **MOBI/AZW**: libmobi (vendored sources).
- **EPUB/FB2**: libxml2 (FB2) + miniz (ZIP; vendored).
- **TXT**: none.

### Platform dependencies (Android)
- Qt Widgets is not used on Android builds.
- Android TTS + keychain deferred (disabled for now).
- SQLite is required for the core DB; Android builds use the bundled amalgamation.

## Build strategy
### 1) Cross-compile third-party libraries
We’ll build and install Android libs to:

```
third_party/install/android/<ABI>/<dep>
```

Use the build helper:
```
ANDROID_NDK=/path/to/ndk \
ANDROID_ABI=arm64-v8a \
ANDROID_API=24 \
./scripts/android/build_android_deps.sh
```
Notes:
- Poppler Qt6 is not built for Android; the app uses QtPdf on Android.
- If you intentionally want to attempt Poppler later, set `BUILD_POPPLER=1` and provide the Poppler source.

### 2) Android CMake configuration
- Qt Widgets is linked only on non-Android.
- Android builds use `QGuiApplication`.
- Bundled Poppler/Libarchive are resolved from the Android install paths.
- APK target is available via CMake (`--target apk`).

### 3) File access on Android
- Use native Android file/folder pickers (SAF) or manual fallback dialog.
- Library DB and caches go into app data.

## Requirements before building Android deps
You must provide these sources (vendored or cloned):
- `third_party/zlib` (zlib source)
- `third_party/libxml2` (libxml2 source)
- `third_party/freetype` (freetype source)
- `third_party/libjpeg-turbo` (libjpeg source)
- `third_party/libpng` (libpng source)
- `third_party/sqlite` (SQLite amalgamation: sqlite3.c/sqlite3.h/sqlite3ext.h)

Ensure the Qt 6.10.2 Android kit includes the **QtPdf** module (Qt6::Pdf).

The flatpak manifest uses a pinned libxml2 repo. Use the same pinned source for Android.
zlib is pinned to v1.3.1 in `deps.lock`.
freetype is pinned to VER-2-13-2 in `deps.lock`.
libjpeg-turbo is pinned to 3.0.2 in `deps.lock`.
libpng is pinned to v1.6.43 in `deps.lock`.

## Next steps
1. Vendor missing sources (`zlib`, `libxml2`) into `third_party/`.
2. Run `scripts/android/build_android_deps.sh`.
3. Configure Qt 6.10.2 Android kit and build the app.
   - Example:
     ```
     cmake -S . -B build-android-arm64 -GNinja \
       -DANDROID_ABI=arm64-v8a \
       -DANDROID_PLATFORM=android-24 \
       -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
       -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH \
       -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH \
       -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH \
       -DQt6_DIR=$QT_ANDROID_PREFIX/lib/cmake/Qt6 \
       -DCMAKE_PREFIX_PATH=$QT_ANDROID_PREFIX
     cmake --build build-android-arm64 --parallel
     cmake --build build-android-arm64 --target apk
     ```
4. Validate format support (CBZ/CBR, PDF, DJVU, MOBI, EPUB/FB2).
