#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"

char *getcwd(char *buf, size_t size)
{
	char *tmp = NULL;
	int allocated = 0;
	
	if (!buf) {
		buf = malloc(PATH_MAX);
		if (!buf) {
			errno = ENOMEM;
			return NULL;
		}
		tmp = buf;
		size = PATH_MAX;
		allocated = 1;
	} else if (!size) {
		errno = EINVAL;
		return NULL;
	}
	
	long ret = syscall(SYS_getcwd, buf, size);
	if (ret < 0) {
		if (allocated) free(buf);
		return NULL;
	}
	
	if (ret == 0 || buf[0] != '/') {
		errno = ENOENT;
		if (allocated) free(buf);
		return NULL;
	}
	
	if (allocated) {
		char *result = strdup(buf);
		free(buf);
		return result;
	}
	
	return buf;
}
