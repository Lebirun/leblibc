#include "pthread_impl.h"

void __lock(volatile int *l)
{
	int need_locks = libc.need_locks;
	if (!need_locks) return;
	
	int current = a_cas(l, 0, INT_MIN + 1);
	if (need_locks < 0) libc.need_locks = 0;
	if (!current) return;
	
	for (unsigned i = 0; i < 10; ++i) {
		if (current < 0) current -= INT_MIN + 1;
		
		int val = a_cas(l, current, INT_MIN + (current + 1));
		if (val == current) return;
		current = val;
	}
	
	current = a_fetch_add(l, 1) + 1;
	
	for (;;) {
		
		if (current < 0) {
			__futexwait(l, current, 1);
			current -= INT_MIN + 1;
		}
		
		int val = a_cas(l, current, INT_MIN + current);
		if (val == current) return;
		current = val;
	}
}

void __unlock(volatile int *l)
{
	
	if (l[0] < 0) {
		if (a_fetch_add(l, -(INT_MIN + 1)) != (INT_MIN + 1)) {
			__wake(l, 1, 1);
		}
	}
}
