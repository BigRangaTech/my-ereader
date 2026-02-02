#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="/home/jessie/Documents/my ereader"
REPO_DIR="/home/jessie/Documents/my-ereader-flatpak"
BUILD_DIR="$ROOT_DIR/flatpak/build"
MANIFEST="$ROOT_DIR/flatpak/com.bigrangatech.MyEreader.yaml"
INDEX_SRC="$ROOT_DIR/index.html"
GPG_KEY_ID="FF1EC6D9FC8202B9"

if [ ! -f "$MANIFEST" ]; then
  echo "Missing manifest: $MANIFEST" >&2
  exit 1
fi

if [ ! -d "$REPO_DIR/.git" ]; then
  echo "Missing flatpak repo git dir: $REPO_DIR" >&2
  exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
  echo "Missing build dir: $BUILD_DIR" >&2
  exit 1
fi

if [ ! -f "$INDEX_SRC" ]; then
  echo "Missing index.html at $INDEX_SRC" >&2
  exit 1
fi

cd "$ROOT_DIR"

flatpak-builder --disable-rofiles-fuse --force-clean \
  --repo="$REPO_DIR" \
  "$BUILD_DIR" \
  "$MANIFEST"

flatpak build-export --update-appstream --gpg-sign="$GPG_KEY_ID" \
  "$REPO_DIR" \
  "$BUILD_DIR"

# Ensure repo + appstream refs are signed (fixes appstream GPG warnings on clients).
flatpak build-update-repo --gpg-sign="$GPG_KEY_ID" "$REPO_DIR"

# Verify that appstream refs are signed.
if ! ostree show --repo="$REPO_DIR" appstream/x86_64 >/tmp/ereader-appstream.txt 2>/dev/null; then
  echo "Failed to locate appstream/x86_64 in repo" >&2
  exit 1
fi
if ! rg -q "Signed:.*yes" /tmp/ereader-appstream.txt; then
  echo "Appstream ref is not signed. Check GPG setup." >&2
  exit 1
fi

flatpak build-bundle \
  "$REPO_DIR" \
  "$ROOT_DIR/dist/my-ereader-$(date +%Y%m%d%H%M).flatpak" \
  com.bigrangatech.MyEreader \
  --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

cd "$REPO_DIR"

git fetch origin gh-pages || true

git checkout gh-pages

cp "$INDEX_SRC" ./index.html
gpg --export --armor "$GPG_KEY_ID" > "$REPO_DIR/repo-gpg.pub"

git add -A

git commit -m "Update flatpak repo" || echo "No changes to commit"

printf "\nDone. Flatpak repo and bundle updated. Push gh-pages manually when ready.\n"
