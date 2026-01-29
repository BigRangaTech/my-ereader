# My Ereader

A fast, secure, offline-first ebook reader for Linux and Android built with Qt 6.

## Goals
- Fast startup and page turns
- Best-in-class UX/UI for long-form reading
- Local-first library management
- LAN-only sync between devices
- Text-to-speech (TTS)
- Strong security posture (encrypted library + minimal network surface)
- Broad format support via extensible format providers

## Format priority
1) EPUB
2) PDF
3) MOBI/AZW3
4) CBZ/CBR
5) FB2
6) DJVU

## Format status (initial)
- EPUB: implemented (text + TOC + inline images)
- PDF: implemented (rendering + caching + advanced settings)
- FB2: implemented (text + metadata + cover)
- CBZ: implemented (image extraction)
- CBR: implemented if bsdtar/unrar/unar is available
- MOBI/AZW3: experimental (known rendering issues; pending libmobi updates)
- DJVU: implemented (page rendering via djvulibre tools)

## Updates
- Desktop: git pull updates (fast-forward only)
- Android: update path to be defined later

## Non-goals (for now)
- Cloud sync or account systems
- DRM breaking or DRM bypass

## Quick start (desktop)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Settings
Settings are stored under `config/` at the repo root.

- `config/settings.ini` holds general app settings (and legacy fallbacks).
- Per-format settings live in their own files:
  - `config/epub.ini`
  - `config/pdf.ini`
  - `config/cbz.ini`
  - `config/cbr.ini`
  - `config/fb2.ini`
  - `config/txt.ini`
  - `config/mobi.ini` (experimental)
  - `config/azw*.ini` / `config/prc.ini` (experimental)
  - `config/djvu.ini`

## Docs
- docs/ARCHITECTURE.md
- docs/SECURITY.md
- docs/ROADMAP.md
- docs/UPDATES.md
- docs/CRYPTO.md
- docs/SETTINGS.md
- docs/THIRD_PARTY.md
- third_party/README.md
- docs/POPPLER.md
- docs/RELEASE.md
- scripts/package_linux.sh
- scripts/build_libarchive.sh

## License
GPL-2.0-or-later
