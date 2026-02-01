#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "monocypher.h"

static void run_elligator(const uint8_t *data, size_t size)
{
	if (size < 64) {
		return;
	}
	const uint8_t *hidden = data;
	const uint8_t *seed = data + 32;
	uint8_t tweak = (size > 64) ? data[64] : 0;

	uint8_t curve[32];
	uint8_t hidden2[32];
	crypto_elligator_map(curve, hidden);
	(void)crypto_elligator_rev(hidden2, curve, tweak);

	uint8_t hidden_kp[32];
	uint8_t sk_kp[32];
	uint8_t seed_copy[32];
	for (size_t i = 0; i < 32; i++) {
		seed_copy[i] = seed[i];
	}
	crypto_elligator_key_pair(hidden_kp, sk_kp, seed_copy);

	uint8_t pk_kp[32];
	crypto_x25519_dirty_fast(pk_kp, sk_kp);
	(void)crypto_elligator_rev(hidden2, pk_kp, hidden_kp[31]);
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
	run_elligator(buf, total);
	return 0;
}
