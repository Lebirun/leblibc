#include "pthread_impl.h"

int pthread_setspecific(pthread_key_t k, const void *x)
{
	struct pthread *self = __pthread_self();

	return __pthread_tsd_set(self, k, (void *)x);
}
