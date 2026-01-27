# Security

## Threat model (initial)
- Protect book content, annotations, and reading history at rest
- Prevent data leakage over network
- Minimize attack surface on mobile and desktop

## Controls
- Encryption at rest for the library database and annotation content
  - Key derived from a user passphrase (KDF with strong parameters)
- Network defaults
  - Sync is off by default
  - LAN-only discovery and pairing
  - TLS for all sync traffic
- Sandboxing
  - Android scoped storage
  - Desktop minimal permissions and no background services by default
- Updates
  - Git-based updates only (signed release tags)

## Notes
- DRM bypass is out of scope
- Any network exposure is strictly opt-in and local network only
