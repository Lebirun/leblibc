#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int fchdir(int fd)
{
	int ret = __syscall(SYS_fchdir, fd);
	if (ret != -EBADF || __syscall(SYS_fcntl, fd, F_GETFD) < 0)
		return __syscall_ret(ret);

	char buf[15+3*sizeof(int)];
	__procfdname(buf, fd);
	int dfd = __sys_open(buf, O_RDONLY | O_DIRECTORY, 0);
	if (dfd < 0) { errno = -dfd; return -1; }
	ret = __syscall(SYS_fchdir, dfd);
	__syscall(SYS_close, dfd);
	if (ret < 0) { errno = -ret; return -1; }
	return 0;
}
