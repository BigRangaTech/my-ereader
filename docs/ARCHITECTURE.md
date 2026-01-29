# Architecture

## High-level components
- App (QML + C++)
  - UI shell and navigation
  - View models exposed to QML
- Core (C++)
  - Library management (SQLite)
  - Reading state + progress
  - Annotation storage
  - Preferences (general + per-format INI files under `config/`)
- Formats (C++)
  - FormatProvider interface
  - FormatRegistry
  - Individual providers (priority: EPUB, PDF, MOBI/AZW3, CBZ/CBR, FB2, DJVU)
- Sync (C++)
  - LAN discovery
  - Pairing + trust store
  - Delta sync for library/annotations
- TTS (C++)
  - Wrapper around Qt TextToSpeech

## Data model (draft)
- LibraryItem
  - id, title, authors, series, tags, cover
  - file path + hash + format
- ReadingState
  - libraryItemId
  - progress (cfi/page/percent)
  - lastPosition
- Annotation
  - libraryItemId
  - location (format-specific locator)
  - type (highlight, note)
  - text, color, createdAt

## Formats pipeline
1) FormatProvider opens a file and returns a FormatDocument
2) FormatDocument exposes metadata + navigation + content slices
3) ReaderController renders content in QML
4) Annotations are stored in a format-agnostic schema with a locator

## Updates
- Desktop updates use git pull (fast-forward only)
- Update verification uses signed release tags
- Android update flow to be defined later

## Sync (LAN only)
- Discovery via UDP multicast (mDNS-compatible) with a short device advert
- Pairing via QR code or PIN (out-of-band trust)
- Transport via TLS with device certificates
- Sync engine uses change logs per device for conflict-free merges

## Security posture
- Library database and annotations encrypted at rest
- Custom vault format for encrypted data
- Minimally exposed network surface (LAN only, opt-in)
- Strong sandboxing and least-privilege access
- Updates only from the Git repo (signed releases)
