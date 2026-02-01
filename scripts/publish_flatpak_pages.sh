#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO_DIR="${REPO_DIR:-$ROOT/flatpak/repo}"
PAGES_DIR="${PAGES_DIR:-}"
BRANCH="${PAGES_BRANCH:-gh-pages}"

if [[ ! -d "$REPO_DIR" ]]; then
  echo "Flatpak repo not found: $REPO_DIR" >&2
  echo "Run ./scripts/build_flatpak.sh first." >&2
  exit 1
fi

if [[ -z "$PAGES_DIR" ]]; then
  echo "Set PAGES_DIR to your GitHub Pages checkout." >&2
  echo "Example: PAGES_DIR=../my-ereader-flatpak ./scripts/publish_flatpak_pages.sh" >&2
  exit 1
fi

if [[ ! -d "$PAGES_DIR/.git" ]]; then
  echo "PAGES_DIR is not a git repo: $PAGES_DIR" >&2
  exit 1
fi

cd "$PAGES_DIR"

if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
  git checkout "$BRANCH"
else
  git checkout -b "$BRANCH"
fi

echo "Syncing Flatpak repo into $PAGES_DIR..."
rsync -a --delete "$REPO_DIR"/ "$PAGES_DIR"/

if git status --porcelain | rg -q .; then
  git add .
  git commit -m "Update Flatpak repo"
  git push -u origin "$BRANCH"
  echo "Published. Enable GitHub Pages for branch '$BRANCH' (root)."
else
  echo "No changes to publish."
fi
