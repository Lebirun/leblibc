#include <time.h>
#include "syscall.h"

int clock_getres(clockid_t clk, struct timespec *ts)
{
#ifdef SYS_clock_getres_time64
	
	if (SYS_clock_getres != SYS_clock_getres_time64) {
		long ts32[2];
		int r = __syscall(SYS_clock_getres, clk, ts32);
		if (!r && ts) {
			ts->tv_sec = ts32[0];
			ts->tv_nsec = ts32[1];
		}
		return __syscall_ret(r);
	}
#endif
	
	return syscall(SYS_clock_getres, clk, ts);
}
