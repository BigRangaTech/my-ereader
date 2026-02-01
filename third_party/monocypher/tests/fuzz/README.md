Fuzzing harnesses

Seed corpus:
  tests/fuzz/gen_corpus.sh tests/fuzz/corpus
  # Regenerate anytime; the corpus is small and deterministic.

See `TOOLING.md` for the full tooling overview.

LibFuzzer:
  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_aead.c src/monocypher.c -o fuzz_aead

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_aead_safe.c src/monocypher.c -o fuzz_aead_safe

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_x25519.c src/monocypher.c -o fuzz_x25519

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_eddsa.c src/monocypher.c -o fuzz_eddsa

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_argon2.c src/monocypher.c -o fuzz_argon2

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_hashes.c src/monocypher.c -o fuzz_hashes

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_poly1305.c src/monocypher.c -o fuzz_poly1305

  clang -fsanitize=fuzzer,address,undefined -I src -I src/optional \
    tests/fuzz/fuzz_elligator.c src/monocypher.c -o fuzz_elligator

AFL:
  afl-clang-fast -I src -I src/optional tests/fuzz/afl_aead.c \
    src/monocypher.c -o afl_aead

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_aead_safe.c \
    src/monocypher.c -o afl_aead_safe

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_x25519.c \
    src/monocypher.c -o afl_x25519

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_eddsa.c \
    src/monocypher.c -o afl_eddsa

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_argon2.c \
    src/monocypher.c -o afl_argon2

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_hashes.c \
    src/monocypher.c -o afl_hashes

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_poly1305.c \
    src/monocypher.c -o afl_poly1305

  afl-clang-fast -I src -I src/optional tests/fuzz/afl_elligator.c \
    src/monocypher.c -o afl_elligator

Make targets:
  make fuzz
  make fuzz-corpus
  make fuzz-run
  # If clang isn't on PATH, override:
  #   make fuzz FUZZ_CC=/path/to/clang FUZZ_LDFLAGS=-L/path/to/llvm/lib
