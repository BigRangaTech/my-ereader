# Updates

## Desktop (Linux)
We only update from the Git repo. Use fast-forward only to avoid hidden merges.

```bash
./scripts/update.sh
```

If you're running a packaged build from this repo, update with `scripts/update.sh`
and relaunch via `dist/current/ereader.sh` to pick up the latest packaged binary.
Packaged builds keep settings under `dist/config` so they persist across updates.
The in-app updater uses the same fast-forward-only flow.

## Flatpak
When running the Flatpak build, updates are handled by Flatpak itself:
```bash
flatpak update
```

### Flatpak repo updates (recommended)
If you publish a Flatpak repo (HTTP, LAN share, or USB), users can add it once and
then update with `flatpak update`.

Add the repo (example local path):
```bash
flatpak remote-add --user --if-not-exists myereader-origin https://bigrangatech.github.io/my-ereader-flatpak/
```

Then update:
```bash
flatpak update
```

To ship an updated repo:
1) Build: `./scripts/build_flatpak.sh`
2) Publish `flatpak/repo` to your host (or copy to USB/LAN)

## Android
Update path to be defined later. We will not ship background updaters.

## Signed tags
When release tags are signed, verify before updating:

```bash
git fetch --tags
git tag -v <tag>
```
