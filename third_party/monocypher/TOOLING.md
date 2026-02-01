# Tooling

This document tracks test, coverage, and fuzzing tooling for this fork.

Tests
-----

Full test suite:

    $ make test

Hardened build profile:

    $ make harden

Sanitizers + Valgrind (requires clang + valgrind):

    $ tests/test.sh

Local toolchain wrapper (uses local clang/valgrind installs):

    $ tests/test-local.sh

Override tool locations for `tests/test-local.sh`:

    $ LLVM_DIR=/path/to/llvm VALGRIND_DIR=/path/to/valgrind tests/test-local.sh

Coverage
--------

Requires `llvm-profdata` and `llvm-cov`:

    $ tests/coverage.sh

Fuzzing
-------

Harnesses live under `tests/fuzz/` for LibFuzzer and AFL.

Seed corpus (deterministic, small):

    $ tests/fuzz/gen_corpus.sh tests/fuzz/corpus

Make targets:

    $ make fuzz
    $ make fuzz-corpus
    $ make fuzz-run

Common overrides:

    $ make fuzz FUZZ_CC=/path/to/clang FUZZ_LDFLAGS=-L/path/to/llvm/lib
    $ make fuzz-run FUZZ_RUNS=10000

Manual build commands and AFL examples:

    See `tests/fuzz/README.md`.

Planned tooling
---------------

None tracked yet. This section is kept up to date as we add tooling.
