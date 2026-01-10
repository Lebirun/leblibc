#define _GNU_SOURCE
#include <sched.h>
#include "syscall.h"
#include <errno.h>

int unshare(int flags)
{
	errno = ENOSYS;
	return -1;
}
