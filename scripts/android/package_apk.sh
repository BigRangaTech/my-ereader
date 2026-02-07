#!/usr/bin/env bash
set -euo pipefail

build_dir=${1:-build-android-arm64}
app_dir="$build_dir/src/app"
json="$app_dir/android-ereader-deployment-settings.json"

if [[ ! -f "$json" ]]; then
  echo "Deployment settings not found: $json" >&2
  exit 1
fi

python3 - "$json" <<'PY'
import json, sys
json_path = sys.argv[1]
with open(json_path) as f:
    data = json.load(f)
paths = data.get('qml-import-paths')
if isinstance(paths, str):
    # androiddeployqt expects an array; Qt generates a comma-separated string.
    parts = [p for p in (s.strip() for s in paths.split(',')) if p]
    data['qml-import-paths'] = parts
    with open(json_path, 'w') as f:
        json.dump(data, f, indent=3, sort_keys=True)
PY

qt_host_bin="/home/jessie/Qt/6.10.2/gcc_64/bin"
androiddeployqt="$qt_host_bin/androiddeployqt"
if [[ ! -x "$androiddeployqt" ]]; then
  echo "androiddeployqt not found at $androiddeployqt" >&2
  exit 1
fi

"$androiddeployqt" \
  --input "$json" \
  --output "$app_dir/android-build" \
  --apk "$app_dir/android-build/ereader.apk" \
  --depfile "$app_dir/android-build/ereader.d" \
  --builddir "$app_dir"

echo "APK: $app_dir/android-build/build/outputs/apk/debug/android-build-debug.apk"
