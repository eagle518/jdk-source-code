/*
 * @(#)GlyphOutline.h	1.13 10/04/02
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
 

#ifndef glyphOutlineDefined
#define glyphOutlineDefined

#include "Fnt.h"
#include "FSglue.h"

tt_int32 ComputeElementSizes(fastInt contourCount, fastInt pointCount, tt_int32* scratchSize);
void SetElementPointers(fnt_ElementType* outline, fastInt contourCount, 
			fastInt pointCount, void* permBuffer, void* tempBuffer);

#endif
