#ifndef H_FLAGIOCTL
#define H_FLAGIOCTL

#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

struct auth_data {
	/* The SHA-256 hash of the 128-bit key concatenated with the message */
	unsigned char digest[32];
	/* The length in bytes of the message */
	unsigned int message_len;
	/* The null byte delimited list of messages. Unknown items in the list
	   will be silently ignored. e.g. 'UNLOCK_FLAG1\0COMMAND_1\0COMMAND_2 */
	char message[128];
};

#define FLAG_MAGIC 105

/* The maximum length of the null terminated string returned by the GET_FLAG ioctls */
#define FLAG_MAX_LEN 64

/* GET_FLAG2 and GET_FLAG3 will fail with EPERM if the appropriate flag hasn't been
 * unlocked with AUTHENTICATE */

/* Gets flag1. Does not require any unlocking */
#define IOCTL_GET_FLAG1 _IOR(FLAG_MAGIC, 16, char *)

/* Gets flag2. Requires 'UNLOCK_FLAG2' */
#define IOCTL_GET_FLAG2 _IOR(FLAG_MAGIC, 17, char *)

/* Gets flag3. Requires 'UNLOCK_FLAG3' */
#define IOCTL_GET_FLAG3 _IOR(FLAG_MAGIC, 18, char *)

/* Sends an authenticated message to the driver. Message contains zero or more
   instructions, such as 'UNLOCK_FLAG2'. Multiple calls to AUTHENTICATE are
   permitted and previously unlocked flags will still be unlocked. Unknown flags
   are silently ignored. */
#define IOCTL_AUTHENTICATE _IOW(FLAG_MAGIC, 8, struct auth_data *)

#endif // H_FLAGIOCTL
