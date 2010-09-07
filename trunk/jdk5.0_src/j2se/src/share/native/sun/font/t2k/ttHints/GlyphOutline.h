/*
 * @(#)GlyphOutline.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
 

#ifndef glyphOutlineDefined
#define glyphOutlineDefined

#include "Fnt.h"
#include "FSglue.h"

tt_int32 ComputeElementSizes(fastInt contourCount, fastInt pointCount, tt_int32* scratchSize);
void ResetHintedOutline(fnt_ElementType* elem);
void SetElementPointers(fnt_ElementType* outline, fastInt contourCount, 
			fastInt pointCount, void* permBuffer, void* tempBuffer);

#endif
