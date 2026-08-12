#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

#define malloc __libc_malloc
#define free __libc_free

char *__shm_mapname(const char *name)
{
	char *p;
	char *path;
	size_t length;

	while (*name == '/') name++;
	if (*(p = __strchrnul(name, '/')) || p==name ||
	    (p-name <= 2 && name[0]=='.' && p[-1]=='.')) {
		errno = EINVAL;
		return 0;
	}
	length = p-name;
	if (length > SIZE_MAX-10) {
		errno = ENAMETOOLONG;
		return 0;
	}
	path = malloc(length+10);
	if (!path) return 0;
	memcpy(path, "/dev/shm/", 9);
	memcpy(path+9, name, length+1);
	return path;
}

int shm_open(const char *name, int flag, mode_t mode)
{
	int cs;
	int fd;
	int saved_errno;
	char *path;

	path = __shm_mapname(name);
	if (!path) return -1;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	fd = open(path, flag|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK, mode);
	saved_errno = errno;
	free(path);
	pthread_setcancelstate(cs, 0);
	if (fd < 0) errno = saved_errno;
	return fd;
}

int shm_unlink(const char *name)
{
	char *path;
	int result;
	int saved_errno;

	path = __shm_mapname(name);
	if (!path) return -1;
	result = unlink(path);
	saved_errno = errno;
	free(path);
	if (result < 0) errno = saved_errno;
	return result;
}
