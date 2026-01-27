#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/third_party/build/poppler"
INSTALL_DIR="$ROOT/third_party/install/poppler"

mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

cmake -S "$ROOT/poppler" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DENABLE_UNSTABLE_API_ABI_HEADERS=OFF \
  -DENABLE_CPP=OFF \
  -DENABLE_GLIB=OFF \
  -DENABLE_GTK_DOC=OFF \
  -DENABLE_QT5=OFF \
  -DENABLE_QT6=ON \
  -DBUILD_SHARED_LIBS=ON

cmake --build "$BUILD_DIR" --parallel
cmake --install "$BUILD_DIR"

