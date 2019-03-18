#ifndef H_FLAGIOCTL
#define H_FLAGIOCTL

#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

struct auth_data {
	// The Message Authentication Code. The SHA-256 hash of the 128-bit key concatenated with the message
	unsigned char digest[32];
	// Length of data in message
	unsigned int message_len;
	// Null delimited list of commands. Unknown commands will be silently ignored
	// e.g. 'COMMAND1\0COMMAND2\0UNLOCK_FLAG1'
	char message[128];
};

#define MYMAGIC 105

// I promise the null terminated flags will not be longer than this
#define FLAG_MAX_LEN 64

/* Both GET_FLAGs will fail wuth EPERM if the appropriate flag hasn't been
 * unlocked with AUTHENTICATE */

// Requires UNLOCK_FLAG1
#define IOCTL_GET_FLAG1 _IOR(MYMAGIC, 16, char *)

// Requires UNLOCK_FLAG2
#define IOCTL_GET_FLAG2 _IOR(MYMAGIC, 17, char *)

/* AUTHENTICATE will fail with EPERM if the message doesn't have
 * a valid MAC */
#define IOCTL_AUTHENTICATE _IOW(MYMAGIC, 8, struct auth_data *)

#endif // H_FLAGIOCTL
