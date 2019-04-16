#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl.h"
#include "flag.h"

int main(void) {
	int ret;
	int fd = open("/dev/flag", O_RDWR);
	if (fd == -1) {
		ret = errno;
		fprintf(stderr, "Failed to open device: %m\n");
		goto finish;
	}

	struct flag_key fk;
	memcpy(fk.flag[0], flag1, FLAG_LEN);
	memcpy(fk.flag[1], flag2, FLAG_LEN);
	memcpy(fk.flag[2], flag3, FLAG_LEN);
	memcpy(fk.key, key, 16);

	if (ioctl(fd, IOCTL_LOAD_SECRETS, &fk) == -1) {
		ret = errno;
		fprintf(stderr, "Failed to load secrets: %m\n");
		goto rel_dev;
	}
	ret = 0;
	printf("Success!\n");
rel_dev:
	close(fd);
finish:
	return ret;
}
