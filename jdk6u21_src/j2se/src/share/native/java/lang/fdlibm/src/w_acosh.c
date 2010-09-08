
 /* @(#)w_acosh.c	1.10 10/03/23           */
/*
 * @(#)w_acosh.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrapper acosh(x)
 */

#include "fdlibm.h"

#ifdef __STDC__
	double acosh(double x)		/* wrapper acosh */
#else
	double acosh(x)			/* wrapper acosh */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_acosh(x);
#else
	double z;
	z = __ieee754_acosh(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(x<1.0) {
	        return __kernel_standard(x,x,29); /* acosh(x<1) */
	} else
	    return z;
#endif
}
