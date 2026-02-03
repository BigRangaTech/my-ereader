# Settings

Settings live under `config/` at the repo root.

- `config/settings.ini` stores general settings and per-book UI state.
- Each format has its own file, e.g. `config/pdf.ini`, `config/epub.ini`, etc.

## General (`config/settings.ini`)
- `reading/font_size` (default: 20)
- `reading/line_height` (default: 1.4)
- `reader/sidebar/<sha1>` (default: `toc`) — remembers TOC vs annotations per book
- `tts/rate` (default: 0.0) — range `-1.0` to `1.0`
- `tts/pitch` (default: 0.0) — range `-1.0` to `1.0`
- `tts/volume` (default: 1.0) — range `0.0` to `1.0`
- `tts/voice_key` (default: empty) — `VoiceName|locale` when set
- `security/auto_lock_enabled` (default: true)
- `security/auto_lock_minutes` (default: 10) — range `1` to `240`
- `security/remember_passphrase` (default: true) — keep passphrase in memory for this session

## Sync (`config/sync.ini`)
- `sync/enabled` (default: false)
- `device/name` (default: hostname)
- `device/id` (default: generated UUID)
- `device/pin` (default: random 4-digit PIN)
- `network/discovery_port` (default: 45454)
- `network/listen_port` (default: 45455)

## EPUB (`config/epub.ini`)
- `reading/font_size` (default: 20)
- `reading/line_height` (default: 1.4)
- `render/show_images` (default: true)
- `render/text_align` (default: `left`) — `left|right|center|justify`
- `render/paragraph_spacing_em` (default: 0.6)
- `render/paragraph_indent_em` (default: 0.0)
- `render/image_max_width_percent` (default: 100)
- `render/image_spacing_em` (default: 0.6)

## FB2 (`config/fb2.ini`)
- `reading/font_size` (default: 20)
- `reading/line_height` (default: 1.4)
- `render/show_images` (default: true)
- `render/text_align` (default: `left`) — `left|right|center|justify`
- `render/paragraph_spacing_em` (default: 0.6)
- `render/paragraph_indent_em` (default: 0.0)
- `render/image_max_width_percent` (default: 100)
- `render/image_spacing_em` (default: 0.6)

## TXT (`config/txt.ini`)
- `reading/font_size` (default: 20)
- `reading/line_height` (default: 1.4)
- `render/encoding` (default: `auto`) — `auto|utf-8|utf-16le|utf-16be|utf-32le|utf-32be|latin1`
- `render/normalize_line_endings` (default: true)
- `render/trim_trailing_whitespace` (default: false)
- `render/tab_width` (default: 4)
- `render/max_blank_lines` (default: 0) — 0 disables limiting
- `render/split_on_formfeed` (default: true)
- `render/auto_chapters` (default: true) — heading-based chapter detection
- `render/monospace` (default: false)

## MOBI/AZW/PRC
- `config/mobi.ini`, `config/azw.ini`, `config/azw3.ini`, `config/azw4.ini`, `config/prc.ini`
- `reading/font_size` (default: 20)
- `reading/line_height` (default: 1.4)
- `render/show_images` (default: true)
- `render/text_align` (default: `left`) — `left|right|center|justify`
- `render/paragraph_spacing_em` (default: 0.6)
- `render/paragraph_indent_em` (default: 0.0)
- `render/image_max_width_percent` (default: 100)
- `render/image_spacing_em` (default: 0.6)

## Comics (`config/cbz.ini`, `config/cbr.ini`)
- `zoom/min` (default: 0.5)
- `zoom/max` (default: 4.0)
- `render/sort_mode` (default: `path`) — `path|filename|archive`
- `render/sort_desc` (default: false)

## PDF (`config/pdf.ini`)
- `render/preset` (default: `custom`) — `custom|fast|balanced|high`
- `render/dpi` (default: 120)
- `render/cache_limit` (default: 30)
- `render/cache_policy` (default: `fifo`) — `fifo|lru`
- `render/prefetch_distance` (default: 1)
- `render/prefetch_strategy` (default: `symmetric`) — `forward|symmetric|backward`
- `render/progressive` (default: false)
- `render/progressive_dpi` (default: 72)
- `render/color_mode` (default: `color`) — `color|grayscale`
- `render/background_mode` (default: `white`) — `white|transparent|theme|custom`
- `render/background_color` (default: `#202633`)
- `render/max_width` (default: 0) — 0 disables cap
- `render/max_height` (default: 0) — 0 disables cap
- `render/image_format` (default: `png`) — `png|jpeg`
- `render/jpeg_quality` (default: 85)
- `render/extract_text` (default: true)
- `render/tile_size` (default: 0) — 0 disables tiling

## DJVU
- `config/djvu.ini`
- `render/dpi` (default: 120)
- `render/cache_limit` (default: 30)
- `render/prefetch_distance` (default: 1)
- `render/cache_policy` (default: `fifo`) — `fifo|lru`
- `render/format` (default: `ppm`) — `ppm|tiff`
- `render/extract_text` (default: true)
- `render/rotation` (default: 0) — `0|90|180|270`
