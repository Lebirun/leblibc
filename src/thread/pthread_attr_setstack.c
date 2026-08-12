#include "pthread_impl.h"

int pthread_attr_setstack(pthread_attr_t *a, void *addr, size_t size)
{
	size_t address;

	if (size < PTHREAD_STACK_MIN) return EINVAL;
	address = (size_t)addr;
	if (address > SIZE_MAX-size) return EINVAL;
	a->_a_stackaddr = address + size;
	a->_a_stacksize = size;
	return 0;
}
