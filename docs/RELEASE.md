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
