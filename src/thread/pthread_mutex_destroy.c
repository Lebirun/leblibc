#include "pthread_impl.h"

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	
	if (mutex->_m_type > 128) __vm_wait();
	return 0;
}
