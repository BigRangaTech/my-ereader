# Android dependency sources

Place the following sources here for Android builds:

- `third_party/zlib` — zlib source tree (CMake-capable).
- `third_party/libxml2` — libxml2 source tree (CMake-capable or autotools).

Pinned upstreams used in Flatpak:
- libxml2: https://github.com/BigRangaTech/libxml2.git (tag v001)

After adding these sources, run:
```
ANDROID_NDK=/path/to/ndk \
ANDROID_ABI=arm64-v8a \
ANDROID_API=24 \
./scripts/android/build_android_deps.sh
```
