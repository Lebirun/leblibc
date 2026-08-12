#include <stdlib.h>
#include "libc.h"
#include "lock.h"
#include "fork_impl.h"

struct quick_exit_entry {
	struct quick_exit_entry *next;
	void (*func)(void);
};

static struct quick_exit_entry *funcs;
static volatile int lock[1];
volatile int *const __at_quick_exit_lockptr = lock;

void __funcs_on_quick_exit()
{
	void (*func)(void);
	struct quick_exit_entry *entry;
	LOCK(lock);
	while (funcs) {
		entry = funcs;
		funcs = entry->next;
		func = entry->func;
		UNLOCK(lock);
		func();
		free(entry);
		LOCK(lock);
	}
}

int at_quick_exit(void (*func)(void))
{
	struct quick_exit_entry *entry;

	entry = malloc(sizeof(*entry));
	if (!entry)
		return -1;
	entry->func = func;
	LOCK(lock);
	entry->next = funcs;
	funcs = entry;
	UNLOCK(lock);
	return 0;
}
