/*
 * @(#)img_opaque.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains macro definitions for the Alpha category of the
 * macros used by the generic scaleloop function.
 *
 * This implementation of the Alpha macros will ignore all alpha
 * information.  It also provides an empty expansion of the IfAlpha
 * macro which keeps the other macro sets in the image package from
 * wasting time and space on code to fetch or store the alpha
 * information.  This file is only applicable when the incoming
 * data is known to be entirely opaque and there is not yet any
 * image mask or alpha buffer associated with the output data.
 */

/*
 * The macro IfAlpha is used by the varous pixel conversion macros
 * to conditionally compile code that is only needed if alpha values
 * are going to be used.
 */
#define IfAlpha(statements)	/* Omit alpha handling code */

#define DeclareAlphaVars

#define InitAlpha(cvdata, dstY, dstX1, dstX2)			\
    do {} while (0)

#define StartAlphaRow(cvdata, DSTX1, DSTY)			\
    do {} while (0)

#define ApplyAlpha(cvdata, dstX, dstY, alpha)			\
    do {} while (0)

#define EndMaskLine()						\
    do {} while (0)
