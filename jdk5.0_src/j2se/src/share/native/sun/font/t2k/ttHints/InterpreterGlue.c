/*
 * @(#)InterpreterGlue.c	1.7 03/12/19 
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*

	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
 */
#include <string.h>

#include "Hint.h"

#ifdef ENABLE_TT_HINTING
#ifdef debugging
#include <stdio.h>
#endif

#include "InterpreterGlue.h"
#include "SfntAccess.h"

#include "Fnt.h"

local void ScaleCVT(fastInt count, const FWord src[], F26Dot6 dst[], fixed scalar);
local void ScaleCVT(fastInt count, const FWord src[], F26Dot6 dst[], fixed scalar)
{
  if (count) {
      do
	*dst++ = FixedMultiplyRound(scalar, *src++);
      while (--count);
  }
}

void PrepareTheCVT(fsg_SplineKey* key, fixed scale)
{
	tt_int32 length;
 	memoryContext	*at2kMem=key->memContext;
	sfntClass 		 * 	aSfntClassFont;
	perFont* theFont = TheFont(key);

 	aSfntClassFont= at2kMem->aSfntClassFont;	

	if (theFont->cvtCount)
	{	
		perVariation* perVar = TheVari(key);
#if 0
				if (perVar->hasStyleCoord && TheFont(key)->expectCVAR)
					ScaleCVT(
						TheFont(key)->cvtCount,
						(const FWord*)perVar->styledCvt.ptr,
						TheTrans(key)->globalGS.controlValueTable,
						scale);
				else
#endif
		{	
			FWord* cvtTable = (FWord*)(aSfntClassFont->cvt)->varFWords;
 			ScaleCVT(TheFont(key)->cvtCount, cvtTable, 
				 TheTrans(key)->globalGS.controlValueTable, scale);
		}
	}
}

 
local tt_int32 SetUpProgramPtrs(
	fsg_SplineKey* key, fnt_GlobalGraphicStateType* globalGS, boolean needPreProgram);

/* Setup the program pointers for the font program and prep (pre-program). */
/* So why is this routine called for glyphs? */
local tt_int32 SetUpProgramPtrs(
	fsg_SplineKey* key, 
	fnt_GlobalGraphicStateType* globalGS,
	 boolean needPreProgram)
{
	tt_int32 length=0;
 	memoryContext	*at2kMem=key->memContext;
	sfntClass 		 * 	aSfntClassFont;
	perFont* theFont = TheFont(key);

 	aSfntClassFont= at2kMem->aSfntClassFont;	
	globalGS->pgmList[fontProgramIndex] = 
		globalGS->pgmList[preProgramIndex] = (tt_uint8*)nil;

    if (theFont->expectFPGM)
    {
	 	length = (aSfntClassFont->fpgm)->numInstructions;
	 	globalGS->pgmList[fontProgramIndex]= (aSfntClassFont->fpgm)->instructions;
		  /*MTE: replaced   */
		  /*  ScalerGetFontTable(key->memContext, fontProgramTableTag, 0, 0,   */
		  /* 	 &globalGS->pgmList[fontProgramIndex], kRequiredFontTable); */
	}
	if (needPreProgram)
	{	
		IfDebugMessage(theFont->expectPREP == false, "missing prep", 0);
		length =(aSfntClassFont->prep)->numInstructions;
	 	globalGS->pgmList[preProgramIndex]= (aSfntClassFont->prep)->instructions;
				/* MTE: Not needed ScalerGetFontTable(key->memContext, preProgramTableTag, 0, 0, */
				/*  &globalGS->pgmList[preProgramIndex], kRequiredFontTable); */
	}
#if defined(debugging) || defined(performRuntimeErrorChecking)
		globalGS->maxp = &theFont->maxProfile;
#endif
#ifdef performRuntimeErrorChecking
		globalGS->cvtCount = theFont->cvtCount;
#endif
	return length;
}
 
void SetGlobalGSDefaults( fnt_GlobalGraphicStateType *globalGS )
{
	globalGS->defaultParBlock.wTCI				= fnt_pixelSize * 17 / 16;
	globalGS->defaultParBlock.sWCI				= 0;
 	globalGS->defaultParBlock.scanControl		= 0;
	globalGS->defaultParBlock.instructControl	= 0;

	globalGS->defaultParBlock.minimumDistance	= fnt_pixelSize;
	globalGS->defaultParBlock.RoundValue		= (FntRoundFunc) fnt_RoundToGrid;
 	globalGS->defaultParBlock.deltaBase			= 9;
	globalGS->defaultParBlock.deltaShift		= 3;
	globalGS->defaultParBlock.sW				= 0;
	globalGS->defaultParBlock.autoFlip			= true;

	globalGS->preProgramHasDefs					= false;
}

void SetGlobalGSMapping(fnt_GlobalGraphicStateType* globalGS, const gxMapping* stretch, FWord upem)
{
	globalGS->fractionalPPEM.x	= FixedToF26Dot6(stretch->map[0][0]);
	globalGS->fractionalPPEM.y	= FixedToF26Dot6(stretch->map[1][1]);
	globalGS->integerPPEM.x		= FixedRound(stretch->map[0][0]);
	globalGS->integerPPEM.y		= FixedRound(stretch->map[1][1]);
	globalGS->upemScale.x		= MultiplyDivide(stretch->map[0][0], 64, upem);
	globalGS->upemScale.y		= MultiplyDivide(stretch->map[1][1], 64, upem);
	globalGS->cvtStretch.x		= FixedDivide(stretch->map[0][0], globalGS->cvtScale);
	globalGS->cvtStretch.y		= FixedDivide(stretch->map[1][1], globalGS->cvtScale);

	globalGS->identityTransformation = globalGS->cvtStretch.x == fixed1 && globalGS->cvtStretch.y == fixed1;
	globalGS->non90DegreeTransformation = 0;		/* need to compute this */
}


/* The following routine is called before each call to fnt_Execute to setup */
/*	parameters for the globalGS stack. These will be copied again, later, when */
/*		the local state is setup.. Arg.. */
static void SetGlobalGSStackData(fsg_SplineKey* key,fnt_GlobalGraphicStateType *globalGS); 
static void SetGlobalGSStackData(fsg_SplineKey* key,fnt_GlobalGraphicStateType *globalGS) 
{

	globalGS->stackZone = key->theTrans->stackTransPtr;
	globalGS->stackSize =key->theTrans->stackBytes;

}
/* MTE: Without T2K, void RunFontProgram(fsg_SplineKey* key, voidFunc traceFunc) */
/* For some reason, the globalGS must exist before running the program. */
/*		But his means that the transform variables must exist! */
/*	Do the glyphs and glyph scratch also need to be setup?? */
/*			Apparently so, see debugcode statements within the routine. */
 void RunFontProgram(fsg_SplineKey* key, voidFunc traceFunc)
{
	tt_int32 tableLength;
	fnt_ElementType* elements[ELEMENTCOUNT];
	fnt_GlobalGraphicStateType *globalGS;
	
	globalGS = &TheTrans(key)->globalGS;
	globalGS->instrDefCount = 0;
	globalGS->pgmIndex = fontProgramIndex;
	tableLength = SetUpProgramPtrs(key, globalGS, false);
	DebugCode(elements[TWILIGHTZONE] = kBusErrorValue );
	DebugCode(elements[GLYPHELEMENT] = kBusErrorValue);
	SetGlobalGSStackData(  key,globalGS) ;
	fnt_Execute(
	    elements, 
	    globalGS, 
	    globalGS->pgmList[fontProgramIndex],
		globalGS->pgmList[fontProgramIndex] + tableLength, 
		traceFunc,
		 key->memContext, 
		 false, false, false);
	TheFont(key)->IDefCount = globalGS->instrDefCount;	
}

void RunPreProgram(register fsg_SplineKey *key, const transformState* state, voidFunc traceFunc) 
{
	tt_int32 tableLength;
	fnt_ElementType* elements[ELEMENTCOUNT];
	fnt_GlobalGraphicStateType *globalGS = &TheTrans(key)->globalGS;

	IfDebug(TheFont(key)->expectPREP == false, "\pDon't call RunPreProgram w/o a 'prep'");
	SetGlobalGSDefaults(globalGS);

	globalGS->pgmIndex = preProgramIndex;
	tableLength = SetUpProgramPtrs(key, globalGS, true);
	SetGlobalGSMapping(globalGS, &state->stretchBase, TheFont(key)->emResolution);
	globalGS->localParBlock = globalGS->defaultParBlock;

	elements[TWILIGHTZONE] = &TheScratch(key)->elementStorage[TWILIGHTZONE];
	DebugCode(elements[GLYPHELEMENT] = kBusErrorValue);
	SetGlobalGSStackData(  key,globalGS) ;
	fnt_Execute(elements, globalGS, globalGS->pgmList[preProgramIndex],
				globalGS->pgmList[preProgramIndex] + tableLength,
				traceFunc, key->memContext, false, false, false);
	if (!(globalGS->localParBlock.instructControl & DEFAULTFLAG))
		globalGS->defaultParBlock = globalGS->localParBlock;	/* change default parameters */
}

/* - called by StretchGlyph, of all things. */
#ifdef UIDebug
	void SetInitialInstructionBlock( Ptr s,  Ptr e);  
#endif
void RunGlyphProgram(
	fsg_SplineKey* key, 
	fnt_ElementType* outline, 
	const transformState* state,
	fastInt instructionSize, 
	unsigned char instructionData[], 
	boolean hintingAComposit)
{
	fnt_ElementType* elements[ELEMENTCOUNT];
	fnt_GlobalGraphicStateType *globalGS = &TheTrans(key)->globalGS;

	IfDebug(instructionSize <= 0, "\pDon't call RunGlyphProgram with non-positive instructionSize");
	globalGS->pgmIndex = noProgramIndex;
	SetUpProgramPtrs(key, globalGS, globalGS->preProgramHasDefs);
	SetGlobalGSMapping(globalGS, &state->stretchBase, TheFont(key)->emResolution);
	globalGS->localParBlock = globalGS->defaultParBlock;
#ifdef UIDebug
	{
		int glyphCode;
		glyphCode= outline->glyphIndex;
		if (glyphCode==78)
		{
			int defScan, localScan;
			int k;
			defScan= globalGS->defaultParBlock.scanControl;
			localScan= globalGS->localParBlock.scanControl;
			k= defScan+localScan;
		}
 	}
#endif
	elements[TWILIGHTZONE] = &TheScratch(key)->elementStorage[TWILIGHTZONE];
	elements[GLYPHELEMENT] = outline;
	FillBytes(outline->f, outline->pointCount * sizeof(char), 0);	
	SetGlobalGSStackData(  key,globalGS) ;
#ifdef  UIDebug
	/* Save the start and end of the top level glyph program. */
	SetInitialInstructionBlock(((void *)instructionData),(void *)(instructionData + instructionSize)); 
#endif

 	fnt_Execute(
		elements,
		globalGS,
		instructionData,
		instructionData + instructionSize,
		(voidFunc)nil,
		key->memContext,
		TheVari(key)->hasStyleCoord,
		TheFont(key)->expectCVAR,
		hintingAComposit);
#ifdef UIDebug
	{
		int glyphCode;
		glyphCode= outline->glyphIndex;
		if (glyphCode==78)
		{
			int defScan, localScan;
			int k;
			defScan= globalGS->defaultParBlock.scanControl;
			localScan= globalGS->localParBlock.scanControl;
			k= defScan+localScan;
		}
 	}
#endif
}
#endif  
