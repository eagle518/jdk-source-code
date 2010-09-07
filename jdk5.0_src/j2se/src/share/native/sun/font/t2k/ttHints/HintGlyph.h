/*
 * @(#)HintGlyph.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/*	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

#ifndef	NewGlyphIncludes
#define	NewGlyphIncludes

#include "Hint.h"

/* NewGlyph.c,  */
/* Replaces  TTScalerNewGlyph. Also adds a Close routine. */
 

int TTScalerHintGlyph(
 	GlyphClass *theGlyphT2K,	/* Original T2K glyph data as input */
 	T2K 	   *aT2KScaler , 	/* T2K scaler data	*/
 	void 	   **scHintPtrVoid,	/* use null if no image is to be created. */
	tt_int32      applyStyle
	);
    
/* As of yet, there is no "close" function. */
/*	its not yet needed. */

#endif
