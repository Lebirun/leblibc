#include <mqueue.h>
#include <fcntl.h>
#include <stdarg.h>
#include "syscall.h"
#include <errno.h>

mqd_t mq_open(const char *name, int flags, ...)
{
	errno = ENOSYS;
	return -1;
}
