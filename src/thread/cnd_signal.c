#include <threads.h>
#include <pthread.h>

int cnd_signal(cnd_t *c)
{
	
	return __private_cond_signal((pthread_cond_t *)c, 1);
}
