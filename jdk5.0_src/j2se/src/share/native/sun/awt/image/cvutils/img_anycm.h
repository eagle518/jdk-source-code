/*
 * @(#)img_anycm.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Decoding category of
 * the macros used by the generic scaleloop function.
 *
 * This implementation can decode the pixel information associated
 * with any valid Java ColorModel object by dynamically invoking the
 * getRGB method on that object.  The implementation will also
 * optimally handle pixel data coming from IndexColorModel and
 * DirectColorModel objects so that it can be used as the default
 * fallback implementation for corner cases without imposing the
 * enormous performance penalty required for handling the custom
 * ColorModel objects in those cases.
 *
 * This file can be used to provide the default implementation of the
 * Decoding macros, handling all color conversion cases.
 */

/*
 * These definitions vector the standard macro names to the "Any"
 * versions of those macros.  The "DecodeDeclared" keyword is also
 * defined to indicate to the other include files that they are not
 * defining the primary implementation.  All other include files
 * will check for the existance of the "DecodeDeclared" keyword
 * and define their implementations of the Decoding macros using
 * more specific names without overriding the standard names.
 * This is done so that the other files can be included here to
 * reuse their implementations for the specific optimization cases.
 */
#define DecodeDeclared
#define DeclareDecodeVars	DeclareAnyVars
#define InitPixelDecode		InitPixelAny
#define PixelDecode		PixelAnyDecode

/* Include the optimal implementations for Index and Direct ColorModels */
#include "img_icm.h"
#include "img_dcm.h"

#define ICMTYPE		0
#define DCMTYPE		1
#define OCMTYPE		2

#define DeclareAnyVars						\
    DeclareICMVars						\
    DeclareDCMVars						\
    struct execenv *ee;						\
    struct methodblock *mb = 0;					\
    int CMtype;

#define InitPixelAny(CM)						\
    do {								\
	Classjava_awt_image_ColorModel *cm =				\
	    (Classjava_awt_image_ColorModel *) unhand(CM);		\
	ImgCMData *icmd = (ImgCMData *) cm->pData;			\
	if ((icmd->type & IMGCV_CMBITS) == IMGCV_ICM) {			\
	    CMtype = ICMTYPE;						\
	    InitPixelICM(cm);						\
	} else if (((icmd->type & IMGCV_CMBITS) == IMGCV_DCM)		\
		   || ((icmd->type & IMGCV_CMBITS) == IMGCV_DCM8)) {	\
	    CMtype = DCMTYPE;						\
	    InitPixelDCM(cm);						\
	} else {							\
	    CMtype = OCMTYPE;						\
	    ee = EE();							\
	    mb = icmd->mb;						\
	}								\
    } while (0)

#define PixelAnyDecode(CM, pixel, red, green, blue, alpha)		\
    do {								\
	switch (CMtype) {						\
	case ICMTYPE:							\
	    PixelICMDecode(CM, pixel, red, green, blue, alpha);		\
	    break;							\
	case DCMTYPE:							\
	    PixelDCMDecode(CM, pixel, red, green, blue, alpha);		\
	    break;							\
	case OCMTYPE:							\
	    pixel = do_execute_java_method(ee, (void *) CM,		\
					   "getRGB","(I)I", mb,		\
					   FALSE, pixel);		\
	    if (exceptionOccurred(ee)) {				\
		return SCALEFAILURE;					\
	    }								\
	    IfAlpha(alpha = pixel >> ALPHASHIFT;)			\
	    red = (pixel >> REDSHIFT) & 0xff;				\
	    green = (pixel >> GREENSHIFT) & 0xff;			\
	    blue = (pixel >> BLUESHIFT) & 0xff;				\
	    break;							\
	}								\
    } while (0)
