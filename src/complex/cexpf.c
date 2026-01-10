
/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
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

#include "complex_impl.h"

static const uint32_t
exp_ovfl  = 0x42b17218,  
cexp_ovfl = 0x43400074;  

float complex cexpf(float complex z)
{
	float x, y, exp_x;
	uint32_t hx, hy;

	x = crealf(z);
	y = cimagf(z);

	GET_FLOAT_WORD(hy, y);
	hy &= 0x7fffffff;

	
	if (hy == 0)
		return CMPLXF(expf(x), y);
	GET_FLOAT_WORD(hx, x);
	
	if ((hx & 0x7fffffff) == 0)
		return CMPLXF(cosf(y), sinf(y));

	if (hy >= 0x7f800000) {
		if ((hx & 0x7fffffff) != 0x7f800000) {
			
			return CMPLXF(y - y, y - y);
		} else if (hx & 0x80000000) {
			
			return CMPLXF(0.0, 0.0);
		} else {
			
			return CMPLXF(x, y - y);
		}
	}

	if (hx >= exp_ovfl && hx <= cexp_ovfl) {
		
		return __ldexp_cexpf(z, 0);
	} else {
		
		exp_x = expf(x);
		return CMPLXF(exp_x * cosf(y), exp_x * sinf(y));
	}
}
