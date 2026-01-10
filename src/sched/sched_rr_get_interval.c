#include <sched.h>
#include "syscall.h"

int sched_rr_get_interval(pid_t pid, struct timespec *ts)
{
#ifdef SYS_sched_rr_get_interval_time64
	
	if (SYS_sched_rr_get_interval != SYS_sched_rr_get_interval_time64) {
		long ts32[2];
		int r = __syscall(SYS_sched_rr_get_interval, pid, ts32);
		if (!r) {
			ts->tv_sec = ts32[0];
			ts->tv_nsec = ts32[1];
		}
		return __syscall_ret(r);
	}
#endif
	
	return syscall(SYS_sched_rr_get_interval, pid, ts);
}
