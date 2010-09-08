/*
 * @(#)HintScan.h	1.7 03/12/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/*	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

#ifndef	HintScanIncludes
#define	HintScanIncludes

#include "dtypes.h"
#include "t2ksc.h"
#include "sc.h"

typedef struct {
  F16Dot16 x;
  F16Dot16 y;
} Position;

typedef struct {
  F16Dot16 xMin; 
  F16Dot16 yMin; 
  F16Dot16 xMax; 
  F16Dot16 yMax; 
} Bounds;

typedef struct {
  char*     image;            /* Image bit data  */
  tt_uint16 rowBytes;         /* bytes per row (divisible by 8 ?) */
  Bounds    bounds;           /* limit of used area. */
  Position  topLeft;
  tt_uint32 actualBitmapSize ;
  tt_uint32 scanConvertScratchSize;
  tt_uint32 dropoutScratchSize;
} scalerBitmap;

 /* Prototypes for the lower-level gxFont scaler routines */
tt_int32 fs_ContourScan3(tsiScanConv *t, sc_BitMapData *bitRecPtr, 
                         scalerBitmap* glyphImage, tt_int32 dropoutMode);
tt_int32 fs_CalculateBounds(sc_BitMapData *bbox, register tsiScanConv *t, 
                            scalerBitmap *glyphImage);
void fs_FindBitMapSize4(sc_BitMapData *bitRecPtr, tsiScanConv *t, 
                        scalerBitmap *glyphImage, tt_int32 dropoutMode);

#endif
