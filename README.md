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
