#!/usr/bin/env bash
set -euo pipefail

# Placeholder updater for vendored dependencies.
# Replace URLs and tags with your fork locations.

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

MONOCYPHER_REMOTE="https://github.com/BigRangaTech/Monocypher"
MONOCYPHER_REF="v001"
POPPLER_REMOTE="https://github.com/BigRangaTech/my-poppler"
POPPLER_REF="001"

update_monocypher() {
  echo "Updating Monocypher to $MONOCYPHER_REF"
  rm -rf "$ROOT/third_party/monocypher"
  git clone --depth 1 --branch "$MONOCYPHER_REF" "$MONOCYPHER_REMOTE" "$ROOT/third_party/monocypher"
}

update_poppler() {
  echo "Updating Poppler to $POPPLER_REF"
  rm -rf "$ROOT/poppler"
  git clone --depth 1 --branch "$POPPLER_REF" "$POPPLER_REMOTE" "$ROOT/poppler"
}

update_monocypher
update_poppler
