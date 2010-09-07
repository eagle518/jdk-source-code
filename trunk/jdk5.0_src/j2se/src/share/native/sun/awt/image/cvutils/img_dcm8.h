/*
 * @(#)img_dcm8.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Decoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can decode the pixel information associated
 * with Java DirectColorModel objects where the color masks are
 * guaranteed to be at least 8-bits wide each.  It is slightly more
 * efficient then the generic DCM parsing code since it does not need
 * to store or test component scaling values.  This implementation
 * examines some of the private fields of the DirectColorModel
 * object and decodes the red, green, blue, and possibly alpha values
 * directly rather than calling the getRGB method on the Java object.
 */

/*
 * These definitions vector the standard macro names to the "DCM8"
 * versions of those macros only if the "DecodeDeclared" keyword has
 * not yet been defined elsewhere.  The "DecodeDeclared" keyword is
 * also defined here to claim ownership of the primary implementation
 * even though this file does not rely on the definitions in any other
 * files.
 */
#ifndef DecodeDeclared
#define DeclareDecodeVars	DeclareDCM8Vars
#define InitPixelDecode(CM)	InitPixelDCM8(unhand(CM))
#define PixelDecode		PixelDCM8Decode
#define DecodeDeclared
#endif

#include "java_awt_image_DirectColorModel.h"

#define DeclareDCM8Vars						\
    IfAlpha(unsigned int alpha_off;)				\
    unsigned int red_off, green_off, blue_off;

#define InitPixelDCM8(CM)						\
    do {								\
	Classjava_awt_image_DirectColorModel *dcm =			\
	    (Classjava_awt_image_DirectColorModel *) CM;		\
	red_off = dcm->red_offset;					\
	green_off = dcm->green_offset;					\
	blue_off = dcm->blue_offset;					\
	IfAlpha(alpha_off = (dcm->alpha_mask == 0			\
			     ? -1					\
			     : dcm->alpha_offset);)			\
    } while (0)

#define PixelDCM8Decode(CM, pixel, red, green, blue, alpha)		\
    do {								\
	IfAlpha(alpha = ((alpha_off < 0)				\
			 ? 255						\
			 : (pixel >> alpha_off) & 0xff);)		\
	red = (pixel >> red_off) & 0xff;				\
	green = (pixel >> green_off) & 0xff;				\
	blue = (pixel >> blue_off) & 0xff;				\
    } while (0)
