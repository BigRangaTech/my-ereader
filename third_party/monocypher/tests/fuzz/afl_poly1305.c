#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "monocypher.h"

static void run_poly1305(const uint8_t *data, size_t size)
{
	if (size < 32) {
		return;
	}
	const uint8_t *key = data;
	const uint8_t *msg = data + 32;
	size_t msg_len = size - 32;

	uint8_t mac1[16];
	uint8_t mac2[16];
	crypto_poly1305(mac1, msg, msg_len, key);

	crypto_poly1305_ctx ctx;
	crypto_poly1305_init(&ctx, key);
	size_t half = msg_len / 2;
	crypto_poly1305_update(&ctx, msg, half);
	crypto_poly1305_update(&ctx, msg + half, msg_len - half);
	crypto_poly1305_final(&ctx, mac2);

	(void)mac1;
	(void)mac2;
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
	run_poly1305(buf, total);
	return 0;
}
