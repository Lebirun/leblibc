#include <threads.h>

int cnd_wait(cnd_t *c, mtx_t *m)
{
	
	return cnd_timedwait(c, m, 0);
}
