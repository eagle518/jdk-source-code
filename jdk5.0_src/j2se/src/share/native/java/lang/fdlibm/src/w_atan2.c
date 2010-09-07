
 /* @(#)w_atan2.c	1.8 03/12/19           */
/*
 * @(#)w_atan2.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrapper atan2(y,x)
 */

#include "fdlibm.h"


#ifdef __STDC__
	double atan2(double y, double x)	/* wrapper atan2 */
#else
	double atan2(y,x)			/* wrapper atan2 */
	double y,x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_atan2(y,x);
#else
	double z;
	z = __ieee754_atan2(y,x);
	if(_LIB_VERSION == _IEEE_||isnan(x)||isnan(y)) return z;
	if(x==0.0&&y==0.0) {
	        return __kernel_standard(y,x,3); /* atan2(+-0,+-0) */
	} else
	    return z;
#endif
}
