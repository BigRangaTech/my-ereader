#!/usr/bin/env bash
set -euo pipefail

# Placeholder updater for vendored dependencies.
# Replace URLs and tags with your fork locations.

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

update_monocypher() {
  echo "Monocypher: update script not configured yet."
  echo "- Set MONOCYPHER_REMOTE and MONOCYPHER_REF in this script."
}

update_poppler() {
  echo "Poppler: update script not configured yet."
  echo "- Set POPPLER_REMOTE and POPPLER_REF in this script."
}

update_monocypher
update_poppler

