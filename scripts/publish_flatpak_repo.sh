#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="/home/jessie/Documents/my ereader"
REPO_DIR="$ROOT_DIR/flatpak/repo"
BUILD_DIR="$ROOT_DIR/flatpak/build"
MANIFEST="$ROOT_DIR/flatpak/com.bigrangatech.MyEreader.yaml"
INDEX_SRC="$ROOT_DIR/index.html"
GPG_KEY_ID="FF1EC6D9FC8202B9"

if [ ! -f "$MANIFEST" ]; then
  echo "Missing manifest: $MANIFEST" >&2
  exit 1
fi

mkdir -p "$REPO_DIR"

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

# Explicitly sign appstream refs when present (some clients require it).
if ostree show --repo="$REPO_DIR" appstream/x86_64 >/dev/null 2>&1; then
  ostree gpg-sign --repo="$REPO_DIR" appstream/x86_64 "$GPG_KEY_ID"
fi
if ostree show --repo="$REPO_DIR" appstream2/x86_64 >/dev/null 2>&1; then
  ostree gpg-sign --repo="$REPO_DIR" appstream2/x86_64 "$GPG_KEY_ID"
fi

flatpak build-bundle \
  "$REPO_DIR" \
  "$ROOT_DIR/dist/my-ereader-$(date +%Y%m%d%H%M).flatpak" \
  com.bigrangatech.MyEreader \
  --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

cp "$INDEX_SRC" "$REPO_DIR/index.html"
gpg --export --armor "$GPG_KEY_ID" > "$REPO_DIR/repo-gpg.pub"
GPG_KEY_B64=$(gpg --export "$GPG_KEY_ID" | base64 -w 0)
cat > "$REPO_DIR/my-ereader.flatpakrepo" <<EOF
[Flatpak Repo]
Title=My Ereader
Url=https://bigrangatech.github.io/my-ereader-flatpak/
GPGKey=${GPG_KEY_B64}
EOF

printf "\nDone. Flatpak repo updated at %s and bundle created in dist/. Copy repo manually as needed.\n" "$REPO_DIR"
