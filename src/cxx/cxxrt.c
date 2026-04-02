#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *__dso_handle = 0;

void __cxa_pure_virtual(void)
{
	fprintf(stderr, "pure virtual method called\n");
	abort();
}

int __cxa_guard_acquire(long long *guard)
{
	char *byte;

	byte = (char *)guard;
	if (*byte) return 0;
	return 1;
}

void __cxa_guard_release(long long *guard)
{
	char *byte;

	byte = (char *)guard;
	*byte = 1;
}

void __cxa_guard_abort(long long *guard)
{
	(void)guard;
}

void __cxa_throw_bad_array_new_length(void)
{
	fprintf(stderr, "bad array new length\n");
	abort();
}

void *_Znwm(size_t size)
{
	void *p;

	if (size == 0) size = 1;
	p = malloc(size);
	if (!p) {
		fprintf(stderr, "operator new: allocation failed\n");
		abort();
	}
	return p;
}

void *_Znam(size_t size)
{
	return _Znwm(size);
}

void *_ZnwmRKSt9nothrow_t(size_t size, const void *nt)
{
	(void)nt;
	if (size == 0) size = 1;
	return malloc(size);
}

void *_ZnamRKSt9nothrow_t(size_t size, const void *nt)
{
	return _ZnwmRKSt9nothrow_t(size, nt);
}

void _ZdlPv(void *ptr)
{
	free(ptr);
}

void _ZdlPvm(void *ptr, size_t size)
{
	(void)size;
	free(ptr);
}

void _ZdaPv(void *ptr)
{
	free(ptr);
}

void _ZdaPvm(void *ptr, size_t size)
{
	(void)size;
	free(ptr);
}

void _ZdlPvRKSt9nothrow_t(void *ptr, const void *nt)
{
	(void)nt;
	free(ptr);
}

void _ZdaPvRKSt9nothrow_t(void *ptr, const void *nt)
{
	(void)nt;
	free(ptr);
}
