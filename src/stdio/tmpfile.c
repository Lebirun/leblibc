#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "stdio_impl.h"

FILE *tmpfile(void)
{
	char s[] = "/tmp/tmpfile_XXXXXX";
	int fd;
	FILE *f;
	for (;;) {
		__randname(s+13);
		fd = sys_open(s, O_RDWR|O_CREAT|O_EXCL, 0600);
		if (fd >= 0) {
#ifdef SYS_unlink
			__syscall(SYS_unlink, s);
#else
			__syscall(SYS_unlinkat, AT_FDCWD, s, 0);
#endif
			f = __fdopen(fd, "w+");
			if (!f) __syscall(SYS_close, fd);
			return f;
		}
		if (fd != -EEXIST) return 0;
	}
}
