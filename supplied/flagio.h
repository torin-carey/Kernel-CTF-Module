#ifndef H_FLAGIO
#define H_FLAGIO

#include "ioctl.h"

int flag_open(void);
void flag_close(void);

int flag_auth(const struct auth_data *auth);
const char *get_flag1(void);
const char *get_flag2(void);

#endif // H_FLAGIO
