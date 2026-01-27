# Updates

## Desktop (Linux)
We only update from the Git repo. Use fast-forward only to avoid hidden merges.

```bash
./scripts/update.sh
```

## Android
Update path to be defined later. We will not ship background updaters.

## Signed tags
When release tags are signed, verify before updating:

```bash
git fetch --tags
git tag -v <tag>
```
