#include <stdlib.h>
#include <stdint.h>
#include "libc.h"
#include "lock.h"
#include "fork_impl.h"

#define malloc __libc_malloc
#define calloc __libc_calloc
#define realloc undef
#define free undef

#define COUNT 32

static struct fl
{
	struct fl *next;
	void (*f[COUNT])(void *);
	void *a[COUNT];
} builtin, *head;

static int finished_atexit;
static int slot;
static volatile int lock[1];
volatile int *const __atexit_lockptr = lock;

void __funcs_on_exit()
{
	void (*func)(void *), *arg;
	LOCK(lock);
	for (; head; head=head->next, slot=COUNT) while(slot-->0) {
		func = head->f[slot];
		arg = head->a[slot];
		UNLOCK(lock);
		func(arg);
		LOCK(lock);
	}
	finished_atexit = 1;
	UNLOCK(lock);
}

void __cxa_finalize(void *dso)
{
}

int __cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	LOCK(lock);

	if (finished_atexit) {
		UNLOCK(lock);
		return -1;
	}

	
	if (!head) head = &builtin;

	
	if (slot==COUNT) {
		struct fl *new_fl = calloc(sizeof(struct fl), 1);
		if (!new_fl) {
			UNLOCK(lock);
			return -1;
		}
		new_fl->next = head;
		head = new_fl;
		slot = 0;
	}

	
	head->f[slot] = func;
	head->a[slot] = arg;
	slot++;

	UNLOCK(lock);
	return 0;
}

static void call(void *p)
{
	((void (*)(void))(uintptr_t)p)();
}

int atexit(void (*func)(void))
{
	return __cxa_atexit(call, (void *)(uintptr_t)func, 0);
}
