#!/bin/sh

set -e

LLVM_DIR="${LLVM_DIR:-/home/jessie/.local/LLVM-21.1.8-Linux-X64}"
VALGRIND_DIR="${VALGRIND_DIR:-/home/jessie/.local/valgrind-3.26.0}"

if [ ! -x "$LLVM_DIR/bin/clang" ]; then
	echo "clang not found at $LLVM_DIR/bin/clang" >&2
	echo "Set LLVM_DIR or install clang before running this script." >&2
	exit 1
fi

if [ ! -x "$VALGRIND_DIR/bin/valgrind" ]; then
	echo "valgrind not found at $VALGRIND_DIR/bin/valgrind" >&2
	echo "Set VALGRIND_DIR or install valgrind before running this script." >&2
	exit 1
fi

PATH="$LLVM_DIR/bin:$VALGRIND_DIR/bin:$PATH"
LD_LIBRARY_PATH="$LLVM_DIR/lib:${LD_LIBRARY_PATH:-}"
export PATH LD_LIBRARY_PATH

exec "$(dirname "$0")/test.sh"
