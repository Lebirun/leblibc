#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"

char *tmpnam(char *buf)
{
	static char internal[L_tmpnam];
	char s[] = "/tmp/tmpnam_XXXXXX";
	char probe;
	int r;
	for (;;) {
		__randname(s+12);
#ifdef SYS_readlink
		r = __syscall(SYS_readlink, s, &probe, 1);
#else
		r = __syscall(SYS_readlinkat, AT_FDCWD, s, &probe, 1);
#endif
		if (r == -ENOENT) return strcpy(buf ? buf : internal, s);
		if (r < 0 && r != -EINVAL) return 0;
	}
}
