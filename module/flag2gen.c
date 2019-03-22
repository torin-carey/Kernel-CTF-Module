#include <stdio.h>

#include "sha256.h"
#include "flag.h"

static char alpha[] = "0123456789abcdef";

int main(void) {
	struct sha256_state state;
	sha256_init(&state);
	sha256_update(&state, key, sizeof(key));
	sha256_update(&state, "UNLOCK_FLAG2", 12);
	unsigned char mac[32];
	sha256_final(&state, mac);
	fputs("Message: 'UNLOCK_FLAG2'\nDigest: 0x", stdout);
	for (int i = 0; i < 32; i++) {
		putchar(alpha[mac[i] >> 4]);
		putchar(alpha[mac[i] & 0xf]);
	}
	putchar('\n');
}
