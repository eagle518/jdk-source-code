/*
 * @(#)FontMath.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 /*
  * @(#) FontMath.h: 6/21/99, preincluded header for all files.
 *
 * Copyright 1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 
	File:		FontMath.h

 	Copyright:	© 1990 by Apple Computer, Inc., all rights reserved.
 */

#ifndef fontMathIncludes
#define fontMathIncludes

#ifndef fontScalerDefinitionIncludes
#include "FSCdefs.h"
#endif

#ifndef trueTypeIncludes
#include "TruetypeTypes.h"
#endif

#define HIBITSET			0x80000000
#define POSINFINITY			0x7FFFFFFF
#define NEGINFINITY			0x80000000  

#define HIWORDMASK		0xffff0000
#define LOWORDMASK		0x0000ffff
#define ONESHORTFRAC		(1 << 14)

#define FIXROUND( x )		(tt_int16)(((x) + fixed1/2) >> 16)
#define ROUNDFIXED( x )		(((x) + fixed1/2) & HIWORDMASK)
#define DOT6TOFIX(n)		((n) << 10)
#define HIWORD(n)			((tt_uint16)((tt_uint32)(n) >> 16))
#define LOWORD(n)			((tt_uint16)(n))
#define LOWSIXBITS			63

#ifdef __cplusplus
extern "C" {
#endif

tt_int32 ShortFracMul(tt_int32 a, shortFrac b);
shortFrac ShortFracDot(shortFrac a, shortFrac b);
shortFrac ShortFracDivide(shortFrac a, shortFrac b);
tt_int32 ShortMulDiv(tt_int32 a, tt_int16 b, tt_int16 c);	/* (a*b)/c */
F26Dot6 Mul26Dot6(F26Dot6 a, F26Dot6 b);
F26Dot6 Div26Dot6(F26Dot6 a, F26Dot6 b);

#ifdef __cplusplus
}
#endif

#endif
