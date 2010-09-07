/*
 * @(#)util.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * UTIL.H
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
#ifndef __T2K_UTIL__
#define __T2K_UTIL__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

unsigned char *ReadFileIntoMemory( tsiMemObject *mem, const char *fname, tt_uint32 *size );
void WriteDataToFile( const char *fname, void *dataIn, tt_uint32 size );

void util_SortShortArray( short *a, tt_int32 n );
/* Return mA * mB */
F16Dot16 util_FixMul( F16Dot16 mA, F16Dot16 mB );
/* Return mA / mB */
F16Dot16 util_FixDiv( F16Dot16 mA, F16Dot16 mB );
/* Returns sin( in * pi/180.0 ) valid when in belongs to [0,90] */
F16Dot16 util_FixSin( F16Dot16 in );
/* Returns cos( in * pi/180.0 ) valid when in belongs to [0,90] */
#define util_FixCos( in ) util_FixSin( (90*0x10000 - (in)) )
void TESTFIXDIV( void );

/*
 * Description:		returns sqrt( A*A + B*B );
 * How used:		Call with the data (dx, and dy).
 * Side Effects: 	None.
 * Return value: 	sqrt( A*A + B*B ).
 */
F16Dot16 util_EuclidianDistance( register F16Dot16 A, register F16Dot16 B );
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_UTIL__ */

