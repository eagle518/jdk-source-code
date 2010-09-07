/*
 * @(#)dtypes.h	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * DTYPES.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#ifndef __T2K_DTYPES__
#define __T2K_DTYPES__

#include <sys/types.h>

#ifdef _LP64
#define tt_int32 	int
#define tt_uint32 	unsigned int
#else
#define tt_int32 	long
#define tt_uint32 	unsigned long
#endif
#define tt_int16 short
#define tt_uint16 unsigned short
#define tt_uint8 unsigned char
#define tt_int8 signed char

#ifndef _LP64
#define F26Dot6  long
#define F16Dot16 long
#else
#define F26Dot6  int
#define F16Dot16 int
#endif
#define Frac2Dot14 short    /* 2.14 number representing -2.0000ä+1.9999 */
#define Frac2Dot14FromFixed(x) ( (Frac2Dot14) (x>>2))
#define ONE16Dot16 0x10000
#define ONE2Dot14 0x4000

#define ONE16Dot16 0x10000


#ifndef false

#define false 0

#endif



#ifndef true

#define true 1

/* 64 bit stuff. This was added not sure if we need to do this. May need to go back to original long typedef's */
#endif

#ifndef _LP64
typedef long fixed;
typedef long fract;
#else
typedef tt_int32 fixed;
typedef tt_int32 fract;
#endif 

typedef struct {
	F16Dot16 t00, t01;
	F16Dot16 t10, t11;
} T2K_TRANS_MATRIX;


#endif /* _T2K_DTYPES__ */





