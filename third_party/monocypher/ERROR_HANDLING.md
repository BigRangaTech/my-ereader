# Error Handling Plan

This fork uses explicit error codes to reduce misuse and make failures
actionable. This document captures the plan for improving error handling
over time.

Current state
-------------

- `crypto_err` enum provides stable error codes:
  `CRYPTO_ERR_NULL`, `CRYPTO_ERR_SIZE`, `CRYPTO_ERR_OVERFLOW`,
  `CRYPTO_ERR_AUTH`, `CRYPTO_ERR_CONFIG`.
- Checked wrappers validate pointers/sizes and return `crypto_err`.
- Checked wrappers cover core APIs and optional SHA-512/Ed25519 APIs,
  including incremental interfaces.
- `crypto_random` returns `crypto_err` on failure.
- Optional `crypto_strerror(int)` helper is available when compiled with
  `MONOCYPHER_STRERROR=1`.
- Optional RNG diagnostics (`MONOCYPHER_RNG_DIAGNOSTICS=1`) expose
  `crypto_random_last_error()` for platform-specific error detail.

Plan
----

1) **Complete checked coverage**
   - Audit remaining public APIs and add `*_checked` variants where missing.
   - Ensure auth failures consistently map to `CRYPTO_ERR_AUTH`.

2) **Error-string helper (optional)**
   - Implemented behind `MONOCYPHER_STRERROR=1`; keep strings stable and
     documented.

3) **Documentation alignment**
   - Update manpages and examples to prefer checked APIs for application code.
   - Document the error mapping for each checked wrapper in the API docs.

4) **RNG failure detail (optional)**
   - Implemented: `MONOCYPHER_RNG_DIAGNOSTICS=1` adds
     `crypto_random_last_error()` for debugging.

Policy
------

New public APIs should include a checked variant unless there is a clear
reason not to. Documentation should call out expected error codes.
