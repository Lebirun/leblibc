#include "libm.h"

double sqrt(double x)
{
	union ldshape ux;
	unsigned fpsr;
	__asm__ ("fsqrt; fnstsw %%ax": "=t"(ux.f), "=a"(fpsr) : "0"(x));
	if ((ux.i.m & 0x7ff) != 0x400)
		return (double)ux.f;
	
	ux.i.m ^= (fpsr & 0x200) + 0x300;
	return (double)ux.f;
}
