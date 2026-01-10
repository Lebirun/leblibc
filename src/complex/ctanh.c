
/*-
 * Copyright (c) 2011 David Schultz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "complex_impl.h"

double complex ctanh(double complex z)
{
	double x, y;
	double t, beta, s, rho, denom;
	uint32_t hx, ix, lx;

	x = creal(z);
	y = cimag(z);

	EXTRACT_WORDS(hx, lx, x);
	ix = hx & 0x7fffffff;

	
	if (ix >= 0x7ff00000) {
		if ((ix & 0xfffff) | lx)        
			return CMPLX(x, (y == 0 ? y : x * y));
		SET_HIGH_WORD(x, hx - 0x40000000);      
		return CMPLX(x, copysign(0, isinf(y) ? y : sin(y) * cos(y)));
	}

	
	if (!isfinite(y))
		return CMPLX(x ? y - y : x, y - y);

	
	if (ix >= 0x40360000) { 
		double exp_mx = exp(-fabs(x));
		return CMPLX(copysign(1, x), 4 * sin(y) * cos(y) * exp_mx * exp_mx);
	}

	
	t = tan(y);
	beta = 1.0 + t * t;     
	s = sinh(x);
	rho = sqrt(1 + s * s);  
	denom = 1 + beta * s * s;
	return CMPLX((beta * rho * s) / denom, t / denom);
}
