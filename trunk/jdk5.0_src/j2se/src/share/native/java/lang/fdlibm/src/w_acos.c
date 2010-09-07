
 /* @(#)w_acos.c	1.8 03/12/19           */
/*
 * @(#)w_acos.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrap_acos(x)
 */

#include "fdlibm.h"


#ifdef __STDC__
	double acos(double x)		/* wrapper acos */
#else
	double acos(x)			/* wrapper acos */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_acos(x);
#else
	double z;
	z = __ieee754_acos(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabs(x)>1.0) {
	        return __kernel_standard(x,x,1); /* acos(|x|>1) */
	} else
	    return z;
#endif
}
