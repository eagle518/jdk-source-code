/*
 * @(#)dtypes.h	1.20 03/12/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

/* FIXME: why can not we unify types? why we need another declaration of about the same types?
          Fixed ~ F16Dot16, Fract ~ fract, etc.  */
typedef tt_int32 Fract;
typedef tt_int32 Fixed;
typedef char *   Ptr;

#define fixed1  (  0x00010000L)
#define fract1  (  0x40000000L)

typedef tt_int32  scalerError;
enum {
    scalerError_NoError=0,    /* Normal Return */
    scalerError_Memory=1,     /* Insufficient Memory. */
    scalerError_ScanError=2,  /* Insufficient Memory. */
    scalerError_POINT_MIGRATION_ERR=3
};

/* Following modes are defined in TrueType specification.
   See description of SCANTYPE instruction for more details. 
   Note that only first 3 modes are defined in the Apple spec.
   Last 2 modes (smart dropout control were added by Microsoft. */
#define DOCONTROL_FULL           0x0
#define DOCONTROL_NOSTUBS        0x1
#define DOCONTROL_DISABLED       0x2
#define DOCONTROL_SMART          0x4
#define DOCONTROL_SMART_NOSTUBS  0x5

/* 0x01ff = enable dropout for all sizes
   see comment for getDropoutMode in t2k.c for definition of meaning of particular bits */
/* Note: shall we use smart dropout by default? This will be mostly applicable to Type1 fonts */
#define DEFAULT_DROPOUT_MODE       ((DOCONTROL_FULL << 16) | 0x01ff)

#endif /* _T2K_DTYPES__ */





