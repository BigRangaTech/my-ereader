#include <stddef.h>
#include <stdint.h>

#include "monocypher.h"

static void run_eddsa(const uint8_t *data, size_t size)
{
	if (size < 32) {
		return;
	}
	const uint8_t *seed = data;
	const uint8_t *msg = data + 32;
	size_t msg_len = size - 32;

	uint8_t sk[64];
	uint8_t pk[32];
	uint8_t sig[64];
	uint8_t seed_copy[32];
	for (size_t i = 0; i < 32; i++) {
		seed_copy[i] = seed[i];
	}

	crypto_eddsa_key_pair(sk, pk, seed_copy);
	crypto_eddsa_sign(sig, sk, msg, msg_len);
	(void)crypto_eddsa_check(sig, pk, msg, msg_len);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	run_eddsa(data, size);
	return 0;
}
