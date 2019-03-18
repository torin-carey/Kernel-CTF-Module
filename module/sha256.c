#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/string.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Torin Carey <tcarey1@sheffield.ac.uk>");
#else
#include <stdint.h>
#include <string.h>

#define min(a,b) (((a) > (b)) ? (b) : (a))
#endif

#include "sha256.h"
#define RROT(n, l) (((n >> l) | (n << (32-l))) & 0xffffffff)
const uint32_t k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const uint32_t initial_h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

void sha256_init(struct sha256_state *state) {
	int i;
	for (i = 0; i < 8; i++)
		state->h[i] = initial_h[i];
	state->size = 0;
	state->L = 0;
}


void sha256_update(struct sha256_state *state, const unsigned char *data, unsigned int len) {
	unsigned int r;
	if (state->size) {
		r = min(64 - state->size, len);
		memcpy(state->buf + state->size, data, r);
		state->size += r;
		data += r;
		len -= r;
		if (state->size == 64) {
			add_chunk(state->h, state->buf);
			state->L += 512;
			state->size = 0;
		} else {
			return;
		}
	}
	while (len >= 64) {
		add_chunk(state->h, data);
		state->L += 512;
		data += 64;
		len -= 64;
	}
	if (len) {
		memcpy(state->buf + state->size, data, len);
		state->size += len;
	}
}

void sha256_final(struct sha256_state *state, unsigned char *digest) {
	int i;
	unsigned char buf[72];
	unsigned long size;

	state->L += state->size * 8;
	size = state->size <= 55 ? (64 - state->size) : (128 - state->size);
	memset(buf, 0, size);
	buf[0] = 0x80;
	for (i = 0; i < 8; i++)
		buf[size - 8 + i] = state->L >> ((7 - i) * 8);
	sha256_update(state, buf, size);
	for (i = 0; i < 8; i++) {
		digest[4*i] = state->h[i] >> 24;
		digest[4*i + 1] = state->h[i] >> 16;
		digest[4*i + 2] = state->h[i] >> 8;
		digest[4*i + 3] = state->h[i];
	}
}

void add_chunk(uint32_t *h, const unsigned char *chunk) {
	int i;
	uint32_t w[64];
	uint32_t la, lb, lc, ld, le, lf, lg, lh;
	for (i = 0; i < 16; i++)
		w[i] = chunk[4*i] << 24
			| chunk[4*i + 1] << 16
			| chunk[4*i + 2] << 8
			| chunk[4*i + 3];
	for (i = 16; i < 64; i++) {
		uint32_t s0, s1;
		s0 = RROT(w[i-15], 7) ^ RROT(w[i-15], 18) ^ (w[i-15] >> 3);
		s1 = RROT(w[i-2], 17) ^ RROT(w[i-2], 19) ^ (w[i-2] >> 10);
		w[i] = w[i-16] + s0 + w[i-7] + s1;
	}
	la = h[0];
	lb = h[1];
	lc = h[2];
	ld = h[3];
	le = h[4];
	lf = h[5];
	lg = h[6];
	lh = h[7];


	for (i = 0; i < 64; i++) {
		uint32_t S1, ch, temp1, S0, maj, temp2;
		S1 = RROT(le, 6) ^ RROT(le, 11) ^ RROT(le, 25);
		ch = (le & lf) ^ (~le & lg);
		temp1 = lh + S1 + ch + k[i] + w[i];
		S0 = RROT(la, 2) ^ RROT(la, 13) ^ RROT(la, 22);
		maj = (la & lb) ^ (la & lc) ^ (lb & lc);
		temp2 = S0 + maj;

		lh = lg;
		lg = lf;
		lf = le;
		le = ld + temp1;
		ld = lc;
		lc = lb;
		lb = la;
		la = temp1 + temp2;
	}
	h[0] += la;
	h[1] += lb;
	h[2] += lc;
	h[3] += ld;
	h[4] += le;
	h[5] += lf;
	h[6] += lg;
	h[7] += lh;
}
