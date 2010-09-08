/*
 * @(#)HintGlyph.c	1.21 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

#include "HintGlyph.h"
#include "HintScan.h"
#ifdef UIDebug
#include "HintState.h"
#endif

#ifdef UIDebug					
extern int isAllowHintingFlag;
#endif

#include <setjmp.h>
#include "t1hint.h"

#ifdef ENABLE_TT_HINTING

local void CreateGlyphElement(tt_int32 glyphIndex, 
                              fnt_ElementType *anOutline,
                              GlyphClass  *t2kGlyph,
                              fsg_SplineKey* key,
                              boolean useHints,
                              jmp_buf* env) {
	perFont* theFont = TheFont(key);
	perTransformation* theTrans = TheTrans(key);
	perGlyphScratch* theScratch = TheScratch(key);

	if (useHints) {	
		theScratch->elementStorage[TWILIGHTZONE].contourCount = 1;
		theScratch->elementStorage[TWILIGHTZONE].pointCount = 
		  theFont->maxProfile.maxTwilightPoints;
		SetElementPointers(
				&theScratch->elementStorage[TWILIGHTZONE], 
				1, 
				theScratch->elementStorage[TWILIGHTZONE].pointCount,
				nil, theTrans->twilightZone.ptr);
		{ 
		
		  theTrans->globalGS.function = globPerScaler.instructionJumpTable;
		  theTrans->globalGS.funcDef = (fnt_funcDef*)key->fontBlockPtrs.functionDefs;
		  theTrans->globalGS.instrDef = (fnt_instrDef*)key->fontBlockPtrs.instructionDefs;
		  theTrans->globalGS.store    = (F26Dot6*)theTrans->storage.ptr;
			theTrans->globalGS.controlValueTable	= (F26Dot6*)theTrans->scaledCvt.ptr;
		  theTrans->globalGS.variationCoord	= (shortFrac*)TheVari(key)->userCoord.ptr;
#ifdef check_cvt_access_direction
				theTrans->globalGS.cvtFlags		= theTrans->cvtFlags.ptr;
#endif
            if (theFont->expectFPGM && !theFont->fontProgramRan) {
                RunFontProgram(key, (voidFunc)nil, env);
				theFont->fontProgramRan = true;
			}
			if (theFont->cvtCount && !theTrans->cvtHasBeenScaled) {	
				PrepareTheCVT(key, MultiplyDivide(theTrans->globalGS.cvtScale, 64, theFont->emResolution));
				theTrans->cvtHasBeenScaled = true;
			}
            if (theFont->expectPREP && !theTrans->preProgramRan) {
                RunPreProgram(key, &theTrans->stretchTransform, (voidFunc)nil, env);
                theTrans->preProgramRan = true;
            }
		}
	} 
	{
 		boolean hintingAComposit=true; 	/* Can we use the GlyphClass data to set	*/
 															/*  this correctly? 	*/
 		fastInt numInstructions = t2kGlyph->hintLength;	/* 	*/
	 	tt_uint8 *instructionPtr = t2kGlyph->hintFragment;	/* Print_glyphClassInstructions	*/ 
 		/* See StretchGlyph for setting up the "outline" parameter. 	*/		
	 
	 	/* Debug Break: turn hinting on/off, check before and after. */
		if ( useHints  && (numInstructions>0)
#ifdef UIDebug					
					&& isAllowHintingFlag
#endif
		 )  {	   				 
		/*  should the storage be zeroed? Not really.	*/
  		/*	FillBytes(theTrans->storageTransPtr,theTrans->storageBytes, 0);	*/
            RunGlyphProgram(key, anOutline, &(theTrans->stretchTransform), numInstructions,
                            instructionPtr, hintingAComposit, env);
 		}
	}
}

/* Routine to process a glyph outline.	*/
/*  The processing of multi-component glyphs is handled by T2K.	*/
/*		This routine only performs the lowest level hinting of a single component.	*/
/*	All context including transformation matrices are already accomplished.	*/
#ifdef UIDebug
extern tt_int32 gridMagnify;
#endif

	/* Routine to add an offset a list of  values: e.g. x,y axes */
	static void OffsetAxis(F26Dot6* aAxis, F26Dot6 offset, tt_uint32 count)
	{
		 tt_int32 i;
		 for (i=0;i<i;i++)
		 	*aAxis++ += offset;
	}
 	static void AdjustPhantomAxis( F26Dot6* axis,   tt_uint32 count)
	{
		tt_int32 totalCount= count+2; /* Add the 2 phantom points. */
		F26Dot6* phantoms = axis + count;
  		F26Dot6  orgLeftPhantom, orgRightPhantom, orgAdvance, newLeftPhantom, leftChange;
 		F26Dot6   roundedAdvance, adjustedRightPhantom;
 	
		/* Find the new left side x phantom */
		 orgLeftPhantom= phantoms[leftSidePhantom];
		 newLeftPhantom = RoundF26Dot6(orgLeftPhantom);
		 leftChange= newLeftPhantom-orgLeftPhantom;
		 orgRightPhantom= phantoms[rightSidePhantom];
		 orgAdvance= orgRightPhantom-orgLeftPhantom;
		 roundedAdvance=  RoundF26Dot6(orgAdvance);
		 
		 /* offset  vector so that left phantom is rounded */
		 OffsetAxis(axis, leftChange,totalCount);
		 /* now adjust the right phantom */
		 adjustedRightPhantom= newLeftPhantom+roundedAdvance;
		 phantoms[rightSidePhantom] = adjustedRightPhantom;
	}
	
	/* Adjust phantom points from defunct "RoundPhantomPoints"  */
	/* First the entire glyph is adjusted to the origin, then	*/
	/* the right side phantom point is rounded					*/
	static void AdjustPhantoms(F26Dot6 *x,F26Dot6 *y,   tt_int32 points)
	{
		/* Fix the x axis. */
		AdjustPhantomAxis(x,  points);
		/* Fix the y axis. (Taligent doesn't do this completely)*/
		AdjustPhantomAxis(y,  points);
	}
		
 

int TTScalerHintGlyph(GlyphClass *theGlyphT2K, /* Original T2K glyph data as input */
                      T2K * aT2KScaler) { /* T2K scaler data */
     jmp_buf env;
 	fsg_SplineKey		*key, keyData ;  	/* Setup with non-glyph data.	*/
	scalerError		theScalerResult= scalerError_NoError;
	/* Specify the 3 possible allocated memory blocks.	*/
		perGlyphScratch*	theScratch=0L;
		void 	*glyphTemphBuffer=0L;
		perGlyph*			theGlyph=0L;
	fnt_ElementType*	glyphElement;
 	size_t				perGlyphScratchSize=sizeof(perGlyphScratch);
 	size_t				thetempGlyphSize,perGlyphPlusPermSize;
  	 perTransformation   *aPerTrans = 
 		(perTransformation  *)  aT2KScaler->TTHintTranData; ;
	
 	// to do special stuff only for non - TT fonts
	if ( (aT2KScaler->font->T1 != NULL ) || ( aT2KScaler->font->T2 != NULL )) {
        Type1HintGlyph(theGlyphT2K, aT2KScaler);
	    return (0);
	}
 	/* Init the key before the font program.	*/
 	key=&keyData;
        InitTheKeyByTrans(key,  aPerTrans);

        /* temp memory is allocated based on max point and contour values.
           Do not even try to hint glyph if it has too many points/contours. */
        if (theGlyphT2K->pointCount >= TheFont(key)->maxPointCount || 
            theGlyphT2K->contourCount >= TheFont(key)->maxContourCount) {
            return -1;
        }

 	/* Allocate temporary memory.	*/
	key->theScratch =
		theScratch =  (perGlyphScratch*) GetPerFontMemory(key->theFont, perGlyphScratchSize);
	theScratch->glyphTemphBuffer = nil;

 	/*if (TheTrans(key)->methodToBeUsedOtherwise == outlineWillBeUsed) */	/*truetype gxPath*/
 	glyphElement = &theScratch->elementStorage[GLYPHELEMENT];/* fnt_ElementType	*/
	
	/*
	 *	If we have the glyph block, we DON'T need
	 *		temp buffers for the glyph outline
	*/
 		
	thetempGlyphSize=TheFont( key)->tempGlyphSize;
	theScratch->glyphTemphBuffer=
		glyphTemphBuffer =GetPerFontMemory(key->theFont, thetempGlyphSize);
	/* Check for memory allocation error.	*/
	if (theScratch->glyphTemphBuffer==0)
		goto badExit;
	key->fontBlockPtrs.compoundClone = 
			(tt_uint8*)theScratch->glyphTemphBuffer +  TheFont( key)->compoundCloneOffset;
					
	perGlyphPlusPermSize= sizeof(perGlyph) + TheFont( key)->permGlyphSize;
	key->theGlyph= theGlyph = (perGlyph*) GetPerFontMemory(key->theFont, perGlyphPlusPermSize);
	if (theGlyph==0)
		goto badExit;
		
	/* All memory is now allocated for the full processing.	*/
	theGlyph->permanentOutline.offset = sizeof(perGlyph);
	theGlyph->permanentOutline.ptr = (char*)theGlyph + theGlyph->permanentOutline.offset;
	/*glyphElement->contourCount = 0;	*/
	/*glyphElement->pointCount = 0;	*/
	
	
	/* Setup pointers within the glyphElement.	*/
	 
	SetElementPointers(
		glyphElement, 
		TheFont( key)->maxContourCount, 
		TheFont( key)->maxPointCount,
		theGlyph->permanentOutline.ptr, 
		theScratch->glyphTemphBuffer);
	{
			tt_int32 points,pointBytesShort,pointBytesLong,pointBytesLongWithPhantom;
			tt_int32 contourPointerBytes;
			/*Copy theT2K format to hinting format data.	*/
 			theGlyph->contourCount	= glyphElement->contourCount= theGlyphT2K->contourCount;
 				contourPointerBytes=theGlyph->contourCount * 2; /* 2 bytes per contour count.	*/
			theGlyph->pointCount	= glyphElement->pointCount= points= theGlyphT2K->pointCount;
			pointBytesShort= points *2;
			pointBytesLong= points*4;
			pointBytesLongWithPhantom=pointBytesLong+ 2 * 4; /* 2 phantom points for T2K.	*/
			 
			/* x->x, y->y	*/
			CopyBytes ( theGlyphT2K->x, glyphElement->x,  pointBytesLongWithPhantom); /* T2K-> tt	*/
			CopyBytes ( theGlyphT2K->y, glyphElement->y,  pointBytesLongWithPhantom); /* T2K-> tt	*/
			
			/* hinting engine has support for additional phantom points.
			   ensure they are not initialized with weird values */
			glyphElement->x[points+2] = 0;
			glyphElement->y[points+2] = 0;
			glyphElement->x[points+3] = 0;
			glyphElement->y[points+3] = 0;
			
			/*  x->ox,  y->oy  NOT needed because these points are overwritten by	*/
			
			/* Adjust the phantom points for a specific integral position */
			AdjustPhantoms(glyphElement->x,glyphElement->y,   points); 
			/*		a call CopyHintedOutlineToUnhintedOutline.	*/
 			CopyBytes ( theGlyphT2K->x, glyphElement->ox,  pointBytesLongWithPhantom); /* T2K-> tt	*/
			CopyBytes ( theGlyphT2K->y, glyphElement->oy,  pointBytesLongWithPhantom); /* T2K-> tt	*/
			/* hinting engine has support for additional phantom points.
			   ensure they are not initialized with weird values */
			glyphElement->ox[points+2] = 0;
			glyphElement->oy[points+2] = 0;
			glyphElement->ox[points+3] = 0;
			glyphElement->oy[points+3] = 0;
		
            /* make a copy of this so it is  up-to-date */
            /* if the hinting engine needs it. */
            /* Copy phantom points too. 
               Memory is allocated for maxPointCount points. So it is safe. */ 
            CopyBytes(theGlyphT2K->oox, glyphElement->oox,
                      pointBytesShort + 2*sizeof(short)); /* T2K-> tt */
            CopyBytes(theGlyphT2K->ooy, glyphElement->ooy,
                      pointBytesShort + 2*sizeof(short)); /* T2K-> tt */
			
			/* Copy the contour pointers.	*/
		 	/* glyphElement->sp	 <<<< theGlyphT2K->sp;	*/
			/*glyphElement->ep	<<<< theGlyphT2K->ep */	
			CopyBytes ( theGlyphT2K->sp, glyphElement->sp,  contourPointerBytes); /* T2K-> tt	*/
			CopyBytes ( theGlyphT2K->ep, glyphElement->ep,  contourPointerBytes); /* T2K-> tt	*/
			
			/* Copy the onCurve flags.	*/
			CopyBytes ( theGlyphT2K->onCurve, glyphElement->onCurve,  points); 
			/* MTE: the following fillbytes is not needed because it is done	*/
			/*		 inside RunGlyphProgram		*/	 
			/* FillBytes( theGlyph->f, pointCount, 0); */ /* T2K has no flags??	*/
					theGlyph->bounds.xMin=theGlyphT2K->xmin;
					theGlyph->bounds.yMin=theGlyphT2K->ymin;
					theGlyph->bounds.xMax=theGlyphT2K->xmax;
					theGlyph->bounds.yMax=theGlyphT2K->ymax;
					
 			glyphElement->bounds		= theGlyph->bounds;
 		/*   Setup horizontal and vertical.*/
		/*	glyphElement->hori			= theGlyph->hori= hori;	*/
		/*	glyphElement->vert			= theGlyph->vert= vert;	*/
			glyphElement->glyphIndex		= theGlyphT2K->gIndex;
 			key->theGlyph = theGlyph;

        if (setjmp(env) == 0) {
            /* Note that no recovery is neccessary - if something goes wrong we
               simply will not copy new outline */

            CreateGlyphElement(theGlyphT2K->gIndex, glyphElement, theGlyphT2K, 
                               key, TheTrans(key)->executeInstructions, &env);
		
            /* hinting instructions overwrite current dropout settings */
            aT2KScaler->glyphDropOutControlFlags = TheTrans(key)->globalGS.localParBlock.scanControl;
	 			
            /* Copy the onCurve flags. */
            CopyBytes (glyphElement->onCurve, theGlyphT2K->onCurve,  points);
            /* x->x, y->y */
            CopyBytes (glyphElement->x, theGlyphT2K->x, pointBytesLongWithPhantom); /* tt->T2K */
            CopyBytes (glyphElement->y, theGlyphT2K->y, pointBytesLongWithPhantom); /* tt->T2K */
        }
}
	 
 	goto exit;
badExit:
	theScalerResult=scalerError_Memory;
  exit:
	/* Release all memory allocated by this routine. */
	if (theScratch)
	{
		if (theScratch->glyphTemphBuffer)
			{
				ReleasePerFontMemory( key->theFont,theScratch->glyphTemphBuffer);	
 				if (theGlyph) 
					ReleasePerFontMemory( key->theFont,theGlyph);	
			}
		ReleasePerFontMemory( key->theFont,theScratch);	
	}
	return (theScalerResult);
 }
 
#endif
