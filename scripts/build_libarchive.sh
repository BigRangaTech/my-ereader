#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/third_party/build/libarchive"
INSTALL_DIR="$ROOT/third_party/install/libarchive"

mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

cmake -S "$ROOT/third_party/libarchive" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
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

cmake --build "$BUILD_DIR" --parallel
cmake --install "$BUILD_DIR"
