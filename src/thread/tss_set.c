#include "pthread_impl.h"
#include <threads.h>

int tss_set(tss_t k, void *x)
{
	struct pthread *self = __pthread_self();
	int result;

	result = __pthread_tsd_set(self, k, x);
	return result ? thrd_error : thrd_success;
}
