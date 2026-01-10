#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include "syscall.h"

pid_t vfork(void)
{
	
#ifdef SYS_fork
	return syscall(SYS_fork);
#else
	return syscall(SYS_clone, SIGCHLD, 0);
#endif
}
