/*
 * @(#)img_input8_32.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Fetching category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can load either 8-bit or 32-bit pixels from an
 * array of bytes or longs where the data for pixel (srcX, srcY) is
 * loaded from index (srcOff + srcY * srcScan + srcX) in the array.
 *
 * This file can be used to provide the default implementation of the
 * Fetching macros to handle all input sizes.
 */

#define DeclareInputVars					\
    pixptr srcP;						\
    int src32;

#define InitInput(srcBPP)						\
    do {								\
	switch (srcBPP) {						\
	case 8: src32 = 0; break;					\
	case 32: src32 = 1; break;					\
	default:							\
	    SignalError(0, JAVAPKG "InternalError",			\
			"unsupported source depth");			\
	    return SCALEFAILURE;					\
	}								\
    } while (0)

#define SetInputRow(pixels, srcOff, srcScan, srcY, srcOY)		\
    do {								\
	srcP.vp = pixels;						\
	if (src32) {							\
	    srcP.ip += srcOff + ((srcY-srcOY) * srcScan);		\
	} else {							\
	    srcP.bp += srcOff + ((srcY-srcOY) * srcScan);		\
	}								\
    } while (0)

#define GetPixelInc()							\
    (src32 ? *srcP.ip++ : ((int) *srcP.bp++))

#define GetPixel(srcX)							\
    (src32 ? srcP.ip[srcX] : ((int) srcP.bp[srcX]))

#define InputPixelInc(X)						\
    do {								\
	if (src32) {							\
	    srcP.ip += X;						\
	} else {							\
	    srcP.bp += X;						\
	}								\
    } while (0)

#define VerifyPixelRange(pixel, mapsize)				\
    do {								\
	if (((unsigned int) pixel) >= mapsize) {			\
	    SignalError(0, JAVAPKG "ArrayIndexOutOfBoundsException", 0);\
	    return SCALEFAILURE;					\
	}								\
    } while (0)
