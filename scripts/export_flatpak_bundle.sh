#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO_DIR="$ROOT/flatpak/repo"
OUT_DIR="$ROOT/dist"
APP_ID="com.bigrangatech.MyEreader"
BRANCH="${FLATPAK_BRANCH:-master}"
RUNTIME_ID="${FLATPAK_RUNTIME_ID:-org.kde.Platform}"
RUNTIME_BRANCH="${FLATPAK_RUNTIME_BRANCH:-6.7}"

if [[ ! -d "$REPO_DIR" ]]; then
  echo "Flatpak repo not found: $REPO_DIR" >&2
  echo "Run ./scripts/build_flatpak.sh first." >&2
  exit 1
fi

mkdir -p "$OUT_DIR"

VERSION="${EREADER_VERSION:-}"
if [[ -z "$VERSION" ]] && git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  VERSION="$(git -C "$ROOT" describe --tags --always --dirty 2>/dev/null || true)"
fi
if [[ -z "$VERSION" ]]; then
  VERSION="dev"
fi
VERSION_SAFE="${VERSION//\//-}"

APP_BUNDLE="$OUT_DIR/my-ereader-${VERSION_SAFE}.flatpak"

if command -v flatpak >/dev/null 2>&1; then
  flatpak build-bundle "$REPO_DIR" "$APP_BUNDLE" "$APP_ID" "$BRANCH"
else
  echo "flatpak not found. Install flatpak to export bundles." >&2
  exit 1
fi

echo "App bundle created: $APP_BUNDLE"

echo ""
echo "Optional offline repo (includes runtime + app)"
if flatpak --help 2>/dev/null | rg -q "create-usb"; then
  USB_DIR="$OUT_DIR/flatpak-offline"
  rm -rf "$USB_DIR"
  mkdir -p "$USB_DIR"
  flatpak create-usb "$USB_DIR" "$APP_ID" "$RUNTIME_ID"//"$RUNTIME_BRANCH" || true
  echo "Offline Flatpak repo created at: $USB_DIR"
else
  echo "flatpak create-usb not available on this system." >&2
  echo "You can still install the app bundle and the runtime via flatpak." >&2
fi
