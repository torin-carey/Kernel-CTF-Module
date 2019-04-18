#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <ctfmod.h>

#define ENV_FLAG1 "FLAG1"
#define ENV_FLAG2 "FLAG2"
#define ENV_FLAG3 "FLAG3"
#define ENV_KEY   "KEY"

#define KEY_LEN 16

static const char *HEXALPHA = "0123456789abcdef";

int valid_flag(const char *flag)
{
	if (strlen(flag) != FLAG_LEN - 1) {
		fprintf(stderr, "flag invalid: '%s' must be %d characters\n",
							flag, FLAG_LEN-1);
		return 0;
	}
	return 1;
}

int parse_key(unsigned char key[KEY_LEN], const char *str)
{
	int i = 0;
	int b = -1;
	char c;
	const char *pos;
	while ((c = *str++)) {
		pos = strchr(HEXALPHA, tolower(c));
		if (!pos)
			continue;
		if (i == KEY_LEN) {
			fprintf(stderr, "key invalid: must be %zu bytes\n", KEY_LEN);
			return -1;
		}
		if (b == -1) {
			b = (pos - HEXALPHA) << 4;
		} else {
			key[i++] = b | (pos - HEXALPHA);
			b = -1;
		}
	}
	if (i != KEY_LEN || b != -1) {
		fprintf(stderr, "key invalid: must be %zu bytes\n", KEY_LEN);
		return -1;
	}
	return 0;
}

int main(void)
{
	int ret;

	const char *flag1, *flag2, *flag3, *key;
	flag1 = getenv(ENV_FLAG1);
	flag2 = getenv(ENV_FLAG2);
	flag3 = getenv(ENV_FLAG3);
	key = getenv(ENV_KEY);

	ret = 0;
	if (!flag1) {
		fprintf(stderr, "missing FLAG1\n");
		ret = -1;
	}
	if (!flag2) {
		fprintf(stderr, "missing FLAG2\n");
		ret = -1;
	}
	if (!flag3) {
		fprintf(stderr, "missing FLAG3\n");
		ret = -1;
	}
	if (!key) {
		fprintf(stderr, "missing KEY\n");
		ret = -1;
	}
	if (ret)
		goto finish;

	if (!valid_flag(flag1) || !valid_flag(flag2) || !valid_flag(flag3)) {
		ret = -1;
		goto finish;
	}

	struct flag_key fk;
	memcpy(fk.flag[0], flag1, FLAG_LEN);
	memcpy(fk.flag[1], flag2, FLAG_LEN);
	memcpy(fk.flag[2], flag3, FLAG_LEN);

	if ((ret = parse_key(fk.key, key))) {
		goto finish;
	}

	int fd = open("/dev/flag", O_RDONLY);
	if (fd == -1) {
		ret = errno;
		fprintf(stderr, "failed to open device: %m\n");
		goto finish;
	}

	if (ioctl(fd, CTFMOD_LOAD_SECRETS, &fk) == -1) {
		ret = errno;
		switch (ret) {
		case EBUSY:
			fprintf(stderr, "failed to load secrets: already initialised\n");
			break;
		default:
			fprintf(stderr, "failed to load secrets: %m\n");
		}
		goto rel_dev;
	}
	ret = 0;
	printf("secrets loaded successfully\n");
rel_dev:
	close(fd);
finish:
	return ret;
}
