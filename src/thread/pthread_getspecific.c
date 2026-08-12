#include "pthread_impl.h"
#include <threads.h>

static void *__pthread_getspecific(pthread_key_t k)
{
	struct pthread *self = __pthread_self();
	return __pthread_tsd_get(self, k);
}

weak_alias(__pthread_getspecific, pthread_getspecific);
weak_alias(__pthread_getspecific, tss_get);
