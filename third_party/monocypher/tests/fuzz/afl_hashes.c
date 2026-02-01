#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "monocypher.h"

static void run_hashes(const uint8_t *data, size_t size)
{
	if (size == 0) {
		return;
	}
	size_t pos = 0;
	uint8_t selector = data[pos++];
	const uint8_t *msg = data + pos;
	size_t msg_len = size - pos;

	uint8_t key[32] = {0};
	size_t key_len = msg_len < sizeof(key) ? msg_len : sizeof(key);
	if (key_len > 0) {
		memcpy(key, msg, key_len);
	}

	size_t hash_size = (size_t)(selector % 64) + 1;
	uint8_t hash64[64];
	uint8_t hash32[32];
	uint8_t okm[64];
	uint8_t b3_out[64];

	crypto_blake2b(hash64, hash_size, msg, msg_len);
	crypto_blake2b_keyed(hash64, hash_size, key, sizeof(key), msg, msg_len);
	crypto_sha256(hash32, msg, msg_len);
	crypto_sha256_hmac(hash32, key, sizeof(key), msg, msg_len);

	size_t okm_size = (size_t)(selector % sizeof(okm)) + 1;
	crypto_sha256_hkdf(okm, okm_size,
	                   key, sizeof(key),
	                   key, sizeof(key),
	                   msg, msg_len);

	size_t b3_out_size = (size_t)(selector % sizeof(b3_out)) + 1;
	crypto_blake3(b3_out, b3_out_size, msg, msg_len);
	crypto_blake3_keyed(b3_out, b3_out_size, key, msg, msg_len);
	crypto_blake3_derive_key(b3_out, b3_out_size,
	                         key, sizeof(key),
	                         msg, msg_len);
}

int main(void)
{
	uint8_t buf[4096];
	size_t total = 0;
	while (total < sizeof(buf)) {
		ssize_t n = read(0, buf + total, sizeof(buf) - total);
		if (n <= 0) {
			break;
		}
		total += (size_t)n;
	}
	run_hashes(buf, total);
	return 0;
}
