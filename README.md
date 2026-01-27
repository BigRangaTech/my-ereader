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

## Docs
- docs/ARCHITECTURE.md
- docs/SECURITY.md
- docs/ROADMAP.md
- docs/UPDATES.md
- docs/CRYPTO.md
- docs/THIRD_PARTY.md
- third_party/README.md

## License
GPL-2.0-or-later
