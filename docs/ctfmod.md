% CTFMOD(4)
% Torin Carey <torin@tcarey.uk>
% April 2019

# NAME

ctfmod - ioctls for ctfmod

# SYNOPSIS

**#include <ctfmod.h>**

# DESCRIPTION

The following **ioctl**(2) requests are supported by the ctfmod driver. Each request
except for **CTFMOD_CHECK_STATUS** a third argument, named *argp*.

**CTFMOD_GETFLAG1**
: Gets the first flag. *argp* is a pointer to a user defined char array of minimum
length **FLAG_LEN**. This call will fill *argp* with a nul-terminated ASCII string
which is the first flag.

**CTFMOD_GETFLAG2**
: Gets the second flag. *argp* is a pointer to a user defined char array of minimum
length **FLAG_LEN**. This call will fill *argp* with a nul-terminated ASCII string
which is the second flag. This call requires the authenticate command
**UNLOCK_FLAG2** and will fail with **EPERM** if this is not unlocked.

**CTFMOD_GETFLAG3**
: Gets the third flag. *argp* is a pointer to a user defined char array of minimum
length **FLAG_LEN**. This call will fill *argp* with a nul-terminated ASCII string
which is the third flag. This call requires the authenticate command
**UNLOCK_FLAG3** and will fail with **EPERM** if this is not unlocked.

**CTFMOD_AUTHENTICATE**
: Allows the process to unlock specific flags and functionality by providing a
nul-delimited list of commands. This list of commands are 'secured' using a basic
Message Authentication Code (MAC).
The type **struct auth_data** has the following form:

```
	struct auth_data {
		/* The SHA-256 hash of the 128-bit key
		 * concatenated with the message */
		unsigned char digest[32];

		/* The length in bytes of the message */
		unsigned int message_len;

		/* The null byte delimited list of messages.
		 * Unknown items in the list will be silently ignored.
		 * e.g. 'UNLOCK_FLAG1\0COMMAND_1\0COMMAND_2 */
		char message[128];
	};
```

**CTFMOD_LOAD_SECRETS**
: Used to load the flags and key into the ctfmod driver. Requires the process
to run as root or have the capability **CAP_SYS_ADMIN**.

	This call is required before the driver will accept any other **ioctl**(2) requests.
Other requests will fail with **EBUSY** if requested before this call has succeeded.
Attempting to make this request after the driver has already been initialised will
result in the error code **EBUSY**. To load a different set of secrets the module
needs to be manually unloaded and then loaded again.

	*Note*: The ctfmod driver initialisation is global and affects all
present and future open instances of the ctfmod driver.

	The argument *argp* points to a structure
**struct flag_key** defined as following:

```
	struct flag_key {
		char flag[3][FLAG_LEN];
		unsigned char key[16];
	};
```

**CTFMOD_CHECK_STATUS**
: Used to check whether or not the driver has been initialised with
**CTFMOD_LOAD_SECRETS**. Returns zero if initialised and the error code
**EBUSY** if not.

# ERRORS

All **ioctl**(2) requests (with the exception of **CTFMOD_LOAD_SECRETS**) will
return the error code **EBUSY** if the driver has not been initialised with
**CTFMOD_LOAD_SECRETS**.

The following error can be returned by the **CTFMOD_GET_FLAG\*** requests:

**EPERM**
: The process has not unlocked the requested flag with **CTFMOD_AUTHENTICATE**.

The following errors can be returned by **CTFMOD_AUTHENTICATE**:

**EFAULT**
: The user supplied *argp* cannot be fully read.

**EINVAL**
: The user supplied **struct auth_data** has an invalid *message_len*.

**EPERM**
: The MAC in **struct auth_data** is invalid.

The following errors can be returned by **CTFMOD_LOAD_SECRETS**:

**EBUSY**
: The ctfmod driver has already been initialised.

**EPERM**
: The calling process does not have an EUID of zero and does not have the
**CAP_SYS_ADMIN** capability.

**EINTR**
: The process received a signal while waiting.

**EFAULT**
: The user supplied *argp* cannot be fully read.

# EXAMPLE

An example use of obtaining flag2 with **getflag()** and **authenticate()**:

```
#include <ctfmod.h>

unsigned char mac[32] = {
	0x2d, 0x1e, 0xb1, 0x7f, 0x18, 0xbc, 0x65, 0x8d,
	0x09, 0xe4, 0xda, 0x5c, 0x6a, 0xf6, 0x33, 0xa6,
	0x39, 0xdb, 0xdc, 0xb3, 0xaf, 0xc2, 0x04, 0x91,
	0x1e, 0xce, 0x20, 0x1c, 0xb1, 0xe6, 0x69, 0xe5
};

char message[12] = "UNLOCK_FLAG2";

struct auth_data auth = {.digest = mac, .message = message,
	.message_len = 12};

int fd = open("/dev/flag", O_RDONLY);
if (fd == -1) {
	fprintf(stderr, "Error opening device: %m\n");
	return -1;
}

if (ioctl(fd, CTFMOD_AUTHENTICATE, &auth) == -1) {
	fprintf(stderr, "Error authenticating: %m\n");
	close(fd);
	return -1;
}

char flag[FLAG_LEN];

if (ioctl(fd, CTFMOD_GET_FLAG2, flag) == -1) {
	fprintf(stderr, "Error getting flag2: %m\n");
	close(fd);
	return -1;
}

close(fd);

printf("flag2: %s\n", flag);
```

# SEE ALSO

**ioctl**(2)
