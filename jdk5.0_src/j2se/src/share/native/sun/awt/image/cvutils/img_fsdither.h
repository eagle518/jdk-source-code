/*
 * @(#)img_fsdither.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Encoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation uses a Floyd-Steinberg error diffusion technique
 * to produce a very high quality version of an image with only an 8-bit
 * (or less) RGB colormap or gray ramp.  The error diffusion technique
 * requires that the input color information be delivered in a special
 * order from the top row to the bottom row and then left to right within
 * each row, thus it is only valid in cases where the ImageProducer has
 * specified the TopDownLeftRight delivery hint.  If the data is not read
 * in that order, no mathematical or memory access errors should occur,
 * but the dithering error will be spread through the pixels of the output
 * image in an unpleasant manner.
 */

/*
 * These definitions vector the standard macro names to the "Any"
 * versions of those macros.  The "DitherDeclared" keyword is also
 * defined to indicate to the other include files that they are not
 * defining the primary implementation.  All other include files
 * will check for the existance of the "DitherDeclared" keyword
 * and define their implementations of the Encoding macros using
 * more specific names without overriding the standard names.  This
 * is done so that the other files can be included here to reuse
 * their implementations for the specific cases.
 */
#define DitherDeclared
#define DeclareDitherVars	DeclareAnyDitherVars
#define InitDither		InitAnyDither
#define StartDitherLine		StartAnyDitherLine
#define DitherPixel		AnyDitherPixel
#define DitherBufComplete	AnyDitherBufComplete

/* Include the specific implementations for color and grayscale displays */
#include "img_fscolor.h"
#include "img_fsgray.h"

#define DeclareAnyDitherVars					\
    DeclareColorDitherVars					\
    DeclareGrayDitherVars					\
    int grayscale;

#define InitAnyDither(cvdata, clrdata, dstTW)				\
    do {								\
	if (grayscale = clrdata->grayscale) {				\
	    InitGrayDither(cvdata, clrdata, dstTW);			\
	} else {							\
	    InitColorDither(cvdata, clrdata, dstTW);			\
	}								\
    } while (0)

#define StartAnyDitherLine(cvdata, dstX1, dstY)				\
    do {								\
	if (grayscale) {						\
	    StartGrayDitherLine(cvdata, dstX1, dstY);			\
	} else {							\
	    StartColorDitherLine(cvdata, dstX1, dstY);			\
	}								\
    } while (0)

#define AnyDitherPixel(dstX, dstY, pixel, red, green, blue) 		\
    do {								\
	if (grayscale) {						\
	    GrayDitherPixel(dstX, dstY, pixel, red, green, blue);	\
	} else {							\
	    ColorDitherPixel(dstX, dstY, pixel, red, green, blue);	\
	}								\
    } while (0)

#define AnyDitherBufComplete(cvdata, dstX1)				\
    do {								\
	if (grayscale) {						\
	    GrayDitherBufComplete(cvdata, dstX1);			\
	} else {							\
	    ColorDitherBufComplete(cvdata, dstX1);			\
	}								\
    } while (0)
