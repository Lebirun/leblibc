#include <mqueue.h>
#include <errno.h>
#include "syscall.h"

int mq_unlink(const char *name)
{
	errno = ENOSYS;
	return -1;
}
