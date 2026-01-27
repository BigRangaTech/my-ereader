#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$ROOT/licenses"

mkdir -p "$OUT_DIR"

copy_if_exists() {
  local src="$1"
  local dst="$2"
  if [[ -f "$src" ]]; then
    cp "$src" "$dst"
  else
    echo "Missing: $src" >&2
  fi
}

copy_if_exists "$ROOT/LICENSE" "$OUT_DIR/PROJECT_LICENSE.txt"
copy_if_exists "$ROOT/third_party/monocypher/LICENCE.md" "$OUT_DIR/Monocypher.txt"
copy_if_exists "$ROOT/poppler/COPYING" "$OUT_DIR/Poppler_COPYING.txt"
copy_if_exists "$ROOT/poppler/COPYING3" "$OUT_DIR/Poppler_COPYING3.txt"

