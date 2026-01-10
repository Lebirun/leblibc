
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

float complex csqrtf(float complex z)
{
	float a = crealf(z), b = cimagf(z);
	double t;

	
	if (z == 0)
		return CMPLXF(0, b);
	if (isinf(b))
		return CMPLXF(INFINITY, b);
	if (isnan(a)) {
		t = (b - b) / (b - b);  
		return CMPLXF(a, t);  
	}
	if (isinf(a)) {
		
		if (signbit(a))
			return CMPLXF(fabsf(b - b), copysignf(a, b));
		else
			return CMPLXF(a, copysignf(b - b, b));
	}
	

	
	if (a >= 0) {
		t = sqrt((a + hypot(a, b)) * 0.5);
		return CMPLXF(t, b / (2.0 * t));
	} else {
		t = sqrt((-a + hypot(a, b)) * 0.5);
		return CMPLXF(fabsf(b) / (2.0 * t), copysignf(t, b));
	}
}
