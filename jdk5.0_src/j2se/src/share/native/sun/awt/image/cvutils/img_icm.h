/*
 * @(#)img_icm.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Decoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can decode the pixel information associated
 * with any Java IndexColorModel object.  This implementation examines
 * some of the private fields of the IndexColorModel object and decodes
 * the red, green, blue, and possibly alpha values directly rather than
 * calling the getRGB method on the Java object.
 */

/*
 * These definitions vector the standard macro names to the "ICM"
 * versions of those macros only if the "DecodeDeclared" keyword has
 * not yet been defined elsewhere.  The "DecodeDeclared" keyword is
 * also defined here to claim ownership of the primary implementation
 * even though this file does not rely on the definitions in any other
 * files.
 */
#ifndef DecodeDeclared
#define DeclareDecodeVars	DeclareICMVars
#define InitPixelDecode(CM)	InitPixelICM(unhand(CM))
#define PixelDecode		PixelICMDecode
#define DecodeDeclared
#endif

#include "java_awt_image_IndexColorModel.h"

#define DeclareICMVars					\
    unsigned int mapsize;				\
    unsigned int *cmrgb;

#define InitPixelICM(CM)					\
    do {							\
	Classjava_awt_image_IndexColorModel *icm =		\
	    (Classjava_awt_image_IndexColorModel *) CM;		\
	cmrgb = (unsigned int *) unhand(icm->rgb);		\
	mapsize = obj_length(icm->rgb);				\
    } while (0)

#define PixelICMDecode(CM, pixel, red, green, blue, alpha)	\
    do {							\
	VerifyPixelRange(pixel, mapsize);			\
	pixel = cmrgb[pixel];					\
	IfAlpha(alpha = (pixel >> ALPHASHIFT) & 0xff;)		\
	red = (pixel >> REDSHIFT) & 0xff;			\
	green = (pixel >> GREENSHIFT) & 0xff;			\
	blue = (pixel >> BLUESHIFT) & 0xff;			\
    } while (0)
