
/*-
 * Copyright (c) 2007 David Schultz <das@FreeBSD.ORG>
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

#pragma STDC CX_LIMITED_RANGE ON

#define THRESH  0x1.a827999fcef32p+1022

double complex csqrt(double complex z)
{
	double complex result;
	double a, b;
	double t;
	int scale;

	a = creal(z);
	b = cimag(z);

	
	if (z == 0)
		return CMPLX(0, b);
	if (isinf(b))
		return CMPLX(INFINITY, b);
	if (isnan(a)) {
		t = (b - b) / (b - b);  
		return CMPLX(a, t);   
	}
	if (isinf(a)) {
		
		if (signbit(a))
			return CMPLX(fabs(b - b), copysign(a, b));
		else
			return CMPLX(a, copysign(b - b, b));
	}
	

	
	if (fabs(a) >= THRESH || fabs(b) >= THRESH) {
		a *= 0.25;
		b *= 0.25;
		scale = 1;
	} else {
		scale = 0;
	}

	
	if (a >= 0) {
		t = sqrt((a + hypot(a, b)) * 0.5);
		result = CMPLX(t, b / (2 * t));
	} else {
		t = sqrt((-a + hypot(a, b)) * 0.5);
		result = CMPLX(fabs(b) / (2 * t), copysign(t, b));
	}

	
	if (scale)
		result *= 2;
	return result;
}
