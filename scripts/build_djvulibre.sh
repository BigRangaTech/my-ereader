#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC_DIR="$ROOT/third_party/djvulibre"
BUILD_DIR="$ROOT/third_party/build/djvulibre"
INSTALL_DIR="$ROOT/third_party/install/djvulibre"

if [[ ! -d "$SRC_DIR" ]]; then
  echo "DjVuLibre source not found at: $SRC_DIR" >&2
  echo "Clone it into third_party/djvulibre (or adjust the path)" >&2
  exit 1
fi

mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

if [[ -x "$SRC_DIR/configure" ]]; then
  (cd "$BUILD_DIR" && "$SRC_DIR/configure" --prefix="$INSTALL_DIR")
  cmake --build "$BUILD_DIR" --parallel || make -C "$BUILD_DIR" -j"$(nproc || echo 4)"
  make -C "$BUILD_DIR" install
elif [[ -x "$SRC_DIR/autogen.sh" ]]; then
  (cd "$SRC_DIR" && ./autogen.sh)
  (cd "$BUILD_DIR" && "$SRC_DIR/configure" --prefix="$INSTALL_DIR")
  cmake --build "$BUILD_DIR" --parallel || make -C "$BUILD_DIR" -j"$(nproc || echo 4)"
  make -C "$BUILD_DIR" install
else
  echo "DjVuLibre configure script not found. Ensure the source tree is complete." >&2
  exit 1
fi

cat <<MSG
DjVuLibre installed to: $INSTALL_DIR
- bin: $INSTALL_DIR/bin
MSG
