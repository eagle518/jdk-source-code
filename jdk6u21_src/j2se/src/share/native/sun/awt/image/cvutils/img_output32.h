/*
 * @(#)img_output32.h	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Storing category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can store 32-bit pixels into an array of
 * longs such that the pixel for (srcX, srcY) is stored at index
 * (srcOff + srcY * srcScan + srcX) in the array.
 */

#define DeclareOutputVars				\
    pixptr dstP;

#define InitOutput(cvdata, clrdata, dstX, dstY)			\
    do {							\
	img_check(clrdata->bitsperpixel == 32);			\
	img_check((ScanBytes(cvdata) & 3) == 0);		\
	dstP.vp = cvdata->outbuf;				\
	dstP.bp += dstY * ScanBytes(cvdata);			\
	dstP.ip += dstX;					\
    } while (0)

#define PutPixelInc(pixel, red, green, blue)			\
    *dstP.ip++ = pixel

#define EndOutputRow(cvdata, dstY, dstX1, dstX2)		\
    do {							\
	SendRow(cvdata, dstY, dstX1, dstX2);			\
	dstP.ip -= (dstX2 - dstX1);				\
	dstP.bp += ScanBytes(cvdata);				\
    } while (0)

#define EndOutputRect(cvdata, dstX1, dstY1, dstX2, dstY2)	\
    SendBuffer(cvdata, dstX1, dstY1, dstX2, dstY2)
