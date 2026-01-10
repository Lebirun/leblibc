#include <threads.h>
#include <pthread.h>

int tss_create(tss_t *tss, tss_dtor_t dtor)
{
	
	return __pthread_key_create(tss, dtor) ? thrd_error : thrd_success;
}
