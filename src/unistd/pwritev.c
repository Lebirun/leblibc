#define _GNU_SOURCE
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include "syscall.h"

ssize_t pwritev(int fd, const struct iovec *iov, int count, off_t ofs)
{
	int result;

	if (ofs == -1) ofs--;
	result = __syscall_cp(SYS_pwritev2, fd, iov, count,
		(long)(ofs), (long)(ofs>>32), RWF_NOAPPEND);
	if (result != -EOPNOTSUPP && result != -ENOSYS)
		return __syscall_ret(result);
	if (fcntl(fd, F_GETFL) & O_APPEND)
		return __syscall_ret(-EOPNOTSUPP);
	return syscall_cp(SYS_pwritev, fd, iov, count,
		(long)(ofs), (long)(ofs>>32));
}
