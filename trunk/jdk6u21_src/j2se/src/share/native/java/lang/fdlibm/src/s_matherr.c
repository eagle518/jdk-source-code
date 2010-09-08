
 /* @(#)s_matherr.c	1.10 10/03/23           */
/*
 * @(#)s_matherr.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "fdlibm.h"

#ifdef __STDC__
	int matherr(struct exception *x)
#else
	int matherr(x)
	struct exception *x;
#endif
{
	int n=0;
	if(x->arg1!=x->arg1) return 0;
	return n;
}
