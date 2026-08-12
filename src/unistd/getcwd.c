#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "syscall.h"

char *getcwd(char *buf, size_t size)
{
	char *grown;
	int allocated = 0;
	long ret;
	
	if (!buf) {
		if (!size) size = 128;
		buf = malloc(size);
		if (!buf) {
			errno = ENOMEM;
			return NULL;
		}
		allocated = 1;
	} else if (!size) {
		errno = EINVAL;
		return NULL;
	}
	
	retry:
	ret = syscall(SYS_getcwd, buf, size);
	if (ret < 0) {
		if (allocated && errno == ERANGE && size <= SIZE_MAX/2) {
			size *= 2;
			grown = realloc(buf, size);
			if (!grown) {
				free(buf);
				return NULL;
			}
			buf = grown;
			goto retry;
		}
		if (allocated) free(buf);
		return NULL;
	}
	
	if (ret == 0 || buf[0] != '/') {
		errno = ENOENT;
		if (allocated) free(buf);
		return NULL;
	}
	
	if (allocated) {
		char *result;

		result = strdup(buf);
		free(buf);
		return result;
	}
	
	return buf;
}
