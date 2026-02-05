# AGENTS.md

## Repo overview
My Ereader is a Qt 6 ebook reader with local-first library management, LAN-only sync, and per-format settings.
Active work centers on UI, sync, security/vault, and Flatpak packaging.

## Key directories
- `src/app/qml/Main.qml` — main UI; settings dialog, sync UI, keyboard shortcuts.
- `src/core` — vault, database, keychain, logging, app paths.
- `src/sync` — LAN discovery, pairing, sync transport.
- `config/` — per-format and app settings (`sync.ini`, `settings.ini`, etc.).
- `flatpak/` — Flatpak manifest and packaging.
- `scripts/` — build/publish scripts.
- `docs/` — architecture, security, roadmap, updates, settings.

## Recent changes (high signal)
- **Sync stability fixes:** TCP sync responses now buffered on client; server closes socket only after bytes are written. Added extensive sync logs. Transfer limit default set to 5120 MB (5 GB).
- **Pairing fixes:** Client uses peer IP, server logs pin match; added socket error logging.
- **Keychain:** libsecret-backed keychain added with portal fallback; Flatpak uses libsecret module.
- **Vault/DB:** in-memory DB serialize/deserialize uses VACUUM/ATTACH; avoids missing sqlite handle.
- **Logging:** app logs go to repo `logs/` if running from repo, otherwise to app data logs (Flatpak uses `~/.var/app/.../data/ereader/logs/app.log`).
- **Keyboard shortcuts:** added global key handler in `Main.qml`.

## Current task in progress
Refactor Settings UI: convert long settings list into shorter settings panel with buttons to open:
1) **Keyboard Shortcuts** dialog
2) **Format Settings** dialog (contains all per-format settings)

Work pending:
- Move the large per-format settings block out of `settingsDialog` and into a new `formatSettingsDialog`.
- Insert `keyboardDialog` and `formatSettingsDialog` into `Main.qml`.

## Known issues to keep in mind
- Some QML warnings about undefined search value may appear; not critical but should be cleaned.
- `QSqlDatabasePrivate::removeDatabase` warnings can occur when closing DB connections; not fatal.

## Build/run notes
- Run binary (repo build):  
  `LD_LIBRARY_PATH=third_party/install/poppler/lib:third_party/install/libarchive/lib:$LD_LIBRARY_PATH ./build/src/app/ereader`
- Flatpak logs (if built):  
  `~/.var/app/com.bigrangatech.MyEreader/data/ereader/logs/app.log`

## User preferences
- Prefers Qt stack, Linux + Android.
- Flatpak is the packaging target; wants self-contained.
- Uses local network only; no cloud sync.
- Wants clean, concise settings UI and strong UX.

