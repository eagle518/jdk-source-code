
 /* @(#)e_gamma.c	1.8 03/12/19           */
/*
 * @(#)e_gamma.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* __ieee754_gamma(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_gamma_r
 */

#include "fdlibm.h"

extern int signgam;

#ifdef __STDC__
	double __ieee754_gamma(double x)
#else
	double __ieee754_gamma(x)
	double x;
#endif
{
	return __ieee754_gamma_r(x,&signgam);
}
