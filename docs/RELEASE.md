# Release checklist (Linux)

## Licenses
```bash
./scripts/package_licenses.sh
```

Ensure `licenses/` is included alongside the app binary in the release.

## Poppler
```bash
./scripts/build_poppler.sh
```

Copy required Poppler shared libraries next to the app binary.

## libarchive
```bash
./scripts/build_libarchive.sh
```

Copy required libarchive shared libraries next to the app binary.

## Notes
- Update `deps.lock` with final tags/commits
- Verify license bundle in About dialog
- Package for release:
```bash
./scripts/package_linux.sh
```

The package script creates a versioned folder under `dist/` plus a
`dist/current` symlink and `ereader.sh` launcher.

## Flatpak
```bash
./scripts/build_flatpak.sh
```

This builds a Flatpak repo under `flatpak/repo` and vendors Poppler,
libarchive, DjVuLibre, plus bundled TTS backends (speech-dispatcher + espeak-ng)
as modules.

## Flatpak offline bundles
```bash
./scripts/export_flatpak_bundle.sh
```

This exports an app bundle into `dist/` and (when supported) creates an
offline Flatpak repo under `dist/flatpak-offline` that includes the runtime.
