# Poppler (Bundled build)

We build Poppler as shared libraries and bundle them alongside the app in
prebuilt binaries.

## Build (Linux)
```bash
./scripts/build_poppler.sh
```

## Install output
- `third_party/install/poppler/`

## Notes
- Qt6 bindings enabled
- Qt5, GLib, and docs disabled for a minimal bundle
