# Android dependency sources

Place the following sources here for Android builds:

- `third_party/zlib` — zlib source tree (CMake-capable).
- `third_party/libxml2` — libxml2 source tree (CMake-capable or autotools).
- `third_party/freetype` — freetype source tree (CMake-capable).
- `third_party/libjpeg-turbo` — libjpeg source tree (CMake-capable).
- `third_party/libpng` — libpng source tree (CMake-capable).

Pinned upstreams used in Flatpak:
- libxml2: https://github.com/BigRangaTech/libxml2.git (tag v001)
- freetype: https://github.com/freetype/freetype (tag VER-2-13-2)
- libjpeg-turbo: https://github.com/libjpeg-turbo/libjpeg-turbo (tag 3.0.2)
- libpng: https://github.com/pnggroup/libpng (tag v1.6.43)

After adding these sources, run:
```
ANDROID_NDK=/path/to/ndk \
ANDROID_ABI=arm64-v8a \
ANDROID_API=24 \
./scripts/android/build_android_deps.sh
```
