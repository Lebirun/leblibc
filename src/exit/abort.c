#include <stdlib.h>
#include <signal.h>
#include "syscall.h"
#include "pthread_impl.h"
#include "atomic.h"
#include "lock.h"
#include "ksigaction.h"

_Noreturn void abort(void)
{
	raise(SIGABRT);

	
	__block_all_sigs(0);
	LOCK(__abort_lock);
	__syscall(SYS_rt_sigaction, SIGABRT,
		&(struct k_sigaction){.handler = SIG_DFL}, 0, _NSIG/8);
	__syscall(SYS_tkill, __pthread_self()->tid, SIGABRT);
	__syscall(SYS_rt_sigprocmask, SIG_UNBLOCK,
		&(long[_NSIG/(8*sizeof(long))]){1UL<<(SIGABRT-1)}, 0, _NSIG/8);

	
	a_crash();
	raise(SIGKILL);
	_Exit(127);
}
