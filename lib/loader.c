#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>

#include <ctfmod.h>

#define CTFMOD_DEV "/dev/flag"

#define RET_SUCCESS 0
#define RET_MISSING 1
#define RET_INPUT   2
#define RET_PERM    3
#define RET_OPEN    4

#define ENV_FLAG1 "FLAG1"
#define ENV_FLAG2 "FLAG2"
#define ENV_FLAG3 "FLAG3"
#define ENV_KEY   "KEY"

#define KEY_LEN 16

static const char *usage =
"Usage: %s [options]\n\n"
"Options:\n"
"-s, --show\t\tshow secrets\n"
"-1, --flag1 FLAG\tset flag1\n"
"-2, --flag2 FLAG\tset flag2\n"
"-3, --flag3 FLAG\tset flag3\n"
"-k, --key KEY\t\tset key\n";

static const char *HEXALPHA = "0123456789abcdef";

static const struct option longopt[] = {
	{"help", no_argument, NULL, '?'},
	{"show", no_argument, NULL, 's'},
	{"flag1", required_argument, NULL, '1'},
	{"flag2", required_argument, NULL, '2'},
	{"flag3", required_argument, NULL, '3'},
	{"key", required_argument, NULL, 'k'},
	{NULL, 0, NULL, 0}
};

static const char *shortopt = "s1:2:3:k:?";

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
			fprintf(stderr, "key invalid: must be %u bytes\n", KEY_LEN);
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
		fprintf(stderr, "key invalid: must be %u bytes\n", KEY_LEN);
		return -1;
	}
	return 0;
}

int set_secrets(const char *flag1, const char *flag2, const char *flag3, const char *key);

int get_secrets(void);

int main(int argc, char **argv)
{
	int ret;
	int showonly = 0;

	const char *flag1, *flag2, *flag3, *key;
	flag1 = getenv(ENV_FLAG1);
	flag2 = getenv(ENV_FLAG2);
	flag3 = getenv(ENV_FLAG3);
	key = getenv(ENV_KEY);

	while ((ret = getopt_long(argc, argv, shortopt, longopt, NULL)) != -1) {
		switch (ret) {
		case 's':
			showonly = 1;
			break;
		case '1':
			flag1 = optarg;
			break;
		case '2':
			flag2 = optarg;
			break;
		case '3':
			flag3 = optarg;
			break;
		case 'k':
			key = optarg;
			break;
		case '?':
			fprintf(stderr, usage, argv[0]);
			return RET_INPUT;
		}
	}

	if (showonly) {
		return get_secrets();
	} else {
		ret = 0;
		if (!flag1) {
			fprintf(stderr, "missing " ENV_FLAG1 "\n");
			ret = RET_MISSING;
		}
		if (!flag2) {
			fprintf(stderr, "missing " ENV_FLAG2 "\n");
			ret = RET_MISSING;
		}
		if (!flag3) {
			fprintf(stderr, "missing " ENV_FLAG3 "\n");
			ret = RET_MISSING;
		}
		if (!key) {
			fprintf(stderr, "missing " ENV_KEY "\n");
			ret = RET_MISSING;
		}
		if (ret)
			return ret;

		return set_secrets(flag1, flag2, flag3, key);
	}
}

int get_secrets(void)
{
	int ret;
	struct flag_key fk;
	int fd = open(CTFMOD_DEV, O_RDONLY);
	if (fd == -1) {
		ret = RET_OPEN;
		fprintf(stderr, "failed to open device: %m\n");
		goto finish;
	}

	if (ioctl(fd, CTFMOD_RETRIEVE_SECRETS, &fk) == -1) {
		switch (errno) {
		case EBUSY:
			fprintf(stderr, "failed to retrieve secrets: not initialised\n");
			break;
		default:
			fprintf(stderr, "failed to retrieve secrets: %m\n");
		}
		ret = RET_PERM;
		goto rel_dev;
	}

	// Just to be safe
	fk.flag[0][FLAG_LEN - 1] = '\0';
	fk.flag[1][FLAG_LEN - 1] = '\0';
	fk.flag[2][FLAG_LEN - 1] = '\0';
	char key[33];
	for (int i = 0; i < 16; i++) {
		key[2*i] = HEXALPHA[fk.key[i] >> 4];
		key[2*i + 1] = HEXALPHA[fk.key[i] & 0xf];
	}
	key[32] = '\0';

	printf("FLAG1:\t%s\nFLAG2:\t%s\nFLAG3:\t%s\nKEY:\t%s\n",
				fk.flag[0], fk.flag[1], fk.flag[2], key);
	ret = RET_SUCCESS;
rel_dev:
	close(fd);
finish:
	return ret;
}

int set_secrets(const char *flag1, const char *flag2, const char *flag3, const char *key)
{
	int ret;
	if (!valid_flag(flag1) || !valid_flag(flag2) || !valid_flag(flag3)) {
		ret = RET_INPUT;
		goto finish;
	}

	struct flag_key fk;
	memcpy(fk.flag[0], flag1, FLAG_LEN);
	memcpy(fk.flag[1], flag2, FLAG_LEN);
	memcpy(fk.flag[2], flag3, FLAG_LEN);

	if (parse_key(fk.key, key)) {
		ret = RET_INPUT;
		goto finish;
	}

	int fd = open(CTFMOD_DEV, O_RDONLY);
	if (fd == -1) {
		ret = RET_OPEN;
		fprintf(stderr, "failed to open device: %m\n");
		goto finish;
	}

	if (ioctl(fd, CTFMOD_LOAD_SECRETS, &fk) == -1) {
		switch (errno) {
		case EBUSY:
			fprintf(stderr, "failed to load secrets: already initialised\n");
			break;
		default:
			fprintf(stderr, "failed to load secrets: %m\n");
		}
		ret = RET_PERM;
		goto rel_dev;
	}
	ret = RET_SUCCESS;
	printf("secrets loaded successfully\n");
rel_dev:
	close(fd);
finish:
	return ret;
}
