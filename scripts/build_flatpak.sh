#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MANIFEST="$ROOT/flatpak/com.bigrangatech.MyEreader.yaml"
BUILD_DIR="$ROOT/flatpak/build"
REPO_DIR="$ROOT/flatpak/repo"

mkdir -p "$BUILD_DIR" "$REPO_DIR"

EXTRA_ARGS=()
if [[ "${DISABLE_ROFILES_FUSE:-}" == "1" ]]; then
  EXTRA_ARGS+=(--disable-rofiles-fuse)
elif [[ ! -e /dev/fuse ]] || ! command -v fusermount3 >/dev/null 2>&1; then
  EXTRA_ARGS+=(--disable-rofiles-fuse)
fi

flatpak-builder --force-clean --install-deps-from=flathub \
  "${EXTRA_ARGS[@]}" \
  --repo="$REPO_DIR" \
  "$BUILD_DIR" \
  "$MANIFEST"

echo "Flatpak build complete. Repo: $REPO_DIR"
