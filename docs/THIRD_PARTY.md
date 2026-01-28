# Third-party

## Policy
- Vendored dependencies are pinned and recorded in `deps.lock`
- License files are stored alongside the source
- Prebuilt binaries bundle required third-party shared libraries

## Monocypher
- Purpose: crypto primitives for custom vault encryption
- License: see `third_party/monocypher/LICENCE.md`
- Source: `third_party/monocypher/monocypher.c` and `third_party/monocypher/monocypher.h`

## Poppler
- Purpose: PDF rendering
- License: see `poppler/` tree
- Source: `poppler/`
- Build: `scripts/build_poppler.sh`

## miniz
- Purpose: ZIP reader for EPUB parsing
- License: `third_party/miniz/LICENSE.txt`
- Source: `third_party/miniz/`

## libarchive
- Purpose: CBR extraction (RAR archives)
- License: `third_party/libarchive/COPYING`
- Source: `third_party/libarchive/`
- Build: `scripts/build_libarchive.sh`
