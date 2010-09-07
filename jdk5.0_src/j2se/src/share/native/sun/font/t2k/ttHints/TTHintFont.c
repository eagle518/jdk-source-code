/*
 * @(#)TTHintFont.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

/* Needed to define intptr_t */
#include "gdefs.h"

#include "HintFont.h"
#ifdef ENABLE_TT_HINTING

 
  /* Allocate a block from the scaler.  This routine is used internally.*/
  		tsiMemObject * GetPerFontMemoryAllocator(perFont	*aPerFont) 
		{
			return(aPerFont->aMemoryContext.aTSIMemObject);
		}
		
void * GetPerFontMemory(perFont	*aPerFont, size_t memSize)
{
	return(tsi_AllocMem( GetPerFontMemoryAllocator( aPerFont) , memSize  ));
}
 
void ReleasePerFontMemory(perFont	*aPerFont,void *block)
{
	tsi_DeAllocMem(GetPerFontMemoryAllocator( aPerFont), block );
}
 
	/* 
		The following routine is used only within the TTScalerNewFont routine to
 		determine the maximum condition. 
 		This routine is untouched from original code.
 	*/
 		
	local fastInt LargestElementCounts(sfntMaxProfileTable* maxProfile, fastInt* contourCountPtr);
	local fastInt LargestElementCounts(sfntMaxProfileTable* maxProfile, fastInt* contourCountPtr)
	{
		fastInt pointCount, contourCount;

		contourCount = maxProfile->maxContours;
		if (contourCount < maxProfile->maxCompositeContours)
			contourCount = maxProfile->maxCompositeContours;
		pointCount = maxProfile->maxPoints;
		if (pointCount < maxProfile->maxCompositePoints)
			pointCount = maxProfile->maxCompositePoints;

		*contourCountPtr = contourCount + privatePhantomCount;
		return pointCount + privatePhantomCount;
	}
	

/* Key Processing.	*/
	/*	The whole "key" structured  appears to be related to the fact that the	*/
	/*		original developer didn't know how to create forward referencing in	*/
	/*		data structures. 	*/
	/*	Therefore a "key" was developed that kept track	*/
	/*		of these interlinked data structures.  	*/
	/*	Its cumbersome, and should someday be eliminated.	*/
	/*  this routine is used solely in InitTheKey and was extracted for clarity.	*/
 
	static void SetupDebugPointers(fsg_SplineKey *key);
	static void SetupDebugPointers(fsg_SplineKey *key)
	{
			key->memContext= kBusErrorValue;		
  			key->theFont = kBusErrorValue;
			key->theVari = kBusErrorValue;
			key->theTrans = kBusErrorValue;
			key->theGlyph = kBusErrorValue;
			key->fontBlockPtrs.functionDefs = kBusErrorValue;
			key->fontBlockPtrs.instructionDefs = kBusErrorValue;
			key->fontBlockPtrs.compoundClone = kBusErrorValue;
 	}
 
 /* MTE: Initialize a redundant data structure of pointers.	*/
void InitTheKeyByScaler( fsg_SplineKey *key)
{
   	SetupDebugPointers(key);
  }

/* MTE: Initialize a redundant data structure of pointers.	*/
void InitTheKeyByFont(  fsg_SplineKey *key, perFont * aPerFont)
{
 	InitTheKeyByScaler(key); 
 	key->memContext=&aPerFont->aMemoryContext;
 	key->theFont = aPerFont;
	key->fontBlockPtrs.functionDefs = aPerFont->funcDefPtr;
	key->fontBlockPtrs.instructionDefs= aPerFont->instrDefPtr; 
}
 /* MTE: Initialize a redundant data structure of pointers.	*/
void InitTheKeyByVary(fsg_SplineKey *key, perVariation * aPerVary)
{
	struct perFont * xxx= aPerVary->xPerFontContext;
	perFont * aPerFont = (perFont *) xxx;
	
	InitTheKeyByFont(key, aPerFont);
 	key->theVari = aPerVary;
 	/* all of the following is probably ignored..	*/
		aPerVary->userCoord.ptr = (char*)aPerVary + aPerVary->userCoord.offset;
		aPerVary->globalCoord.ptr = (char*)aPerVary + aPerVary->globalCoord.offset;
		aPerVary->styledCvt.ptr = (char*)aPerVary + aPerVary->styledCvt.offset;
}
 /* MTE: Initialize a redundant data structure of pointers.	*/
void InitTheKeyByTrans(fsg_SplineKey *key, perTransformationPtr aPerTran)
{
	struct perVariation * xxx= aPerTran->xPerVaryContext;
	perVariation * aPerVary = (perVariation *) xxx;
	perGlyphPtr theGlyph=0;
	
	InitTheKeyByVary(key, aPerVary);
 	key->theTrans = aPerTran; 	
 	/* We skip pointer setup, because its now down by the transformation setup.	*/
 	/* see TTScalerNewTransform	*/
 	/*key->theGlyph=theGlyph;	*/
   }

 
static int initScalerFlag=0; /* non-zero when inited */
perScaler globPerScaler;

/* Once only initialization routine */
static void OnceOnlyTTFDefInit(void)
{
 	if (initScalerFlag==0)
 	{
 		fnt_DefaultJumpTable(  globPerScaler.instructionJumpTable);
 		initScalerFlag=1;
	}
} 

sfntClass 		*GetSFNTClassFromPerFont(perFont *aPerFont)
{
	return(aPerFont->aMemoryContext.aT2KScaler->font);
}
tsiMemObject 	*GetMemObjectFromPerFont(perFont *aPerFont)
{
	return(aPerFont->aMemoryContext.aT2KScaler->mem);
}

T2K 		 	*GetT2KFromPerFont(perFont *aPerFont)
{
	return(aPerFont->aMemoryContext.aT2KScaler);
}

scalerError NewTTSHintcalerFont( T2K *	aT2KScaler, perFont **aPerFontPtr)
{
	fsg_SplineKey	key;					/* Scaler internal execution state */
	tt_int32		perFontBlockSize;			/* Total size of the per-gxFont permanent block */
	tt_int32		functionOffset;			/* Offset of the FDEF portion of perfont block */
	tt_int32		instructionOffset;		/* Offset of the IDEF portion of perfonr block */
	sfntMaxProfileTable * maxProfile;		/* maxp table from the sfnt */
	perFont*		theFont=0;
 	memoryContext	memContext, *at2kMem;
	tt_int32 usefulHeaderSize, instrSize, funcSize,extraDataStart;
 	sfntClass 		 *aSfntClassFont;
	maxpClass 			*maxp;
	hheaClass 			*hhea;
	hheaClass 			*vhea;
	headClass			*head;
	cvtClass			*cvt;
 	scalerError		theScalerResult= scalerError_NoError;
 	OnceOnlyTTFDefInit();
 	at2kMem=&memContext;
 	at2kMem->aTSIMemObject= aT2KScaler->mem;
 	at2kMem->aSfntClassFont=  aT2KScaler->font;
	at2kMem->aT2KScaler= aT2KScaler; 
	
	aSfntClassFont=	at2kMem->aSfntClassFont;   /* Get t2kfont data.	*/
	maxp=	aSfntClassFont->maxp;  /* get a pointer to the maxprofile table.*/
	/* Convert t2k max table class to the format for  TT.	*/
	if ( !maxp )
	  return theScalerResult; /* go out because this is required */
	maxProfile= (sfntMaxProfileTable * )&maxp->version;							/* VERY Tricky.	*/
	/* MTE added padding at end of header to ensure alignment.	*/
	usefulHeaderSize=sizeof(perFont);
 	extraDataStart= (usefulHeaderSize+3)& (~3);
 	functionOffset =extraDataStart;	
	funcSize =sizeof(fnt_funcDef) *  maxProfile->maxFunctionDefs ;
	instrSize=sizeof(fnt_instrDef) *  maxProfile->maxInstructionDefs ;
	instructionOffset = functionOffset + funcSize;
	perFontBlockSize = instructionOffset + 	instrSize;
	
	/* Allocate the font blocker	*/
	theFont = (perFont*) tsi_AllocMem( at2kMem->aTSIMemObject , perFontBlockSize  );
	
	/* Remember the context of the font.	*/
	theFont->aMemoryContext= memContext;
	/* Make a copy of the max. Shouldn't we just use the copy in T2K- tbd.	*/
	/* Its magical that the two are equal!	*/
	theFont->maxProfile = *maxProfile;

	/* Now setup areas allocated areas at the end of this block.	*/
	theFont->funcDefPtr=   (fnt_funcDef *)((uintptr_t) theFont + (uintptr_t) functionOffset );    
	/* Point to function definitions.	*/
	theFont->numFuncDefs= maxProfile->maxFunctionDefs; 
	/*function definition entries.	*/				 		
	theFont->instrDefPtr = (fnt_instrDef *)	 /* Point to instruction definitions	*/
            ( (uintptr_t)	theFont + (uintptr_t) instructionOffset);
	/* Point to function definitions.	*/
	theFont->numInstrDefs = maxProfile->maxInstructionDefs;
	/* total number of instruction entries.	*/

	/*
	 *	Need to set these offsets before calling InitTheKey
	*/
	theFont->functionDefsOffset = functionOffset;
	theFont->instructionDefsOffset = instructionOffset;
	/* MTE: Dont need for this routine:...InitTheKey(&key, &memContext);	*/

	/* These are used in other stages, but can be computed now*/
	theFont->storageSize = (tt_int32)sizeof(F26Dot6) * maxProfile->maxStorage;
	{	
	  tt_int32 tempSize;
	  cvt= aSfntClassFont->cvt;   	/* get a pointer to the maxprofile table.	*/
	  if (cvt)
	    theFont->cvtCount = cvt->numFWords;
	  else 
	    theFont->cvtCount =0;
	  theFont->twilightZoneSize = ComputeElementSizes( MAX_TWILIGHT_CONTOURS, 
							   maxProfile->maxTwilightPoints,
							   &tempSize);
	  theFont->twilightZoneSize += tempSize;
	}
	theFont->stackSize =( (maxProfile->maxStackElements+1) * 4);/* max stack size */
	/* Added one for safety. Not needed?	*/
	
	theFont->maxPointCount = LargestElementCounts(maxProfile, &theFont->maxContourCount);
	theFont->permGlyphSize = ComputeElementSizes(theFont->maxContourCount, 
						     theFont->maxPointCount, 
						     &theFont->tempGlyphSize);
	theFont->compoundCloneOffset = theFont->tempGlyphSize;
	theFont->tempGlyphSize += (8 * (maxProfile->maxComponentElements) + 10);	
		
	theFont->fontProgramRan = false;
	theFont->accentsExist =  false;
		 /* MTE: ScalerFontTableExists(&memContext, accentAttachmentTableTag);	*/
		
 	head =	aSfntClassFont->head; /* get a pointer to the maxprofile table.	*/
  	if (head) {	
	    theFont->emResolution	=  (head->unitsPerEm);
	    theFont->xMin		=  (head->xMin);
	    theFont->yMin	       	=  (head->yMin);
	    theFont->xMax	    	=  (head->xMax);
	    theFont->yMax	      	=  (head->yMax);
	    theFont->defaultSideBearing	=  FixedMultiply(theFont->emResolution,
							 SIDE_BEARING_PERCENT);
	    theFont->minimumPixPerEm	=  (head->lowestRecPPEM);
	    theFont->fontFlags			=  (head->flags);
	    theFont->indexToLocFormat	=  (head->indexToLocFormat);
	    theFont->usefulOutlines    	=  true; /* MTE Or shouldn't be called	*/
		
	    /* ScalerFontTableExists(&memContext, glyphDataTableTag);	*/
	}; /* ALWAYS SHOULD BE HEAD if doing outline fonts!	*/
      
    /* hheaClass	*/
    theFont->horiLongMetrics = theFont->vertLongMetrics = 0;
    hhea= aSfntClassFont->hhea;				 
    vhea= aSfntClassFont->vhea;				 
    if (hhea)
      theFont->horiLongMetrics =  hhea->numberOfHMetrics ;
    if (vhea)
      theFont->vertLongMetrics =  vhea->numberOfHMetrics ;
		
	 
  	/* No support for fmtx	*/
    theFont->useFMTX =   0L;
    theFont->expectFPGM = aSfntClassFont->fpgm!=0;
    /* MTE ScalerFontTableExists(&memContext, fontProgramTableTag);*/
    theFont->expectPREP = aSfntClassFont->prep!=0; 
    /* MTE ScalerFontTableExists(&memContext, preProgramTableTag);	*/
				
    /* Make like there is never a variations table.	*/
    theFont->expectFVAR=theFont->axisCount = 0;
    theFont->expectGVAR = theFont->expectCVAR = theFont->expectAVAR = false;
    /* MTE No support for bitmapLocationTableTag	*/
    theFont->usefulBitmaps = false; 	 
    /* We made, so return success	*/
    goto exit;

    theScalerResult=scalerError_Memory;
    if (theFont!=0)
      {
	tsi_DeAllocMem(at2kMem->aTSIMemObject, theFont );
      }
exit:
    *aPerFontPtr= theFont;		/* return new font cookie.	*/
    return (theScalerResult);
}

 /*  The variation record is very simple, because we don't allow variations.
    In fact, no allocation is even made, because we use a portion of the
    font record. This saves memory allocation and deallocation. But
    forces us to use the same variation for each font. Who cares, we
    don't really support variations anyway!
  */
  
  
 /* Call this once at program end to cleanup the context.
 	Currently, there is no cleanup required.
*/
void TTScalerCloseFont(perFont *aPerFont)
{
	if (aPerFont)
	{
		ReleasePerFontMemory(aPerFont,aPerFont);
	}
  }

void TTScalerNewVariation1Dot1(
			perFont  *aPerFontContext, perVariation **aPerVaryContext)
	{
	  	perVariation	* theVar=0;
	  	tt_int32			perVaryBlockSize=sizeof(perVariation);
	 	scalerError		theScalerResult= scalerError_NoError;
	  
		theVar = (perVariation*)  &aPerFontContext->theVary;
		theVar->xPerFontContext = (struct	perFont * )aPerFontContext;
		theVar->hasStyleCoord = false;
		*aPerVaryContext= theVar;		/* return new font cookie.	*/
 }

/* Bring up the default variation- it does nothing.*/
	void TTScalerNewVariationDefault(
			perFont  *aPerFontContext, 
			perVariation **aPerVaryContext)
	{
		TTScalerNewVariation1Dot1( aPerFontContext, aPerVaryContext);
	}

/*/ Call this once at program end to cleanup the context.
 	Currently, there is no cleanup required.*/

void TTScalerCloseVary(perVariation *aPerVaryContext)
	{
	}
 

/* High level calls from T2K to prepare for TrueType Hinting */

void NewTTHintFontForT2K(T2K *	aT2KScaler)
{
	perFont  *aPerFont, **aPerFontPtr;
	perVariation    *aPerVary;
	aPerFontPtr=  (perFont **) &aT2KScaler->TTHintFontData;
	if (aT2KScaler->TTHintFontData==0)
	{
		NewTTSHintcalerFont( aT2KScaler, aPerFontPtr);
		aPerFont= *aPerFontPtr;
		if (aPerFont)
			TTScalerNewVariationDefault(aPerFont, &aPerVary);
	}
}

void ReleaseTTHintFontForT2K(T2K *	aT2KScaler)
{
	perFont  *aPerFont= (perFont *) aT2KScaler->TTHintFontData;
	perVariation  *aPerVary;
	if (aPerFont!=0)
	{
		aPerVary=&aPerFont->theVary;
		TTScalerCloseVary(aPerVary);
 		TTScalerCloseFont(aPerFont);
		aT2KScaler->TTHintFontData=0;
	}
}
 
#endif /* #ifdef ENABLE_TT_HINTING */
























