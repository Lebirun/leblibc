
/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <fenv.h>
#include <math.h>
#include <stdint.h>

float fmaf(float x, float y, float z)
{
	#pragma STDC FENV_ACCESS ON
	double xy, result;
	union {double f; uint64_t i;} u;
	int e;

	xy = (double)x * y;
	result = xy + z;
	u.f = result;
	e = u.i>>52 & 0x7ff;
	
	if ((u.i & 0x1fffffff) != 0x10000000 || 
		e == 0x7ff ||                   
		(result - xy == z && result - z == xy) || 
		fegetround() != FE_TONEAREST)       
	{
		
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
		if (e < 0x3ff-126 && e >= 0x3ff-149 && fetestexcept(FE_INEXACT)) {
			feclearexcept(FE_INEXACT);
			
			volatile float vz = z;
			result = xy + vz;
			if (fetestexcept(FE_INEXACT))
				feraiseexcept(FE_UNDERFLOW);
			else
				feraiseexcept(FE_INEXACT);
		}
#endif
		z = result;
		return z;
	}

	
	double err;
	int neg = u.i >> 63;
	if (neg == (z > xy))
		err = xy - result + z;
	else
		err = z - result + xy;
	if (neg == (err < 0))
		u.i++;
	else
		u.i--;
	z = u.f;
	return z;
}
