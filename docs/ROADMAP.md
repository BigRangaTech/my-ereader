# Roadmap

## Phase 0: Baseline scaffold
- [x] Qt Quick app shell
- [x] Library database schema
- [x] Format provider interface
- [x] Simple TXT reader
- [ ] Desktop update script (git pull --ff-only)

## Phase 1: Core reading
- [x] EPUB provider
- [x] PDF provider
- [x] Pagination and navigation
- [x] Annotation UI
- [x] Library import + cover generation

## Phase 2: Format expansion
- [x] MOBI/AZW3 provider
- [x] CBZ/CBR provider
- [x] FB2 provider
- [x] DJVU provider
- [x] libmobi b01.1 adoption:
  - [x] RESC resource parsing for KF8 spine/TOC
  - [x] EXTH_TTSDISABLE -> disable TTS per book
  - [x] Prefer KF8 when hybrid (explicit flag/log)
  - [x] RESC/EXTH cover fallbacks (when cover offset missing)
  - [x] Extra EXTH metadata (language, page-dir, writing-mode, fixed-layout)

## Phase 3: Sync + TTS
- [x] LAN pairing
- [x] Delta sync for library metadata + annotations
- [x] TTS controls + playback

## Phase 4: Performance + polish
- [ ] Caching and pre-render
- [ ] Large library optimizations
- [ ] UI refinement and accessibility

## Phase 5: Security hardening
- [ ] Encrypted DB + secure key handling
- [ ] Update verification (signed tags)
- [ ] Custom vault format + migration tooling
- [ ] Monocypher v002 adoption:
  - [x] Migrate CryptoBackendMonocypher to checked APIs to harden size/pointer handling
    - [x] Use checked AEAD/hash/signature APIs where available
    - [x] Map Monocypher error codes via crypto_strerror in logs
  - [x] Replace any non-crypto RNG usage in security paths with crypto_random
    - [x] Salts, nonces, passphrase KDF inputs
    - [x] Add optional RNG diagnostics in debug builds
  - [ ] Adopt SHA-256/BLAKE3 helpers where appropriate
    - [ ] BLAKE3 for fast file hashing/indexing (optional)
    - [ ] SHA-256 for security-sensitive integrity checks
  - [ ] Enable build profiles
    - [ ] HARDEN for release builds
    - [ ] SANITIZE for CI/debug builds
    - [ ] SIZE profile for mobile/embedded builds if needed
  - [ ] Review SIMD/runtime dispatch impact
    - [ ] Ensure portability on older CPUs
    - [ ] Validate no regressions on ARM/NEON
  - [ ] Optional: thread-enabled Argon2/BLAKE3/ChaCha20 if performance needs justify
