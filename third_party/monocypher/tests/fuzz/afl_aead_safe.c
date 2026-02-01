#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "monocypher.h"

static void run_aead_safe(const uint8_t *data, size_t size)
{
	if (size < 32 + 24 + 16) {
		return;
	}
	const uint8_t *key = data;
	const uint8_t *nonce = data + 32;
	const uint8_t *mac_in = data + 56;
	size_t pos = 72;
	if (pos > size) {
		return;
	}
	size_t remain = size - pos;
	size_t ad_len = 0;
	if (remain > 0) {
		ad_len = data[pos] % (remain + 1);
		pos += 1;
	}
	if (pos > size) {
		return;
	}
	const uint8_t *ad = data + pos;
	if (ad_len > size - pos) {
		ad_len = size - pos;
	}
	pos += ad_len;
	const uint8_t *msg = data + pos;
	size_t msg_len = size - pos;

	uint8_t *ct = 0;
	uint8_t *pt = 0;
	if (msg_len > 0) {
		ct = (uint8_t *)malloc(msg_len);
		pt = (uint8_t *)malloc(msg_len);
		if (!ct || !pt) {
			free(ct);
			free(pt);
			return;
		}
	}

	uint8_t mac[16];
	crypto_aead_lock(ct, mac, key, nonce, ad, ad_len, msg, msg_len);
	(void)crypto_aead_unlock_safe(pt, mac, key, nonce, ad, ad_len, ct, msg_len);
	(void)crypto_aead_unlock_safe(pt, mac_in, key, nonce, ad, ad_len,
	                              ct, msg_len);

	crypto_aead_ctx ctx_w;
	crypto_aead_ctx ctx_r;
	crypto_aead_init_x(&ctx_w, key, nonce);
	crypto_aead_init_x(&ctx_r, key, nonce);
	if (msg_len > 0) {
		uint8_t mac2[16];
		crypto_aead_write(&ctx_w, ct, mac2, ad, ad_len, msg, msg_len);
		(void)crypto_aead_read_safe(&ctx_r, pt, mac2, ad, ad_len,
		                            ct, msg_len);
		(void)crypto_aead_read_safe(&ctx_r, pt, mac_in, ad, ad_len,
		                            ct, msg_len);
	}

	free(ct);
	free(pt);
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
	run_aead_safe(buf, total);
	return 0;
}
