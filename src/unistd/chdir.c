#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int chdir(const char *path)
{
	int fd = __sys_open(path, O_RDONLY | O_DIRECTORY, 0);
	if (fd < 0) { errno = -fd; return -1; }
	int ret = __syscall(SYS_fchdir, fd);
	__syscall(SYS_close, fd);
	if (ret < 0) { errno = -ret; return -1; }
	return 0;
}
