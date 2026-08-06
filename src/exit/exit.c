#include <stdlib.h>
#include <stdint.h>
#include "libc.h"
#include "pthread_impl.h"
#include "atomic.h"
#include "syscall.h"

static void dummy()
{
}

weak_alias(dummy, __funcs_on_exit);
weak_alias(dummy, __stdio_exit);

extern weak hidden void (*const __fini_array_start)(void), (*const __fini_array_end)(void);
extern weak hidden void (*const __DTOR_LIST__)(void), (*const __DTOR_END__)(void);

static void libc_exit_fini(void)
{
	uintptr_t a = (uintptr_t)&__fini_array_end;
	void (*fn)(void);
	for (; a>(uintptr_t)&__fini_array_start; a-=sizeof(void(*)()))
		(*(void (**)())(a-sizeof(void(*)())))();
	a = (uintptr_t)&__DTOR_END__;
	for (; a>(uintptr_t)&__DTOR_LIST__; a-=sizeof(void(*)())) {
		fn = *(void (**)())(a-sizeof(void(*)()));
		if (fn && (uintptr_t)fn != (uintptr_t)-1)
			fn();
	}
}

weak_alias(libc_exit_fini, __libc_exit_fini);

_Noreturn void exit(int code)
{
	static volatile int exit_lock[1];
	int tid =  __pthread_self()->tid;
	int prev = a_cas(exit_lock, 0, tid);
	if (prev == tid) a_crash();
	else if (prev) for (;;) __sys_pause();

	__funcs_on_exit();
	__libc_exit_fini();
	__stdio_exit();
	_Exit(code);
}
