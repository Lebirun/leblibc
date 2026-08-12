#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "syscall.h"

char *tempnam(const char *dir, const char *pfx)
{
	char *s;
	char probe;
	size_t l, dl, pl;
	int r;

	if (!dir) dir = P_tmpdir;
	if (!pfx) pfx = "temp";

	dl = strlen(dir);
	pl = strlen(pfx);
	if (dl > SIZE_MAX - pl - 9) {
		errno = ENAMETOOLONG;
		return 0;
	}
	l = dl + pl + 8;
	s = malloc(l + 1);
	if (!s) return 0;

	memcpy(s, dir, dl);
	s[dl] = '/';
	memcpy(s+dl+1, pfx, pl);
	s[dl+1+pl] = '_';
	s[l] = 0;

	for (;;) {
		__randname(s+l-6);
#ifdef SYS_readlink
		r = __syscall(SYS_readlink, s, &probe, 1);
#else
		r = __syscall(SYS_readlinkat, AT_FDCWD, s, &probe, 1);
#endif
		if (r == -ENOENT) return s;
		if (r < 0 && r != -EINVAL) {
			free(s);
			return 0;
		}
	}
}
