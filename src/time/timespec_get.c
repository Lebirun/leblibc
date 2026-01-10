#include <time.h>

int timespec_get(struct timespec * ts, int base)
{
	if (base != TIME_UTC) return 0;
	int ret = __clock_gettime(CLOCK_REALTIME, ts);
	return ret < 0 ? 0 : base;
}
