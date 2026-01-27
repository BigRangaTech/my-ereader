# Third-party sources

This directory (and sibling vendor directories) contain vendored third-party code
that is built and bundled into prebuilt binaries.

## Policy
- Pin every dependency to a known upstream commit/tag
- Keep a local snapshot in the repo
- Maintain license files alongside the source
- Record provenance in `deps.lock`
- Prefer bundled shared libraries for Poppler on Linux

## Current vendors
- Monocypher
  - Source: `third_party/monocypher/`
  - License: `third_party/monocypher/LICENCE.md`
  - Notes: custom crypto backend
- Poppler
  - Source: `poppler/`
  - License: see `poppler/` tree
  - Notes: built and bundled in prebuilt binaries
- Monocypher snapshot (legacy)
  - Source: `monocypher-4.0.2/`
  - Notes: older snapshot kept during migration; remove when consolidated
