#define _GNU_SOURCE
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "syscall.h"

ssize_t pwrite(int fd, const void *buf, size_t size, off_t ofs)
{
	int result;

	if (ofs == -1) ofs--;
	result = __syscall_cp(SYS_pwritev2, fd,
		(&(struct iovec){ .iov_base = (void *)buf, .iov_len = size }),
		1, (long)(ofs), (long)(ofs>>32), RWF_NOAPPEND);
	if (result != -EOPNOTSUPP && result != -ENOSYS)
		return __syscall_ret(result);
	if (fcntl(fd, F_GETFL) & O_APPEND)
		return __syscall_ret(-EOPNOTSUPP);
	return syscall_cp(SYS_pwrite, fd, buf, size, __SYSCALL_LL_PRW(ofs));
}
