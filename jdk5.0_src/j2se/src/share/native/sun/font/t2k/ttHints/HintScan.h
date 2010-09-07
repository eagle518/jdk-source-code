/*
 * @(#)HintScan.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
/*	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

#ifndef	HintScanIncludes
#define	HintScanIncludes

#include "Hint.h"

/* This module is largely an extraction from the "FontScaler.h" */
  
 /* Prototypes for the lower-level gxFont scaler routines */
tt_int32 fs_ContourScan3( fsg_SplineKey *key, const fnt_ElementType* charData, 
	 		/*		const scalerGlyph* theGlyphRecord,  */
	 				scalerBitmap* glyphImage);
tt_int32 fs_dropOutVal( fsg_SplineKey *key );
tt_int32 fs_CalculateBounds(fsg_SplineKey *key, const fnt_ElementType* charData, 
					scalerBitmap *glyphImage);
void fs_FindBitMapSize4(
	fsg_SplineKey *key, const fnt_ElementType* charData, 
			 scalerBitmap *glyphImage);

#endif
