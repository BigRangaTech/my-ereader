# Custom Encryption

## Vault format (draft)
- File header:
  - Magic: "MYEVAULT"
  - Version: 1
  - Salt size (u32), Nonce size (u32), Ciphertext size (u64)
  - KDF parameters (opslimit, memlimit)
- Payload:
  - Salt
  - Nonce
  - Ciphertext (AEAD)

## Goals
- Self-contained encrypted container for local data
- Passphrase-derived key material only; no cloud keys
- Authenticated encryption for integrity

## Backend
- Default: vendored Monocypher (Argon2id + AEAD)
- Optional: libsodium backend if enabled
- Fallback: no crypto backend (encryption disabled)

## TODO
- Streaming encryption for large files
- Vault versioning and migration
- Passphrase rotation
