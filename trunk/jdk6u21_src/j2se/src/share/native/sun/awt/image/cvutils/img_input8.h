/*
 * @(#)img_input8.h	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Fetching category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can load 8-bit pixels from an array of bytes
 * where the data for pixel (srcX, srcY) is loaded from index
 * (srcOff + srcY * srcScan + srcX) in the array.
 */

#define DeclareInputVars					\
    pixptr srcP;

#define InitInput(srcBPP)						\
    img_check(srcBPP == 8)

#define SetInputRow(pixels, srcOff, srcScan, srcY, srcOY)		\
    srcP.vp = pixels;							\
    srcP.bp += srcOff + ((srcY-srcOY) * srcScan)

#define GetPixelInc()							\
    ((int) *srcP.bp++)

#define GetPixel(srcX)							\
    ((int) srcP.bp[srcX])

#define InputPixelInc(X)						\
    srcP.bp += X

#define VerifyPixelRange(pixel, mapsize)				\
    do {								\
	img_check(((unsigned int) pixel) <= 255);			\
	img_check(mapsize >= 256);					\
    } while (0)
