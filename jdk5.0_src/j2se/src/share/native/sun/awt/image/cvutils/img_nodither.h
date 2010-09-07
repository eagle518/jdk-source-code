/*
 * @(#)img_nodither.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Encoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation performs no encoding of output pixels at all
 * and is only useful for output pixel formats which consist of 3
 * bytes per pixel containing the red, green and blue components.
 * Since the components are stored explicitly in their own memory
 * accesses, they do not need to be encoded into a single monolithic
 * pixel first.  The value of the "pixel" variable will be undefined
 * during the output stage of the generic scale loop if this file
 * is used.
 */

#define DeclareDitherVars

#define InitDither(cvdata, clrdata, dstTW)			\
    do {} while (0)

#define StartDitherLine(cvdata, dstX1, dstY)			\
    do {} while (0)

#define DitherPixel(dstX, dstY, pixel, red, green, blue) 	\
    do {} while (0)

#define DitherBufComplete(cvdata, dstX1)			\
    do {} while (0)
