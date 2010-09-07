/*
 * @(#)ScalerMatrix.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
**
**	
*/
/*	scaler matrix.h
**
**	Routines to decompose a matrix  into stretch and remainder
**	Useful for picking the best possible size strike.
**	This code is shared by the trueType, sbit, and NFNT scalers.
**
**	Copyright:	© 1992, 1993, 1994 by Apple Computer, Inc., all rights reserved.
 */
 
#ifndef scalerMatrixIncludes
#define scalerMatrixIncludes

#include "FSglue.h"

#define RoundedFixed(x)	((x) + 0x8000 & 0xffff0000)

boolean NonInvertibleMapping(const gxMapping* map);
boolean DecomposeMapping(transformState* state, boolean useIntegerScaling);
fixed ComputeMappingStretch(const gxMapping* map, fixed* yStretch);
gxMapping *InvertRemainder( gxMapping* inverse, const gxMapping* remainder );

#endif
