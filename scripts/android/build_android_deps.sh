#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

ANDROID_NDK="${ANDROID_NDK:-${ANDROID_NDK_ROOT:-}}"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a}"
ANDROID_API="${ANDROID_API:-24}"

if [[ -z "$ANDROID_NDK" ]]; then
  echo "ANDROID_NDK (or ANDROID_NDK_ROOT) is not set" >&2
  exit 1
fi

TOOLCHAIN="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
if [[ ! -f "$TOOLCHAIN" ]]; then
  echo "Android toolchain not found: $TOOLCHAIN" >&2
  exit 1
fi

BUILD_ROOT="$ROOT/third_party/build/android/$ANDROID_ABI"
INSTALL_ROOT="$ROOT/third_party/install/android/$ANDROID_ABI"

prefix_paths=()

prefix_arg() {
  local IFS=";"
  echo "${prefix_paths[*]}"
}

common_cmake_args=(
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
  -DANDROID_ABI="$ANDROID_ABI"
  -DANDROID_PLATFORM="android-$ANDROID_API"
  -DCMAKE_BUILD_TYPE=Release
  -DBUILD_SHARED_LIBS=ON
)

cmake_build() {
  local name="$1"
  local src="$2"
  shift 2
  local build_dir="$BUILD_ROOT/$name"
  local install_dir="$INSTALL_ROOT/$name"
  mkdir -p "$build_dir" "$install_dir"
  local cmake_prefix
  cmake_prefix="$(prefix_arg)"
  local prefix_args=()
  if [[ -n "$cmake_prefix" ]]; then
    prefix_args=("-DCMAKE_PREFIX_PATH=$cmake_prefix")
  fi
  cmake -S "$src" -B "$build_dir" \
    "${common_cmake_args[@]}" \
    "${prefix_args[@]}" \
    -DCMAKE_INSTALL_PREFIX="$install_dir" \
    "$@"
  cmake --build "$build_dir" --parallel
  cmake --install "$build_dir"
  prefix_paths+=("$install_dir")
}

build_zlib() {
  local src="$ROOT/third_party/zlib"
  if [[ ! -d "$src" ]]; then
    echo "zlib source not found at $src" >&2
    echo "Add a zlib snapshot to third_party/zlib or update this script." >&2
    return 1
  fi
  cmake_build zlib "$src" -DZLIB_BUILD_EXAMPLES=OFF
}

build_libxml2() {
  local src="$ROOT/third_party/libxml2"
  if [[ ! -d "$src" ]]; then
    echo "libxml2 source not found at $src" >&2
    echo "Clone your pinned libxml2 into third_party/libxml2 (see flatpak manifest)." >&2
    return 1
  fi
  if [[ -f "$src/CMakeLists.txt" ]]; then
    cmake_build libxml2 "$src" -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_ICU=OFF
    return 0
  fi
  echo "libxml2 CMakeLists.txt not found; add a CMake-capable snapshot or update this script." >&2
  return 1
}

build_freetype() {
  local src="$ROOT/third_party/freetype"
  if [[ ! -d "$src" ]]; then
    echo "freetype source not found at $src" >&2
    return 1
  fi
  cmake_build freetype "$src" -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BROTLI=ON
}

build_libjpeg() {
  local src="$ROOT/third_party/libjpeg-turbo"
  if [[ ! -d "$src" ]]; then
    echo "libjpeg-turbo source not found at $src" >&2
    return 1
  fi
  cmake_build libjpeg-turbo "$src" -DENABLE_SHARED=ON -DENABLE_STATIC=OFF -DWITH_SIMD=OFF
}

build_libpng() {
  local src="$ROOT/third_party/libpng"
  if [[ ! -d "$src" ]]; then
    echo "libpng source not found at $src" >&2
    return 1
  fi
  cmake_build libpng "$src" -DPNG_TESTS=OFF
}

build_libarchive() {
  local src="$ROOT/third_party/libarchive"
  if [[ ! -d "$src" ]]; then
    echo "libarchive source not found at $src" >&2
    return 1
  fi
  cmake_build libarchive "$src" \
    -DENABLE_OPENSSL=OFF \
    -DENABLE_LZ4=OFF \
    -DENABLE_ZSTD=OFF \
    -DENABLE_BZIP2=OFF \
    -DENABLE_ICONV=OFF \
    -DENABLE_CNG=OFF \
    -DENABLE_CAT=OFF \
    -DENABLE_TAR=OFF \
    -DENABLE_CPIO=OFF \
    -DENABLE_TEST=OFF
}

build_poppler() {
  local src="$ROOT/poppler"
  if [[ ! -d "$src" ]]; then
    echo "Poppler source not found at $src" >&2
    return 1
  fi
  local freetype_prefix="$INSTALL_ROOT/freetype"
  local jpeg_prefix="$INSTALL_ROOT/libjpeg-turbo"
  local png_prefix="$INSTALL_ROOT/libpng"
  cmake_build poppler "$src" \
    -DBUILD_QT6_TESTS=OFF \
    -DBUILD_GTK_TESTS=OFF \
    -DBUILD_CPP_TESTS=OFF \
    -DBUILD_MANUAL_TESTS=OFF \
    -DENABLE_UNSTABLE_API_ABI_HEADERS=OFF \
    -DENABLE_CPP=OFF \
    -DENABLE_GLIB=OFF \
    -DENABLE_GTK_DOC=OFF \
    -DENABLE_GPGME=OFF \
    -DENABLE_BOOST=OFF \
    -DENABLE_LIBCURL=OFF \
    -DENABLE_QT5=OFF \
    -DENABLE_QT6=ON \
    -DENABLE_NSS3=OFF \
    -DENABLE_LIBTIFF=OFF \
    -DENABLE_DCTDECODER=libjpeg \
    -DFREETYPE_INCLUDE_DIRS="$freetype_prefix/include/freetype2" \
    -DFREETYPE_LIBRARY="$freetype_prefix/lib/libfreetype.so" \
    -DJPEG_INCLUDE_DIR="$jpeg_prefix/include" \
    -DJPEG_LIBRARY="$jpeg_prefix/lib/libjpeg.so" \
    -DPNG_PNG_INCLUDE_DIR="$png_prefix/include" \
    -DPNG_LIBRARY="$png_prefix/lib/libpng16.so"
}

build_djvulibre() {
  local src="$ROOT/third_party/djvulibre"
  if [[ ! -d "$src" ]]; then
    echo "DjVuLibre source not found at $src" >&2
    return 1
  fi
  local build_dir="$BUILD_ROOT/djvulibre"
  local install_dir="$INSTALL_ROOT/djvulibre"
  mkdir -p "$build_dir" "$install_dir"

  local host_tag
  case "$ANDROID_ABI" in
    arm64-v8a) host_tag=aarch64-linux-android ;;
    armeabi-v7a) host_tag=armv7a-linux-androideabi ;;
    x86_64) host_tag=x86_64-linux-android ;;
    x86) host_tag=i686-linux-android ;;
    *) echo "Unsupported ANDROID_ABI: $ANDROID_ABI" >&2; return 1 ;;
  esac

  local toolchain_bin="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
  if [[ ! -d "$toolchain_bin" ]]; then
    toolchain_bin="$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin"
  fi

  local cc="$toolchain_bin/${host_tag}${ANDROID_API}-clang"
  local cxx="$toolchain_bin/${host_tag}${ANDROID_API}-clang++"

  if [[ ! -x "$cc" ]]; then
    echo "Clang not found for ABI: $ANDROID_ABI (expected $cc)" >&2
    return 1
  fi

  if [[ -x "$src/configure" ]]; then
    (cd "$build_dir" && \
      CC="$cc" CXX="$cxx" \
      "$src/configure" --host="$host_tag" --prefix="$install_dir")
    cmake --build "$build_dir" --parallel || make -C "$build_dir" -j"$(nproc || echo 4)"
    make -C "$build_dir" install
    prefix_paths+=("$install_dir")
    return 0
  fi

  if [[ -x "$src/autogen.sh" ]]; then
    (cd "$src" && ./autogen.sh)
    (cd "$build_dir" && \
      CC="$cc" CXX="$cxx" \
      "$src/configure" --host="$host_tag" --prefix="$install_dir")
    cmake --build "$build_dir" --parallel || make -C "$build_dir" -j"$(nproc || echo 4)"
    make -C "$build_dir" install
    prefix_paths+=("$install_dir")
    return 0
  fi

  echo "DjVuLibre configure script not found; ensure the source tree is complete." >&2
  return 1
}

build_zlib
build_libxml2
build_freetype
build_libjpeg
build_libpng
build_libarchive
build_poppler
build_djvulibre

echo "Android deps installed to: $INSTALL_ROOT"
