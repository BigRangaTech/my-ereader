# Security

## Threat model (initial)
- Protect book content, annotations, and reading history at rest
- Prevent data leakage over network
- Minimize attack surface on mobile and desktop

## Controls
- Encryption at rest for the library database and annotation content
  - Key derived from a user passphrase (KDF with strong parameters)
  - No cloud key storage; keys stay local to the device
- Network defaults
  - Sync is off by default
  - LAN-only discovery and pairing
  - TLS for all sync traffic
- Sandboxing
  - Android scoped storage
  - Desktop minimal permissions and no background services by default
- Updates
  - Desktop: git-based updates only (fast-forward only)
  - Verify release signatures before updating

## Custom encryption (design)
- Use a custom vault file format for encrypted data at rest
- The format is custom, but primitives are standard (KDF + authenticated encryption)
- Backend options: vendored Monocypher (default) or libsodium when available

## Passphrase handling (draft)
- Use a memory-hard KDF (Argon2id or scrypt) with device-tuned parameters
- Store only a verifier; never store the raw passphrase
- Keep decrypted keys in memory only for the active session
- Provide a manual lock action and auto-lock after inactivity

## Notes
- DRM bypass is out of scope
- Any network exposure is strictly opt-in and local network only
