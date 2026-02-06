# Android port plan (Qt 6.7)

## Goals
- Keep **comics early** (CBZ/CBR via libarchive).
- Rebuild all format dependencies for Android.
- Defer desktop-only services (libsecret keychain, speech-dispatcher/espeak).

## Dependency matrix
### Format dependencies
- **CBZ/CBR**: libarchive (CBR support depends on libarchive’s RAR support).
- **PDF**: Poppler Qt6 build.
- **DJVU**: DjVuLibre tools (`ddjvu`, `djvused`, `djvutxt`) or a future in-process API.
- **MOBI/AZW**: libmobi (vendored sources).
- **EPUB/FB2**: libxml2 (FB2) + miniz (ZIP; vendored).
- **TXT**: none.

### Platform dependencies (Android)
- Qt Widgets is not used on Android builds.
- Android TTS + keychain deferred (disabled for now).

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

### 2) Android CMake configuration
- Qt Widgets is linked only on non-Android.
- Android builds use `QGuiApplication`.
- Bundled Poppler/Libarchive are resolved from the Android install paths.

### 3) File access on Android
- Use native Android file/folder pickers (SAF) or manual fallback dialog.
- Library DB and caches go into app data.

## Requirements before building Android deps
You must provide these sources (vendored or cloned):
- `third_party/zlib` (zlib source)
- `third_party/libxml2` (libxml2 source)

The flatpak manifest uses a pinned libxml2 repo. Use the same pinned source for Android.
zlib is pinned to v1.3.1 in `deps.lock`.

## Next steps
1. Vendor missing sources (`zlib`, `libxml2`) into `third_party/`.
2. Run `scripts/android/build_android_deps.sh`.
3. Configure Qt 6.7 Android kit and build the app.
4. Validate format support (CBZ/CBR, PDF, DJVU, MOBI, EPUB/FB2).
