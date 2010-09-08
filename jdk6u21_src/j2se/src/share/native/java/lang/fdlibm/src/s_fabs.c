
 /* @(#)s_fabs.c	1.10 10/03/23           */
/*
 * @(#)s_fabs.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
