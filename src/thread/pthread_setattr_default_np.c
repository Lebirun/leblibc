#define _GNU_SOURCE
#include "pthread_impl.h"
#include <string.h>

#define MAX(a,b) ((a)>(b) ? (a) : (b))

int pthread_setattr_default_np(const pthread_attr_t *attrp)
{
	pthread_attr_t tmp;
	pthread_attr_t zero;
	size_t stack;
	size_t guard;

	tmp = *attrp;
	zero = (pthread_attr_t){ 0 };
	tmp._a_stacksize = 0;
	tmp._a_guardsize = 0;
	if (memcmp(&tmp, &zero, sizeof tmp))
		return EINVAL;

	stack = attrp->_a_stacksize;
	guard = attrp->_a_guardsize;

	__inhibit_ptc();
	__default_stacksize = MAX(__default_stacksize, stack);
	__default_guardsize = MAX(__default_guardsize, guard);
	__release_ptc();

	return 0;
}

int pthread_getattr_default_np(pthread_attr_t *attrp)
{
	__acquire_ptc();
	*attrp = (pthread_attr_t) {
		._a_stacksize = __default_stacksize,
		._a_guardsize = __default_guardsize,
	};
	__release_ptc();
	return 0;
}
