
 /* @(#)e_lgamma.c	1.10 10/03/23           */
/*
 * @(#)e_lgamma.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* __ieee754_lgamma(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgamma_r
 */

#include "fdlibm.h"

extern int signgam;

#ifdef __STDC__
	double __ieee754_lgamma(double x)
#else
	double __ieee754_lgamma(x)
	double x;
#endif
{
	return __ieee754_lgamma_r(x,&signgam);
}
