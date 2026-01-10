#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "pthread_impl.h"

int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
	int fd, cs, status = 0;
	char f[sizeof "/proc/self/task/%d/comm"];
	ssize_t r;

	if (len < 16) return ERANGE;

	if (thread == pthread_self())
		return prctl(PR_GET_NAME, (unsigned long)name, 0UL, 0UL, 0UL) ? errno : 0;

	snprintf(f, sizeof f, "/proc/self/task/%d/comm", thread->tid);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	if ((fd = open(f, O_RDONLY|O_CLOEXEC)) < 0 || (r = read(fd, name, len)) == -1) status = errno;
	else if (r > 0)
		name[r-1] = 0;
	else if (len)
		name[0] = 0;
	if (fd >= 0) close(fd);
	pthread_setcancelstate(cs, 0);
	return status;
}
