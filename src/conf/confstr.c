#include <unistd.h>
#include <stdio.h>
#include <errno.h>

size_t confstr(int name, char *buf, size_t len)
{
	const char *s = "";
	if (!name) {
		s = "/bin:/usr/bin";
	} else if ((name&~4U)!=1 && name-_CS_POSIX_V6_ILP32_OFF32_CFLAGS>35U) {
		errno = EINVAL;
		return 0;
	}
	
	
	return snprintf(buf, len, "%s", s) + 1;
}
