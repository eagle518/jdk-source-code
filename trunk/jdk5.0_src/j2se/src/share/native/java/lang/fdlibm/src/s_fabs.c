
 /* @(#)s_fabs.c	1.8 03/12/19           */
/*
 * @(#)s_fabs.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * fabs(x) returns the absolute value of x.
 */

#include "fdlibm.h"

#ifdef __STDC__
	double fabs(double x)
#else
	double fabs(x)
	double x;
#endif
{
	__HI(x) &= 0x7fffffff;
        return x;
}
