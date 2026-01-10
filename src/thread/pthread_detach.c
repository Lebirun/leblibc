#include "pthread_impl.h"
#include <threads.h>

static int __pthread_detach(pthread_t t)
{
	
	if (a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED) != DT_JOINABLE) {
		int cs;
		__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
		__pthread_join(t, 0);
		__pthread_setcancelstate(cs, 0);
	}
	return 0;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
