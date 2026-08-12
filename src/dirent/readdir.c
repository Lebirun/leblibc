#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include "__dirent.h"
#include "syscall.h"

typedef char dirstream_buf_alignment_check[1-2*(int)(
	offsetof(struct __dirstream, buf) % sizeof(off_t))];

struct dirent *readdir(DIR *dir)
{
	struct dirent *de;
	char *resized;
	int len;
	
	if (dir->buf_pos >= dir->buf_end) {
		if (!dir->buf) {
			dir->buf_capacity = 512;
			dir->buf = malloc(dir->buf_capacity);
			if (!dir->buf) return 0;
		}
		for (;;) {
			len = __syscall(SYS_getdents, dir->fd, dir->buf,
			                dir->buf_capacity);
			if (len != -EINVAL) break;
			if (dir->buf_capacity > UINT_MAX / 2) {
				errno = EOVERFLOW;
				return 0;
			}
			resized = realloc(dir->buf, dir->buf_capacity * 2);
			if (!resized) return 0;
			dir->buf = resized;
			dir->buf_capacity *= 2;
		}
		if (len <= 0) {
			if (len < 0 && len != -ENOENT) errno = -len;
			return 0;
		}
		dir->buf_end = len;
		dir->buf_pos = 0;
	}
	de = (void *)(dir->buf + dir->buf_pos);
	dir->buf_pos += de->d_reclen;
	dir->tell = de->d_off;
	return de;
}
