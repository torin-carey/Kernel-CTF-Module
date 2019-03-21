#ifndef H_SHA256
#define H_SHA256

#include <stdint.h>

struct sha256_state {
	uint32_t h[8];
	unsigned char buf[64];
	unsigned int size;
	uint64_t L;
};

void add_chunk(uint32_t *h, const unsigned char *chunk);

void sha256_init(struct sha256_state *state);

void sha256_update(struct sha256_state *state, const unsigned char *data, unsigned int len);

void sha256_final(struct sha256_state *state, unsigned char *digest);

#endif // H_SHA256
