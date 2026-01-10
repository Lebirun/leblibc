#include <math.h>

float remainderf(float x, float y)
{
	unsigned short fpsr;
	
	do __asm__ ("fprem1; fnstsw %%ax" : "+t"(x), "=a"(fpsr) : "u"(y));
	while (fpsr & 0x400);
	return x;
}

weak_alias(remainderf, dremf);
