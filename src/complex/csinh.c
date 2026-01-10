
/*-
 * Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
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

static const double huge = 0x1p1023;

double complex csinh(double complex z)
{
	double x, y, h;
	int32_t hx, hy, ix, iy, lx, ly;

	x = creal(z);
	y = cimag(z);

	EXTRACT_WORDS(hx, lx, x);
	EXTRACT_WORDS(hy, ly, y);

	ix = 0x7fffffff & hx;
	iy = 0x7fffffff & hy;

	
	if (ix < 0x7ff00000 && iy < 0x7ff00000) {
		if ((iy | ly) == 0)
			return CMPLX(sinh(x), y);
		if (ix < 0x40360000)    
			return CMPLX(sinh(x) * cos(y), cosh(x) * sin(y));

		
		if (ix < 0x40862e42) {
			
			h = exp(fabs(x)) * 0.5;
			return CMPLX(copysign(h, x) * cos(y), h * sin(y));
		} else if (ix < 0x4096bbaa) {
			
			z = __ldexp_cexp(CMPLX(fabs(x), y), -1);
			return CMPLX(creal(z) * copysign(1, x), cimag(z));
		} else {
			
			h = huge * x;
			return CMPLX(h * cos(y), h * h * sin(y));
		}
	}

	
	if ((ix | lx) == 0 && iy >= 0x7ff00000)
		return CMPLX(copysign(0, x * (y - y)), y - y);

	
	if ((iy | ly) == 0 && ix >= 0x7ff00000) {
		if (((hx & 0xfffff) | lx) == 0)
			return CMPLX(x, y);
		return CMPLX(x, copysign(0, y));
	}

	
	if (ix < 0x7ff00000 && iy >= 0x7ff00000)
		return CMPLX(y - y, x * (y - y));

	
	if (ix >= 0x7ff00000 && ((hx & 0xfffff) | lx) == 0) {
		if (iy >= 0x7ff00000)
			return CMPLX(x * x, x * (y - y));
		return CMPLX(x * cos(y), INFINITY * sin(y));
	}

	
	return CMPLX((x * x) * (y - y), (x + x) * (y - y));
}
