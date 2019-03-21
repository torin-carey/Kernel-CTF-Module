#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "flagio.h"

static int dev_flag_fd = -1;

int flag_open(void) {
	if (dev_flag_fd != -1)
		return 0;
	dev_flag_fd = open("/dev/flag", O_RDONLY);
	if (dev_flag_fd == -1) {
		fprintf(stderr, "Failed to open device: %m\n");
		return -1;
	}
	return 0;
}

void flag_close(void) {
	if (dev_flag_fd != -1)
		close(dev_flag_fd);
}

int flag_auth(const struct auth_data *auth) {
	int ret;
	ret = ioctl(dev_flag_fd, IOCTL_AUTHENTICATE, (unsigned long)auth);
	if (ret == -1) {
		switch (errno) {
		case EPERM:
			fputs("Failed to authenticate: Bad MAC\n", stderr);
			break;
		case EINVAL:
			fputs("Failed to authenticate: Invalid auth_data format\n", stderr);
			break;
		default:
			fprintf(stderr, "Failed to authenticate: %m\n");
		}
		return -1;
	}
	return 0;
}

const char *get_flag1(void) {
	int ret;
	static char flag1[FLAG_MAX_LEN];
	ret = ioctl(dev_flag_fd, IOCTL_GET_FLAG1, (unsigned long)flag1);
	if (ret == -1) {
		fprintf(stderr, "Failed to get flag1: %m\n", stderr);
		return NULL;
	}
	return flag1;
}

const char *get_flag2(void) {
	int ret;
	static char flag2[FLAG_MAX_LEN];
	ret = ioctl(dev_flag_fd, IOCTL_GET_FLAG2, (unsigned long)flag2);
	if (ret == -1) {
		switch (errno) {
		case EPERM:
			fputs("Failed to get flag2: Not authorized\n", stderr);
			break;
		default:
			fprintf(stderr, "Failed to get flag2: %m\n", stderr);
		}
		return NULL;
	}
	return flag2;
}

const char *get_flag3(void) {
	int ret;
	static char flag3[FLAG_MAX_LEN];
	ret = ioctl(dev_flag_fd, IOCTL_GET_FLAG3, (unsigned long)flag3);
	if (ret == -1) {
		switch (errno) {
		case EPERM:
			fputs("Failed to get flag3: Not authorized\n", stderr);
			break;
		default:
			fprintf(stderr, "Failed to get flag3: %m\n", stderr);
		}
		return NULL;
	}
	return flag3;
}
