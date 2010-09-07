/*
 * @(#)dtypes.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dtypes.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DTYPES_H
#define _DTYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

/* TBI: A comment of this form preceeds code fragments to be improved (thus, TBI)  */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef unsigned int 	u32;
typedef 	 int 	i32;
typedef unsigned short	u16;
typedef 	 short	i16;
typedef unsigned char	u8;
typedef 	 char	i8;
typedef 	 int	bool;
typedef 	 float	f32;
typedef		 double	f64;

#ifndef	MAXFLOAT
#define	MAXFLOAT	((f64)3.40282346638528860e+38)
#endif

typedef		 int	ixx;
#ifdef DUCTUS_C
#define	int	 /* to avoid casual use of int */
#endif

#ifndef TRUE
#define	TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#ifndef M_PI
#define M_PI    	3.14159265358979323846
#endif

#define POW2(e) 	(1<<(e))
#define pow2(e) 	POW2(e)
#define POW4(e) 	(POW2((e)<<1))
#define pow4(e) 	POW4(e)

#ifndef MAX
#define MAX(a,b)	(((a)>=(b))?(a):(b))
#endif
#ifndef max
#define max(a,b)	MAX(a,b)
#endif
#ifndef MIN
#define MIN(a,b)	(((a)<=(b))?(a):(b))
#endif
#ifndef min
#define min(a,b)	MIN(a,b)
#endif
#ifndef ABS
#define ABS(a)		(((a) < 0) ? -(a) : (a))
#endif
#ifndef ROUND
#define ROUND(a)	floor(((a) + 0.5))
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _DTYPES_H */
