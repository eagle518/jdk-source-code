
 /* @(#)w_asin.c	1.10 10/03/23           */
/*
 * @(#)w_asin.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrapper asin(x)
 */


#include "fdlibm.h"


#ifdef __STDC__
	double asin(double x)		/* wrapper asin */
#else
	double asin(x)			/* wrapper asin */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_asin(x);
#else
	double z;
	z = __ieee754_asin(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabs(x)>1.0) {
	        return __kernel_standard(x,x,2); /* asin(|x|>1) */
	} else
	    return z;
#endif
}
