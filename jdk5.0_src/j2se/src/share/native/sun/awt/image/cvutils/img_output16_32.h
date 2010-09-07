/*
 * @(#)img_output16_32.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Storing category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can store 16-bit or 32-bit pixels into an
 * array of shorts, or longs such that the pixel for (srcX, srcY)
 * is stored at index (srcOff + srcY * srcScan + srcX) in the array.
 */

#define DeclareOutputVars				\
    pixptr dstP;					\
    int dst32;

#define InitOutput(cvdata, clrdata, dstX, dstY)			\
    do {							\
	switch (clrdata->bitsperpixel) {			\
	case 16: dst32 = 1; break;				\
	case 32: dst32 = 2; break;				\
	default:						\
	    SignalError(0, JAVAPKG "InternalError",		\
			"unsupported screen depth");		\
	    return SCALEFAILURE;				\
	}							\
	img_check((ScanBytes(cvdata) & ((1 << dst32)-1)) == 0);	\
	dstP.vp = cvdata->outbuf;				\
	dstP.bp += dstY * ScanBytes(cvdata) + (dstX << dst32);	\
    } while (0)

#define PutPixelInc(pixel, red, green, blue)			\
    do {							\
	switch (dst32) {					\
	case 1:							\
	    *dstP.sp++ = ((unsigned short) pixel);		\
	    break;						\
	case 2:							\
	    *dstP.ip++ = pixel;					\
	    break;						\
	}							\
    } while (0)

#define EndOutputRow(cvdata, dstY, dstX1, dstX2)		\
    do {							\
	SendRow(cvdata, dstY, dstX1, dstX2);			\
	dstP.bp += (ScanBytes(cvdata)				\
		    - ((dstX2 - dstX1) << dst32));		\
    } while (0)

#define EndOutputRect(cvdata, dstX1, dstY1, dstX2, dstY2)	\
    SendBuffer(cvdata, dstX1, dstY1, dstX2, dstY2)
