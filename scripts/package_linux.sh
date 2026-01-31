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
VERSION="${EREADER_VERSION:-}"

if [[ -z "$VERSION" ]]; then
  if git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    VERSION="$(git -C "$ROOT" describe --tags --always --dirty 2>/dev/null || true)"
  fi
fi
if [[ -z "$VERSION" ]]; then
  VERSION="dev"
fi

PACKAGE_DIR="$OUT_DIR/my-ereader-$VERSION"
CURRENT_LINK="$OUT_DIR/current"
SHARED_CONFIG_DIR="$OUT_DIR/config"

mkdir -p "$OUT_DIR"
mkdir -p "$SHARED_CONFIG_DIR"
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"

if [[ ! -f "$BIN_PATH" ]]; then
  echo "Binary not found: $BIN_PATH" >&2
  echo "Build the app first (cmake --build build)" >&2
  exit 1
fi

# Copy binary
cp "$BIN_PATH" "$PACKAGE_DIR/"

# Copy docs/metadata needed for runtime paths
if [[ -f "$ROOT/README.md" ]]; then
  cp "$ROOT/README.md" "$PACKAGE_DIR/"
fi

# Seed shared config defaults (do not overwrite existing user config)
if [[ -d "$ROOT/config" ]]; then
  for cfg in "$ROOT/config"/*.ini; do
    if [[ -f "$cfg" ]]; then
      name="$(basename "$cfg")"
      if [[ ! -f "$SHARED_CONFIG_DIR/$name" ]]; then
        cp "$cfg" "$SHARED_CONFIG_DIR/$name"
      fi
    fi
  done
fi
if [[ -d "$ROOT/icon" ]]; then
  cp -a "$ROOT/icon" "$PACKAGE_DIR/"
fi

# Provide a launcher that sets bundled paths
cat > "$PACKAGE_DIR/ereader.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$ROOT/lib:${LD_LIBRARY_PATH:-}"
export PATH="$ROOT/tools/djvulibre/bin:$PATH"
export MYEREADER_CONFIG_DIR="$ROOT/../config"
exec "$ROOT/ereader" "$@"
EOF
chmod +x "$PACKAGE_DIR/ereader.sh"

# Bundle licenses
"$ROOT/scripts/package_licenses.sh"
cp -r "$ROOT/licenses" "$PACKAGE_DIR/"

# Bundle Poppler shared libs (Qt6 + core)
if [[ -d "$POPPLER_LIB_DIR" ]]; then
  mkdir -p "$PACKAGE_DIR/lib"
  cp -a "$POPPLER_LIB_DIR"/libpoppler*.so* "$PACKAGE_DIR/lib/" 2>/dev/null || true
  cp -a "$POPPLER_LIB_DIR"/libpoppler-qt6*.so* "$PACKAGE_DIR/lib/" 2>/dev/null || true
else
  echo "Poppler install dir not found: $POPPLER_LIB_DIR" >&2
fi

# Bundle libarchive shared libs
if [[ -d "$LIBARCHIVE_LIB_DIR" ]]; then
  mkdir -p "$PACKAGE_DIR/lib"
  cp -a "$LIBARCHIVE_LIB_DIR"/libarchive*.so* "$PACKAGE_DIR/lib/" 2>/dev/null || true
else
  echo "libarchive install dir not found: $LIBARCHIVE_LIB_DIR" >&2
fi

# Bundle DjVuLibre tools
if [[ -d "$DJVULIBRE_BIN_DIR" ]]; then
  mkdir -p "$PACKAGE_DIR/tools/djvulibre/bin"
  cp -a "$DJVULIBRE_BIN_DIR"/ddjvu "$PACKAGE_DIR/tools/djvulibre/bin/" 2>/dev/null || true
  cp -a "$DJVULIBRE_BIN_DIR"/djvused "$PACKAGE_DIR/tools/djvulibre/bin/" 2>/dev/null || true
  cp -a "$DJVULIBRE_BIN_DIR"/djvutxt "$PACKAGE_DIR/tools/djvulibre/bin/" 2>/dev/null || true
else
  echo "DjVuLibre bin dir not found: $DJVULIBRE_BIN_DIR" >&2
fi

# Update current symlink for easy launches
if [[ -L "$CURRENT_LINK" || -e "$CURRENT_LINK" ]]; then
  rm -f "$CURRENT_LINK"
fi
ln -s "my-ereader-$VERSION" "$CURRENT_LINK"

cat <<MSG
Packaged to: $PACKAGE_DIR
- $PACKAGE_DIR/$APP_NAME
- $PACKAGE_DIR/ereader.sh (launcher)
- $PACKAGE_DIR/lib (Poppler/libarchive libs, if found)
- $PACKAGE_DIR/tools/djvulibre/bin (DjVuLibre tools, if found)
- $PACKAGE_DIR/licenses
- $OUT_DIR/config (shared config)
- $OUT_DIR/current -> $PACKAGE_DIR
MSG
