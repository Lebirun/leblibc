#include <threads.h>
#include <pthread.h>

int mtx_unlock(mtx_t *mtx)
{
	
	return __pthread_mutex_unlock((pthread_mutex_t *)mtx);
}
