#include <math.h>

float sqrtf(float x)
{
	long double t;
	
	__asm__ ("fsqrt" : "=t"(t) : "0"(x));
	return (float)t;
}
