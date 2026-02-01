#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

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

int main(void)
{
	uint8_t buf[256];
	size_t total = 0;
	while (total < sizeof(buf)) {
		ssize_t n = read(0, buf + total, sizeof(buf) - total);
		if (n <= 0) {
			break;
		}
		total += (size_t)n;
	}
	run_x25519(buf, total);
	return 0;
}
