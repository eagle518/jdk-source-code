/*
 * @(#)InterpreterGlue.h	1.11 10/04/02 
 */

/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
void RunFontProgram(fsg_SplineKey* key, voidFunc traceFunc, jmp_buf* env);
void RunPreProgram(register fsg_SplineKey *key, const gxMapping* stretch, 
                   voidFunc traceFunc, jmp_buf* env);
void RunGlyphProgram(fsg_SplineKey* key, fnt_ElementType* outline, const gxMapping* stretch,
                     fastInt instructionSize, unsigned char* instructionData, 
                     boolean hintingAComposit, jmp_buf* env);
fastInt CountGlyphComponents(register const short componentData[], tt_int32* bloatedSize);
void CreateGlyphOutline(fsg_SplineKey* key, tt_uint16 glyphIndex, boolean executeInstructions);
void StretchGlyph(fsg_SplineKey* key, fnt_ElementType* outline, const gxMapping* stretch, short* compVarBuffer[], boolean executeInstructions, boolean* hasScaledComponentP);
void GetHmtxData(fsg_SplineKey* key, long glyphIndex, unscaledMetrics* hori, unscaledMetrics* vert, tt_uint16* longHMetrics, tt_uint16* longVMetrics);

#endif
