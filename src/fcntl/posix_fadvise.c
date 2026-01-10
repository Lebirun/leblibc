#include <fcntl.h>
#include "syscall.h"

int posix_fadvise(int fd, off_t base, off_t len, int advice)
{
#if defined(SYSCALL_FADVISE_6_ARG)
	
	return -__syscall(SYS_fadvise, fd, advice,
		__SYSCALL_LL_E(base), __SYSCALL_LL_E(len));
#else
	return -__syscall(SYS_fadvise, fd, __SYSCALL_LL_O(base),
		__SYSCALL_LL_E(len), advice);
#endif
}
