#include "stdio_impl.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "libc.h"

FILE *__fdopen(int fd, const char *mode)
{
	FILE *f;
	struct winsize wsz;

	
	if (!strchr("rwa", *mode)) {
		errno = EINVAL;
		return 0;
	}

	
	if (!(f=malloc(sizeof *f + UNGET + BUFSIZ))) return 0;

	
	memset(f, 0, sizeof *f);

	
	if (!strchr(mode, '+')) f->flags = (*mode == 'r') ? F_NOWR : F_NORD;

	
	if (strchr(mode, 'e')) __syscall(SYS_fcntl, fd, F_SETFD, FD_CLOEXEC);

	
	if (*mode == 'a') {
		int flags = __syscall(SYS_fcntl, fd, F_GETFL);
		if (!(flags & O_APPEND))
			__syscall(SYS_fcntl, fd, F_SETFL, flags | O_APPEND);
		f->flags |= F_APP;
	}

	f->fd = fd;
	f->buf = (unsigned char *)f + sizeof *f + UNGET;
	f->buf_size = BUFSIZ;

	
	f->lbf = EOF;
	if (!(f->flags & F_NOWR) && !__syscall(SYS_ioctl, fd, TIOCGWINSZ, &wsz))
		f->lbf = '\n';

	
	f->read = __stdio_read;
	f->write = __stdio_write;
	f->seek = __stdio_seek;
	f->close = __stdio_close;

	if (!libc.threaded) f->lock = -1;

	
	return __ofl_add(f);
}

weak_alias(__fdopen, fdopen);
