
 /* @(#)e_gamma_r.c	1.8 03/12/19           */
/*
 * @(#)e_gamma_r.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* __ieee754_gamma_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function
 * with user provide pointer for the sign of Gamma(x).
 *
 * Method: See __ieee754_lgamma_r
 */

#include "fdlibm.h"

#ifdef __STDC__
	double __ieee754_gamma_r(double x, int *signgamp)
#else
	double __ieee754_gamma_r(x,signgamp)
	double x; int *signgamp;
#endif
{
	return __ieee754_lgamma_r(x,signgamp);
}
