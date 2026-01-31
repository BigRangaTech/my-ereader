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
- FB2: implemented (text + metadata + cover + inline images)
- CBZ: implemented (image extraction)
- CBR: implemented if bsdtar/unrar/unar is available
- MOBI/AZW3
- DJVU: implemented (page rendering via djvulibre tools)

## Updates
- Desktop: git pull updates (fast-forward only)
- Android: update path to be defined later

For packaged binaries, run the update script and launch via `dist/current/ereader.sh`
so you always use the latest packaged build from the repo.

## Non-goals (for now)
- Cloud sync or account systems
- DRM breaking or DRM bypass

## Quick start (desktop)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Packaging (Linux)
```bash
./scripts/package_linux.sh
```

This creates a versioned folder under `dist/` and a `dist/current` symlink.
Launch the packaged app with:
```bash
./dist/current/ereader.sh
```

Packaged builds use a shared config folder at `dist/config` so settings
persist across updates.

## Flatpak (recommended for releases)
```bash
./scripts/build_flatpak.sh
```

This produces a local Flatpak repo under `flatpak/repo`.
The Flatpak manifest vendors Poppler, libarchive, DjVuLibre, and the
speech-dispatcher + espeak-ng TTS stack so the app is self-contained.

To export an offline bundle:
```bash
./scripts/export_flatpak_bundle.sh
```

### DjVuLibre (optional vendor)
DJVU rendering uses DjVuLibre CLI tools (`ddjvu`, `djvused`, `djvutxt`). You can install them
system-wide or vendor them under `third_party/install/djvulibre/bin` using:

```bash
scripts/build_djvulibre.sh
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
  - `config/mobi.ini`
  - `config/azw*.ini` / `config/prc.ini`
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
