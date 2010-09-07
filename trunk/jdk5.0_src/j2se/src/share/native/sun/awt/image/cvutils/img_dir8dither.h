/*
 * @(#)img_dir8dither.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Encoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can encode the color information into 32-bit
 * output pixels directly by using shift amounts to specify which
 * bits of the 32-bit output pixel should contain the red, green,
 * and blue components.
 */

#define DeclareDitherVars						\
    int red_dither_shift, green_dither_shift, blue_dither_shift;

#define InitDither(cvdata, clrdata, dstTW)			\
    do {							\
	red_dither_shift = clrdata->rOff;			\
	green_dither_shift = clrdata->gOff;			\
	blue_dither_shift = clrdata->bOff;			\
    } while (0)

#define StartDitherLine(cvdata, dstX1, dstY)			\
    do {} while (0)

#define DitherPixel(dstX, dstY, pixel, red, green, blue) 	\
    do {							\
	pixel = ((red << red_dither_shift) |			\
		 (green << green_dither_shift) |		\
		 (blue << blue_dither_shift));			\
    } while (0)

#define DitherBufComplete(cvdata, dstX1)			\
    do {} while (0)
