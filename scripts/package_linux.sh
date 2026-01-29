#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"
OUT_DIR="${2:-$ROOT/dist}"
APP_NAME="ereader"

BIN_PATH="$BUILD_DIR/src/app/$APP_NAME"
POPPLER_LIB_DIR="$ROOT/third_party/install/poppler/lib"
LIBARCHIVE_LIB_DIR="$ROOT/third_party/install/libarchive/lib"
DJVULIBRE_BIN_DIR="$ROOT/third_party/install/djvulibre/bin"

mkdir -p "$OUT_DIR"

if [[ ! -f "$BIN_PATH" ]]; then
  echo "Binary not found: $BIN_PATH" >&2
  echo "Build the app first (cmake --build build)" >&2
  exit 1
fi

# Copy binary
cp "$BIN_PATH" "$OUT_DIR/"

# Bundle licenses
"$ROOT/scripts/package_licenses.sh"
cp -r "$ROOT/licenses" "$OUT_DIR/"

# Bundle Poppler shared libs (Qt6 + core)
if [[ -d "$POPPLER_LIB_DIR" ]]; then
  mkdir -p "$OUT_DIR/lib"
  cp -a "$POPPLER_LIB_DIR"/libpoppler*.so* "$OUT_DIR/lib/" 2>/dev/null || true
  cp -a "$POPPLER_LIB_DIR"/libpoppler-qt6*.so* "$OUT_DIR/lib/" 2>/dev/null || true
else
  echo "Poppler install dir not found: $POPPLER_LIB_DIR" >&2
fi

# Bundle libarchive shared libs
if [[ -d "$LIBARCHIVE_LIB_DIR" ]]; then
  mkdir -p "$OUT_DIR/lib"
  cp -a "$LIBARCHIVE_LIB_DIR"/libarchive*.so* "$OUT_DIR/lib/" 2>/dev/null || true
else
  echo "libarchive install dir not found: $LIBARCHIVE_LIB_DIR" >&2
fi

# Bundle DjVuLibre tools
if [[ -d "$DJVULIBRE_BIN_DIR" ]]; then
  mkdir -p "$OUT_DIR/tools/djvulibre/bin"
  cp -a "$DJVULIBRE_BIN_DIR"/ddjvu "$OUT_DIR/tools/djvulibre/bin/" 2>/dev/null || true
  cp -a "$DJVULIBRE_BIN_DIR"/djvused "$OUT_DIR/tools/djvulibre/bin/" 2>/dev/null || true
  cp -a "$DJVULIBRE_BIN_DIR"/djvutxt "$OUT_DIR/tools/djvulibre/bin/" 2>/dev/null || true
else
  echo "DjVuLibre bin dir not found: $DJVULIBRE_BIN_DIR" >&2
fi

cat <<MSG
Packaged to: $OUT_DIR
- $OUT_DIR/$APP_NAME
- $OUT_DIR/lib (Poppler libs, if found)
- $OUT_DIR/tools/djvulibre/bin (DjVuLibre tools, if found)
- $OUT_DIR/licenses
MSG
