#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "monocypher.h"

static void run_argon2(const uint8_t *data, size_t size)
{
	uint8_t hash[32];
	uint8_t pass[64];
	uint8_t salt[16];

	size_t pass_len = size < sizeof(pass) ? size : sizeof(pass);
	for (size_t i = 0; i < pass_len; i++) {
		pass[i] = data[i];
	}
	for (size_t i = pass_len; i < sizeof(pass); i++) {
		pass[i] = 0;
	}
	for (size_t i = 0; i < sizeof(salt); i++) {
		salt[i] = (i < size) ? data[i] : 0;
	}

	crypto_argon2_config config;
	config.algorithm = CRYPTO_ARGON2_I;
	config.nb_blocks = 8;
	config.nb_passes = 1;
	config.nb_lanes = 1;

	crypto_argon2_inputs inputs;
	inputs.pass = pass;
	inputs.salt = salt;
	inputs.pass_size = (uint32_t)pass_len;
	inputs.salt_size = (uint32_t)sizeof(salt);

	crypto_argon2_extras extras;
	extras.key = 0;
	extras.ad = 0;
	extras.key_size = 0;
	extras.ad_size = 0;

	size_t work_area_size = config.nb_blocks * 1024;
	uint8_t *work_area = (uint8_t *)malloc(work_area_size);
	if (!work_area) {
		return;
	}
	crypto_argon2(hash, sizeof(hash), work_area, config, inputs, extras);
	free(work_area);
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
	run_argon2(buf, total);
	return 0;
}
