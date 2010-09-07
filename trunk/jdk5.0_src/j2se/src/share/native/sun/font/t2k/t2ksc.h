/*
 * @(#)t2ksc.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2KSC.H
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


#ifndef __T2K_SC__
#define __T2K_SC__

#include "dtypes.h"

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


#define POS_XEDGE 	0x01
#define NEG_XEDGE 	0x02
#define POS_YEDGE 	0x04
#define NEG_YEDGE 	0x08
#define DELETE_EDGE	0x10

#define IS_XEDGE (POS_XEDGE | NEG_XEDGE)
#define IS_YEDGE (POS_YEDGE | NEG_YEDGE)

typedef struct {
	/* public */	
	
	tt_int32 left, right, top, bottom;
	F26Dot6 fTop26Dot6, fLeft26Dot6;
	tt_int32 rowBytes;
	unsigned char *baseAddr;

	/* semi private */
	tt_int32 *xEdge;
	tt_int32 *yEdge;
	char *edgeData; /* isXEdge */
	tt_int32 numEdges;
	
	
	/* private */
	tt_int32 xmin, xmax;
	tt_int32 ymin, ymax;
	
	tt_int32 maxEdges;
	short gMul;

	short *startPoint;
	short *endPoint;
	short numberOfContours;

	tt_int32 *x;
	tt_int32 *y;
	char *onCurve;

	tsiMemObject *mem;
} tsiScanConv;

tsiScanConv *tsi_NewScanConv( tsiMemObject *mem, short numberOfContours, short *startPtr, short *endPtr, tt_int32 *xPtr, tt_int32 *yPtr, char *onCurvePtr, char greyScaleLevel, char curveType );
void MakeBits( tsiScanConv *t, char greyScaleLevel, char xWeightIsOne, char omitBitMap, char computeBBox, short isDropout );

void tsi_DeleteScanConv( tsiScanConv *t );


#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_SC__ */
