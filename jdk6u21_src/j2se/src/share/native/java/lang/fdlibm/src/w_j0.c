
 /* @(#)w_j0.c	1.10 10/03/23           */
/*
 * @(#)w_j0.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * wrapper j0(double x), y0(double x)
 */

#include "fdlibm.h"

#ifdef __STDC__
	double j0(double x)		/* wrapper j0 */
#else
	double j0(x)			/* wrapper j0 */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_j0(x);
#else
	double z = __ieee754_j0(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabs(x)>X_TLOSS) {
	        return __kernel_standard(x,x,34); /* j0(|x|>X_TLOSS) */
	} else
	    return z;
#endif
}

#ifdef __STDC__
	double y0(double x)		/* wrapper y0 */
#else
	double y0(x)			/* wrapper y0 */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_y0(x);
#else
	double z;
	z = __ieee754_y0(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x) ) return z;
        if(x <= 0.0){
                if(x==0.0)
                    /* d= -one/(x-x); */
                    return __kernel_standard(x,x,8);
                else
                    /* d = zero/(x-x); */
                    return __kernel_standard(x,x,9);
        }
	if(x>X_TLOSS) {
	        return __kernel_standard(x,x,35); /* y0(x>X_TLOSS) */
	} else
	    return z;
#endif
}
