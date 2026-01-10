#include <mqueue.h>
#include "syscall.h"
#include <errno.h>

int mq_setattr(mqd_t mqd, const struct mq_attr *restrict new, struct mq_attr *restrict old)
{
	errno = ENOSYS;
	return -1;
}
