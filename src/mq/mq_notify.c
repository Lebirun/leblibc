#include <mqueue.h>
#include <errno.h>

int mq_notify(mqd_t mqd, const struct sigevent *sev)
{
	errno = ENOSYS;
	return -1;
}
