/*
 * @(#)img_noscale.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Scaling category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation performs no input resampling whatsoever and
 * is only valid if the input data is delivered at the exact same
 * resolution as the output data is being generated.  At the same
 * time, this implementation of the Scaling macros is the most optimal
 * such implementation.
 */

#define DeclareScaleVars					\
    int dstX, dstY, dstX2, dstY2;

#define SRCX	dstX
#define SRCY	dstY
#define DSTX	dstX
#define DSTY	dstY
#define DSTX1	srcOX
#define DSTY1	srcOY
#define DSTX2	dstX2
#define DSTY2	dstY2

#define InitScale(pixels, srcOff, srcScan,				\
		  srcOX, srcOY, srcW, srcH,				\
		  srcTW, srcTH, dstTW, dstTH)				\
    do {								\
	dstX2 = srcOX + srcW;						\
	dstY2 = srcOY + srcH;						\
	SetInputRow(pixels, srcOff, srcScan, srcOY, srcOY);		\
    } while (0)

#define RowLoop(srcOY)							\
    for (dstY = srcOY; dstY < dstY2; dstY++)

#define RowSetup(srcTH, dstTH, srcTW, dstTW,				\
		 srcOY, pixels, srcOff, srcScan)			\
	do {} while (0)

#define ColLoop(srcOX)							\
	for (dstX = srcOX; dstX < dstX2; dstX++)

#define ColSetup(srcTW, dstTW, pixel)					\
	    pixel = GetPixelInc()

#define RowEnd(srcTH, dstTH, srcW, srcScan)				\
	InputPixelInc(srcScan - srcW)
