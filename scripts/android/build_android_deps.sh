#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

ANDROID_NDK="${ANDROID_NDK:-${ANDROID_NDK_ROOT:-}}"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a}"
ANDROID_API="${ANDROID_API:-24}"
QT_ANDROID_PREFIX="${QT_ANDROID_PREFIX:-${QT6_ANDROID_PREFIX:-}}"
BUILD_POPPLER="${BUILD_POPPLER:-0}"

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
if [[ -n "$QT_ANDROID_PREFIX" ]]; then
  prefix_paths+=("$QT_ANDROID_PREFIX")
fi

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

build_sqlite() {
  local src="$ROOT/third_party/sqlite"
  if [[ ! -d "$src" ]]; then
    echo "sqlite source not found at $src" >&2
    echo "Add the SQLite amalgamation (sqlite3.c, sqlite3.h, sqlite3ext.h) and CMakeLists.txt." >&2
    return 1
  fi
  if [[ ! -f "$src/sqlite3.c" ]]; then
    echo "sqlite3.c not found in $src" >&2
    echo "Place the SQLite amalgamation files in third_party/sqlite." >&2
    return 1
  fi
  cmake_build sqlite "$src" -DSQLITE_OMIT_LOAD_EXTENSION=ON
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
  if [[ -z "$QT_ANDROID_PREFIX" ]]; then
    echo "QT_ANDROID_PREFIX is not set. Set it to your Qt 6.10.2 Android kit path (e.g. ~/Qt/6.10.2/android_arm64_v8a)" >&2
    return 1
  fi
  local qt_cmake_root="$QT_ANDROID_PREFIX/lib/cmake"
  local qt_cmake_dir="$qt_cmake_root/Qt6"
  local qt_widgets_dir="$qt_cmake_root/Qt6Widgets"
  if [[ ! -d "$qt_cmake_dir" ]]; then
    echo "Qt6 CMake config not found at $qt_cmake_dir" >&2
    return 1
  fi
  if [[ ! -d "$qt_widgets_dir" ]]; then
    echo "Qt6Widgets CMake config not found at $qt_widgets_dir" >&2
    return 1
  fi
  local freetype_prefix="$INSTALL_ROOT/freetype"
  local jpeg_prefix="$INSTALL_ROOT/libjpeg-turbo"
  local png_prefix="$INSTALL_ROOT/libpng"
  cmake_build poppler "$src" \
    -DQt6_DIR="$qt_cmake_dir" \
    -DQt6Core_DIR="$qt_cmake_root/Qt6Core" \
    -DQt6Gui_DIR="$qt_cmake_root/Qt6Gui" \
    -DQt6Xml_DIR="$qt_cmake_root/Qt6Xml" \
    -DQt6Widgets_DIR="$qt_widgets_dir" \
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
    -DENABLE_LIBOPENJPEG=none \
    -DENABLE_LCMS=OFF \
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

  # Autoconf scripts in DjVuLibre reject spaces in srcdir/build paths.
  # Use a temp symlink/build/install dir without spaces when needed.
  local src_for_build="$src"
  local build_for_make="$build_dir"
  local install_for_make="$install_dir"
  local need_safe_paths=0
  if [[ "$src" == *" "* || "$build_dir" == *" "* ]]; then
    need_safe_paths=1
    local safe_root="/tmp/ereader_android_build/${ANDROID_ABI}"
    local safe_src="$safe_root/src/djvulibre"
    local safe_build="$safe_root/build/djvulibre"
    local safe_install="$safe_root/install/djvulibre"
    mkdir -p "$safe_root/src" "$safe_root/build"
    if [[ ! -L "$safe_src" ]]; then
      ln -s "$src" "$safe_src"
    fi
    mkdir -p "$safe_build"
    src_for_build="$safe_src"
    build_for_make="$safe_build"
    install_for_make="$safe_install"
  fi

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
  local extra_cxxflags="-Wno-register -Wno-error=register -DDJVU_NO_PTHREAD_CANCEL"

  if [[ ! -x "$cc" ]]; then
    echo "Clang not found for ABI: $ANDROID_ABI (expected $cc)" >&2
    return 1
  fi

  if [[ -x "$src_for_build/configure" ]]; then
    (cd "$build_for_make" && \
      CC="$cc" CXX="$cxx" CXXFLAGS="${CXXFLAGS:-} ${extra_cxxflags}" \
      "$src_for_build/configure" --host="$host_tag" --prefix="$install_for_make")
    cmake --build "$build_for_make" --parallel || make -C "$build_for_make" -j"$(nproc || echo 4)"
    make -C "$build_for_make" install
    if [[ "$need_safe_paths" -eq 1 ]]; then
      mkdir -p "$install_dir"
      cp -a "$install_for_make"/. "$install_dir"/
    fi
    prefix_paths+=("$install_dir")
    return 0
  fi

  if [[ -x "$src_for_build/autogen.sh" ]]; then
    (cd "$src_for_build" && ./autogen.sh)
    (cd "$build_for_make" && \
      CC="$cc" CXX="$cxx" CXXFLAGS="${CXXFLAGS:-} ${extra_cxxflags}" \
      "$src_for_build/configure" --host="$host_tag" --prefix="$install_for_make")
    cmake --build "$build_for_make" --parallel || make -C "$build_for_make" -j"$(nproc || echo 4)"
    make -C "$build_for_make" install
    if [[ "$need_safe_paths" -eq 1 ]]; then
      mkdir -p "$install_dir"
      cp -a "$install_for_make"/. "$install_dir"/
    fi
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
build_sqlite
build_libarchive
if [[ "$BUILD_POPPLER" == "1" ]]; then
  build_poppler
else
  echo "Skipping Poppler (set BUILD_POPPLER=1 to build it)"
fi
build_djvulibre

echo "Android deps installed to: $INSTALL_ROOT"
