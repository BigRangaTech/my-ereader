#!/bin/sh

set -e

OUT_DIR="${1:-tests/fuzz/corpus}"
PYTHON="${PYTHON:-python3}"

if ! command -v "$PYTHON" >/dev/null 2>&1; then
	echo "python3 not found (set PYTHON=... to override)" >&2
	exit 1
fi

"$PYTHON" - "$OUT_DIR" <<'PY'
import os
import sys

out_dir = sys.argv[1]

def seq(start, length):
    return bytes(((start + i) & 0xFF) for i in range(length))

def write(target, name, data):
    d = os.path.join(out_dir, target)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, name), "wb") as f:
        f.write(data)

def aead_seed(ad_len, msg_len, tweak=0):
    key = seq(0 + tweak, 32)
    nonce = seq(32 + tweak, 24)
    mac = bytes([tweak]) * 16
    ad = seq(64 + tweak, ad_len)
    msg = seq(96 + tweak, msg_len)
    return key + nonce + mac + bytes([ad_len % 256]) + ad + msg

for i, (ad_len, msg_len) in enumerate([(0, 0), (4, 8), (16, 32), (1, 64)]):
    data = aead_seed(ad_len, msg_len, i)
    write("fuzz_aead", f"seed{i}", data)
    write("fuzz_aead_safe", f"seed{i}", data)

write("fuzz_x25519", "zero", bytes(64))
write("fuzz_x25519", "seq", seq(0, 64))
write("fuzz_x25519", "ff", bytes([0xFF]) * 64)

write("fuzz_eddsa", "empty", seq(0, 32))
write("fuzz_eddsa", "msg1", seq(0, 33))
write("fuzz_eddsa", "msg64", seq(0, 96))

write("fuzz_argon2", "short", seq(0, 16))
write("fuzz_argon2", "medium", seq(0, 80))
write("fuzz_argon2", "long", seq(0, 256))

def hash_seed(selector, msg_len):
    return bytes([selector]) + seq(selector, msg_len)

write("fuzz_hashes", "sel0", hash_seed(0, 0))
write("fuzz_hashes", "sel1", hash_seed(1, 1))
write("fuzz_hashes", "sel2", hash_seed(2, 64))
write("fuzz_hashes", "sel3", hash_seed(3, 255))

def poly_seed(msg_len, tweak=0):
    key = seq(tweak, 32)
    msg = seq(32 + tweak, msg_len)
    return key + msg

write("fuzz_poly1305", "empty", poly_seed(0))
write("fuzz_poly1305", "short", poly_seed(8, 1))
write("fuzz_poly1305", "block", poly_seed(16, 2))
write("fuzz_poly1305", "long", poly_seed(128, 3))

def elligator_seed(tweak=0):
    hidden = seq(0, 32)
    seed = seq(32, 32)
    return hidden + seed + bytes([tweak])

write("fuzz_elligator", "tweak0", elligator_seed(0))
write("fuzz_elligator", "tweak1", elligator_seed(1))
write("fuzz_elligator", "tweak255", elligator_seed(255))
PY
