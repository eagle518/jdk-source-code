/*
 * @(#)img_input32.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Fetching category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can load 32-bit pixels from an array of longs
 * where the data for pixel (srcX, srcY) is loaded from index
 * (srcOff + srcY * srcScan + srcX) in the array.
 */

#define DeclareInputVars					\
    pixptr srcP;

#define InitInput(srcBPP)						\
    img_check(srcBPP == 32)

#define SetInputRow(pixels, srcOff, srcScan, srcY, srcOY)		\
    srcP.vp = pixels;							\
    srcP.ip += srcOff + ((srcY-srcOY) * srcScan)

#define GetPixelInc()							\
    (*srcP.ip++)

#define GetPixel(srcX)							\
    (srcP.ip[srcX])

#define InputPixelInc(X)						\
    srcP.ip += X

#define VerifyPixelRange(pixel, mapsize)				\
    do {								\
	if (((unsigned int) pixel) >= mapsize) {			\
	    SignalError(0, JAVAPKG "ArrayIndexOutOfBoundsException", 0);\
	    return SCALEFAILURE;					\
	}								\
    } while (0)
