#include "pthread_impl.h"

int pthread_attr_setguardsize(pthread_attr_t *a, size_t size)
{
	a->_a_guardsize = size;
	return 0;
}
