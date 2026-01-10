#include "pthread_impl.h"
#include <threads.h>

int mtx_lock(mtx_t *m)
{
	if (m->_m_type == PTHREAD_MUTEX_NORMAL && !a_cas(&m->_m_lock, 0, EBUSY))
		return thrd_success;
	
	return mtx_timedlock(m, 0);
}
