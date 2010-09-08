
 /* @(#)s_significand.c	1.10 10/03/23           */
/*
 * @(#)s_significand.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * significand(x) computes just
 * 	scalb(x, (double) -ilogb(x)),
 * for exercising the fraction-part(F) IEEE 754-1985 test vector.
 */

#include "fdlibm.h"

#ifdef __STDC__
	double significand(double x)
#else
	double significand(x)
	double x;
#endif
{
	return __ieee754_scalb(x,(double) -ilogb(x));
}
