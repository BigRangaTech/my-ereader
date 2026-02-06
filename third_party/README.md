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
- Upstream: https://github.com/BigRangaTech/Monocypher (tag `v002`)
  - License: `third_party/monocypher/LICENCE.md`
  - Notes: custom crypto backend
- Poppler
  - Source: `poppler/`
  - Upstream: https://github.com/BigRangaTech/my-poppler (tag `v001`)
  - License: see `poppler/` tree
  - Notes: built and bundled in prebuilt binaries
  - Build: `scripts/build_poppler.sh`
- Monocypher snapshot (legacy)
  - Source: `monocypher-4.0.2/`
  - Notes: older snapshot kept during migration; remove when consolidated
- miniz
  - Source: `third_party/miniz/`
  - Upstream: https://github.com/richgel999/miniz
  - License: `third_party/miniz/LICENSE.txt`
  - Notes: ZIP reader used for EPUB parsing
- zlib
  - Source: `third_party/zlib/`
  - Upstream: https://github.com/madler/zlib (tag `v1.3.1`)
  - License: `third_party/zlib/LICENSE`
  - Notes: compression backend for format parsing
- libxml2
  - Source: `third_party/libxml2/`
  - Upstream: https://github.com/BigRangaTech/libxml2 (tag `v001`)
  - License: `third_party/libxml2/Copyright`
  - Notes: XML parser for FB2/metadata
- libarchive
  - Source: `third_party/libarchive/`
  - Upstream: https://github.com/BigRangaTech/libarchive (tag `v001`)
  - License: `third_party/libarchive/COPYING`
  - Notes: CBR extraction (bundled)
  - Build: `scripts/build_libarchive.sh`
- libmobi
  - Source: `third_party/libmobi/`
  - Upstream: https://github.com/BigRangaTech/libmobi (tag `b01.1`)
  - License: `third_party/libmobi/COPYING`
  - Notes: MOBI/AZW3 parsing
