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

### Fedora (older Flatpak) two-step add
Some Fedora installs don’t support `--gpg-import` on `remote-add`. Add the remote
first, then import the key:

```bash
flatpak remote-add --system --if-not-exists myereader \
  https://bigrangatech.github.io/my-ereader-flatpak/

flatpak remote-modify --system --gpg-import=/home/jessie/Downloads/repo-gpg.pub myereader
```

Then install:
```bash
flatpak install --system myereader app/com.bigrangatech.MyEreader/x86_64/master
```

### Common mistakes and fixes
- **Remote URL shows `file:///home/... https://...`**  
  Cause: leading space or stray backslash before the URL.  
  Fix: re-add the remote with a clean URL:
  ```bash
  flatpak remote-delete --system myereader
  flatpak remote-add --system --if-not-exists myereader \
    https://bigrangatech.github.io/my-ereader-flatpak/
  ```
- **“No remote refs found for ‘myereader’”**  
  Cause: remote not added, wrong URL, or repo not published.  
  Fix: `flatpak remote-ls --system myereader` and verify the repo exports refs.
- **Remote exists in both user and system installs**  
  Fix: remove one: `flatpak remote-delete --user myereader` or `--system`.
- **“Invalid id myereader: Names must contain at least 2 periods”**  
  Cause: using the remote name as the app id.  
  Fix: update/install by app id:  
  `flatpak update com.bigrangatech.MyEreader`  
  `flatpak install --system myereader app/com.bigrangatech.MyEreader/x86_64/master`
- **“Remote found in multiple installations”**  
  Fix: add `--user` or `--system` to `remote-add`, `remote-ls`, `remote-info`, `remote-delete`.
- **Appstream/GPG errors**  
  Appstream is optional. You can install/update by ref even without appstream.

## Android
Update path to be defined later. We will not ship background updaters.

## Signed tags
When release tags are signed, verify before updating:

```bash
git fetch --tags
git tag -v <tag>
```
