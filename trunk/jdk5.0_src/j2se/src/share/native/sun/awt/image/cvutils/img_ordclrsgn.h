/*
 * @(#)img_ordclrsgn.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Encoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation uses an ordered dithering error matrix with
 * signed error adjustments to produce a moderately high quality
 * version of an image with only an 8-bit (or less) RGB colormap and
 * a "closest color" lookup table.  The ordered dithering technique
 * does not rely on the order in which the pixels are processed so
 * this file can be used in cases where the ImageProducer has not
 * specified the TopDownLeftRight delivery hint.  The ordered dither
 * technique is also much faster than the Floyd-Steinberg error diffusion
 * algorithm so this implementation would also be appropriate for
 * cases where performance is critical such as the processing of a
 * video stream.
 *
 * This file can be used to provide the default implementation of the
 * Encoding macros for RGB colormapped displays.
 */

/*
 * These definitions vector the standard macro names to the "Color"
 * versions of those macros only if the "DitherDeclared" keyword has
 * not yet been defined elsewhere.  The "DitherDeclared" keyword is
 * also defined here to claim ownership of the primary implementation
 * even though this file does not rely on the definitions in any other
 * files.
 */
#ifndef DitherDeclared
#define DitherDeclared
#define DeclareDitherVars	DeclareAllColorDitherVars
#define InitDither		InitColorDither
#define StartDitherLine		StartColorDitherLine
#define DitherPixel		ColorDitherPixel
#define DitherBufComplete	ColorDitherBufComplete
#endif

#define DeclareAllColorDitherVars			\
    DeclareColorDitherVars				\
    int relx, rely;

#define DeclareColorDitherVars				\
    extern sgn_ordered_dither_array img_oda_red;	\
    extern sgn_ordered_dither_array img_oda_green;	\
    extern sgn_ordered_dither_array img_oda_blue;

#define InitColorDither(cvdata, clrdata, dstTW)			\
    do {} while (0)

#define StartColorDitherLine(cvdata, dstX1, dstY)		\
    do {							\
	relx = dstX1 & 7;					\
	rely = dstY & 7;					\
    } while (0)

#define ColorDitherPixel(dstX, dstY, pixel, red, green, blue)	\
    do {							\
	red += img_oda_red[relx][rely];				\
	red = ComponentBound(red);				\
	green += img_oda_green[relx][rely];			\
	green = ComponentBound(green);				\
	blue += img_oda_blue[relx][rely];			\
	blue = ComponentBound(blue);				\
	pixel = ColorCubeOrdMapSgn(red, green, blue);		\
	relx = (relx + 1) & 7;					\
    } while (0)

#define ColorDitherBufComplete(cvdata, dstX1)			\
    do {} while (0)
