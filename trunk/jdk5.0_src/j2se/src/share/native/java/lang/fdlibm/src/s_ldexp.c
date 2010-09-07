
 /* @(#)s_ldexp.c	1.8 03/12/19           */
/*
 * @(#)s_ldexp.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
	double ldexp(double value, int exp)
#else
	double ldexp(value, exp)
	double value; int exp;
#endif
{
	if(!finite(value)||value==0.0) return value;
	value = scalbn(value,exp);
	if(!finite(value)||value==0.0) errno = ERANGE;
	return value;
}
