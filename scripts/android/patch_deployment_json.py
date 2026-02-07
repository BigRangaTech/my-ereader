#!/usr/bin/env python3
import json
import sys
from pathlib import Path

def main():
    if len(sys.argv) != 2:
        print("Usage: patch_deployment_json.py <deployment-settings.json>", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    if not path.exists():
        print(f"Deployment settings not found: {path}", file=sys.stderr)
        return 1
    data = json.loads(path.read_text())
    paths = data.get("qml-import-paths")
    if isinstance(paths, str):
        parts = [p.strip() for p in paths.split(",") if p.strip()]
        data["qml-import-paths"] = parts
        path.write_text(json.dumps(data, indent=3, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
