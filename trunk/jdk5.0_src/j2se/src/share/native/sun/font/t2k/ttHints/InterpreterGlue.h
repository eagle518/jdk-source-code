/*
 * @(#)InterpreterGlue.h	1.7 03/12/19 
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
     
 */

#ifndef interpreterGlueIncludes
#define interpreterGlueIncludes

#include "FSglue.h"

void SetGlobalGSDefaults( fnt_GlobalGraphicStateType *globalGS );
void SetGlobalGSMapping(fnt_GlobalGraphicStateType* globalGS, const gxMapping* stretch, FWord upem);
void PrepareTheCVT(fsg_SplineKey* key, fixed scale);
void RunFontProgram(fsg_SplineKey* key, voidFunc traceFunc);
void RunPreProgram(register fsg_SplineKey *key, const transformState* state, voidFunc traceFunc);
void RunGlyphProgram(fsg_SplineKey* key, fnt_ElementType* outline, const transformState* state,
						fastInt instructionSize, unsigned char* instructionData, boolean hintingAComposit);

fastInt CountGlyphComponents(register const short componentData[], tt_int32* bloatedSize);
void CreateGlyphOutline(fsg_SplineKey* key, tt_uint16 glyphIndex, boolean executeInstructions);
void StretchGlyph(fsg_SplineKey* key, fnt_ElementType* outline, const transformState* state, short* compVarBuffer[], boolean executeInstructions, boolean* hasScaledComponentP);
void GetHmtxData(fsg_SplineKey* key, long glyphIndex, unscaledMetrics* hori, unscaledMetrics* vert, tt_uint16* longHMetrics, tt_uint16* longVMetrics);

#endif
