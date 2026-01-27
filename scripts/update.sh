#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$repo_root"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "Not a git repository: $repo_root" >&2
  exit 1
fi

if [[ -n "$(git status --porcelain)" ]]; then
  echo "Working tree is not clean. Commit or stash changes before updating." >&2
  exit 1
fi

git fetch --tags

git pull --ff-only
