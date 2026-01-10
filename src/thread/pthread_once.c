#include "pthread_impl.h"

static void undo(void *control)
{
	
	if (a_swap(control, 0) == 3)
		__wake(control, -1, 1);
}

hidden int __pthread_once_full(pthread_once_t *control, void (*init)(void))
{
	

	for (;;) switch (a_cas(control, 0, 1)) {
	case 0:
		pthread_cleanup_push(undo, control);
		init();
		pthread_cleanup_pop(0);

		if (a_swap(control, 2) == 3)
			__wake(control, -1, 1);
		return 0;
	case 1:
		
		a_cas(control, 1, 3);
	case 3:
		__wait(control, 0, 3, 1);
		continue;
	case 2:
		return 0;
	}
}

int __pthread_once(pthread_once_t *control, void (*init)(void))
{
	
	if (*(volatile int *)control == 2) {
		a_barrier();
		return 0;
	}
	return __pthread_once_full(control, init);
}

weak_alias(__pthread_once, pthread_once);
