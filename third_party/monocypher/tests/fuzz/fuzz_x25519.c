#include <stddef.h>
#include <stdint.h>

#include "monocypher.h"

static void run_x25519(const uint8_t *data, size_t size)
{
	if (size < 64) {
		return;
	}
	const uint8_t *sk1 = data;
	const uint8_t *pk2 = data + 32;
	uint8_t pk1[32];
	uint8_t ss1[32];

	crypto_x25519_public_key(pk1, sk1);
	crypto_x25519(ss1, sk1, pk2);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	run_x25519(data, size);
	return 0;
}
