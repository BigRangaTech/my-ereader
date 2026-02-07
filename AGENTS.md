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
- **Android packaging:** added `scripts/android/patch_deployment_json.py` and a build-time patch in `src/app/CMakeLists.txt` to fix `qml-import-paths` for `androiddeployqt`.
- **Android content URIs:** `ReaderController` now resolves `content://` URIs by copying to cache before opening.
- **Android backends bundled:** Poppler Qt6 and djvulibre are bundled into APK; djvulibre tools (ddjvu/djvused/djvutxt) are embedded as resources and extracted at runtime.
- **Android UI safe area:** `MainAndroid.qml` derives insets from `Screen.geometry` vs `Screen.availableGeometry`, with Android-specific top/bottom fallbacks.
- **Android add-file dialog:** on Android, `FileDialog` uses `All files (*)` to avoid SAF filtering that hides CBZ/CBR.
- **Android reader layout:** reader bottom controls are in a horizontal Flickable to prevent overflow; image reader controls are also flickable, with some options hidden on Android to keep controls on-screen.
- **Android swipe/pinch gestures:** text swipe uses a single-touch `DragHandler`; image swipe uses a single-touch `DragHandler` and is disabled while pinching/zoomed. Image pinch now uses `PinchHandler` (tracks pinch state + zoom).
- **Android passphrase dialogs:** setup/unlock/lock dialogs use ScrollView with smaller Android margins/heights to prevent off-screen fields.
- **Android settings layout:** settings page now anchors the content column and adds safe-area padding (top/bottom) to avoid squished layout on small screens.
- **Android reader top bar:** top bar controls are now in a horizontal Flickable on Android to prevent buttons from going off-screen.
- **Android reader pinch:** image flick disables interaction while pinching or when zoomed for panning; image swipe handler can take over from other handlers for single-finger page swipes.
- **Android settings panels:** settings section cards now derive implicit height from content to avoid overlapping/squished layout.

## Current task in progress
Refactor Settings UI: convert long settings list into shorter settings panel with buttons to open:
1) **Keyboard Shortcuts** dialog
2) **Format Settings** dialog (contains all per-format settings)

Work pending:
- Move the large per-format settings block out of `settingsDialog` and into a new `formatSettingsDialog`.
- Insert `keyboardDialog` and `formatSettingsDialog` into `Main.qml`.

## Android UI notes
- If the system file picker greys out CBZ/CBR, Android SAF may be filtering by MIME type. Current workaround is `All files (*)` on Android.
- Safe-area handling in `MainAndroid.qml` uses `Screen.geometry`/`Screen.availableGeometry` with a small Android fallback inset; content container no longer subtracts footer height (footer already reserves space).

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
