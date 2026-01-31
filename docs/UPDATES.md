# Updates

## Desktop (Linux)
We only update from the Git repo. Use fast-forward only to avoid hidden merges.

```bash
./scripts/update.sh
```

If you're running a packaged build from this repo, update with `scripts/update.sh`
and relaunch via `dist/current/ereader.sh` to pick up the latest packaged binary.
Packaged builds keep settings under `dist/config` so they persist across updates.

## Android
Update path to be defined later. We will not ship background updaters.

## Signed tags
When release tags are signed, verify before updating:

```bash
git fetch --tags
git tag -v <tag>
```
