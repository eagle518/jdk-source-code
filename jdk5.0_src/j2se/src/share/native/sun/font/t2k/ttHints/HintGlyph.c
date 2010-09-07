/*
 * @(#)HintGlyph.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

#ifdef ENABLE_TT_HINTING

local void CreateGlyphElement(tt_int32 glyphIndex,fnt_ElementType *anOutline,
	GlyphClass  *t2kGlyph,fsg_SplineKey* key,   boolean useHints);
local void CreateGlyphElement(tt_int32 glyphIndex,
fnt_ElementType *anOutline,
GlyphClass  *t2kGlyph, fsg_SplineKey* key,   boolean useHints)
{
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
			if (theFont->expectFPGM && !theFont->fontProgramRan) 
			{	RunFontProgram(key, (voidFunc)nil);
				theFont->fontProgramRan = true;
			}
			if (theFont->cvtCount && !theTrans->cvtHasBeenScaled) {	
				PrepareTheCVT(key, MultiplyDivide(theTrans->globalGS.cvtScale, 64, theFont->emResolution));
				theTrans->cvtHasBeenScaled = true;
			}
			if (theFont->expectPREP && !theTrans->preProgramRan)
			{	RunPreProgram(key, &theTrans->transState, (voidFunc)nil);
				theTrans->preProgramRan = true;
			}
		}
	} 
	{
		transformState* state= & (theTrans->transState); /* point to required mapping.;	*/
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
 		 	RunGlyphProgram( key, anOutline, state, numInstructions,
					 instructionPtr, hintingAComposit);
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
		
 

int TTScalerHintGlyph(
 	GlyphClass 	*theGlyphT2K,	/* Original T2K glyph data as input	*/
 	T2K 		* aT2KScaler, 			/* T2K scaler data			*/
 	void  		**scHintPtrVoid, /* use null when no image is to be created, otherwise
 								   point to the tsiScanConv ptr to return image. If successful,
 								   the pointer remains, for failure the ptr is set to zero.*/
 	tt_int32		applyStyle			/* If zero, then must not apply applicable style. */
   	)  
{
 	tsiScanConv **scHintPtr =			/* if 0, then don't create image */
 			(void *) scHintPtrVoid;		/* otherwise setup key variables and image.. */
 											
 	tsiScanConv *scHint;
 	fsg_SplineKey		*key, keyData ;  	/* Setup with non-glyph data.	*/
	scalerError		theScalerResult= scalerError_NoError;
	/* Specify the 3 possible allocated memory blocks.	*/
		perGlyphScratch*	theScratch=0L;
		void 	*glyphTemphBuffer=0L;
		perGlyph*			theGlyph=0L;
	fnt_ElementType*	glyphElement;
	memoryContext		memContext;
 	size_t				perGlyphScratchSize=sizeof(perGlyphScratch);
 	size_t				thetempGlyphSize,perGlyphPlusPermSize;
  	 perTransformation   *aPerTrans = 
 		(perTransformation  *)  aT2KScaler->TTHintTranData; ;
	
	if (scHintPtr)
	{
		scHint= *scHintPtr; /* point to the tsiScanConv record (save it)*/
		*scHintPtr=(void *) 0L; /* set pointer to zero, for default failure.*/
	}
 	// to do special stuff only for non - TT fonts
	if ( (aT2KScaler->font->T1 != NULL ) || ( aT2KScaler->font->T2 != NULL )) {

	    if ( applyStyle && (( aT2KScaler->font)->StyleFuncPost != NULL ))
	        ApplyPostStyle ( theGlyphT2K, aT2KScaler);
	    return (0);

	}
 	/* Init the key before the font program.	*/
 	key=&keyData;
 	InitTheKeyByTrans(key,  aPerTrans );
 	
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
			/*  x->ox,  y->oy  NOT needed because these points are overwritten by	*/
			
			/* Adjust the phantom points for a specific integral position */
			AdjustPhantoms(glyphElement->x,glyphElement->y,   points); 
			/*		a call CopyHintedOutlineToUnhintedOutline.	*/
 			CopyBytes ( theGlyphT2K->x, glyphElement->ox,  pointBytesLongWithPhantom); /* T2K-> tt	*/
			CopyBytes ( theGlyphT2K->y, glyphElement->oy,  pointBytesLongWithPhantom); /* T2K-> tt	*/
		
 			/*  make a copy of this so it is  up-to-date */
 			/* if the hinting engine needs it.	*/
			CopyBytes ( theGlyphT2K->oox, glyphElement->oox,  pointBytesShort); /* T2K-> tt	*/
			CopyBytes ( theGlyphT2K->ooy, glyphElement->ooy,  pointBytesShort); /* T2K-> tt	*/
			
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
			 
			CreateGlyphElement(theGlyphT2K->gIndex,glyphElement,theGlyphT2K, key, TheTrans(key)->executeInstructions);
 			
		if (applyStyle&& ( ( (aT2KScaler->font)->StyleFuncPost) !=0)){
			
			{
	 			StyleFuncPostPtr sfp=   (aT2KScaler->font)->StyleFuncPost;

				/* typically, this will be: 	tsi_SHAPET_BoldItalic_GLYPH_Hinted  */
 				(*sfp)( 
					glyphElement->contourCount,	/* number of contours in the character */
					glyphElement->pointCount,      	/* number of points in the characters + 0 for the sidebearing points */
					glyphElement->sp,	       	/* sp[contourCount] Start points */
					glyphElement->ep,  	       	/* ep[contourCount] End points */
				 	glyphElement->x,
				 	glyphElement->y,
				 	glyphElement->ox,
				 	glyphElement->oy,
					aT2KScaler->mem, 
					aT2KScaler->xPixelsPerEm16Dot16,  aT2KScaler->yPixelsPerEm16Dot16, 
	 			 			/*Parameters for BoldOrientation. */
	 			 			theGlyphT2K->curveType, 
							theGlyphT2K->onCurve,	
							&aT2KScaler->theContourData,
					 ( (aT2KScaler->font)->params)  );
						
			}
		}
			
			theGlyph->scanControl	= TheTrans(key)->globalGS.localParBlock.scanControl;
#ifdef  UIDebug
		/* Are we doing a partial hinting for debug? */
		{
			int glyphIndex= glyphElement->glyphIndex;
			if (glyphIndex==78)
			{
			
				unsigned int theControl;
				tt_int32 *x, *y;
				unsigned char *onCurve;
							int k=0;
				theControl= TheGlyph(key)->scanControl;
				onCurve= glyphElement->onCurve;
				x=   glyphElement->x;
				y=   glyphElement->y;
				k++;
			}
		}
		RestorePartialResult(glyphElement->x,glyphElement->y, glyphElement->onCurve, points+2); 	
#endif
	 			
			/* if there is a pointer, then we want to create an image. If successful, then
				we setup the return pointer. 
			*/
			 
 	/*------ Obtain the glyph's gxBitmap if necessary ---------------------------------*/
	if (1 && (scHintPtr != 0) &&  ( glyphElement->contourCount > 0 ))
	{
		int error=0;
		scalerBitmap glyphImageData;				
		scalerBitmap *glyphImage=&glyphImageData;			
		error = fs_CalculateBounds( key, glyphElement, glyphImage );	
		if(error ) 
			goto exit; /* its an error, so let t2k scanner do it the way it wants. */
		fs_FindBitMapSize4(key, glyphElement, glyphImage);
		theScratch->scanConvertScratch.ptr = nil;
		if (TheTrans(key)->methodToBeUsedOtherwise == outlineWillBeUsed)
			theScratch->scanConvertScratch.ptr = 
				  GetPerFontMemory(key->theFont,  theScratch->scanConvertScratch.size);
					 
			
		/*
		  * In allocating the memory for the gxBitmap, we take into account that fact that the client may have supplied
		  * a ScanLinFunction in the scaler context.  The presence of this function means that the client does not
		  * want the gxBitmap in a permanent scalerBitmapBlock, rather the gxBitmap should be returned a scanline
		  * at a time.  In that case a theScratch block, one scanline high (as computed with the bitmapsize routine)
		  * is allocated (and freed later).
		  */
		theScratch->actualBitmap.ptr = nil;
		if (theScratch->actualBitmap.size)
			theScratch->actualBitmap.ptr = 
				GetPerFontMemory(key->theFont,theScratch->actualBitmap.size);
			
		/*
		  * We've allocated the actual permanent gxBitmap block that will be returned to the client.
		  * The scan converter is invoked.  Then the theScratch blocks (if any) are disposed of.
		  */
		 	/* Here we try to allocate the dropout theScratch memory, it's OK if it fails */
		theScratch->dropoutScratch.ptr = nil;
		if (theScratch->dropoutScratch.size)
			theScratch->dropoutScratch.ptr = 
				GetPerFontMemory(key->theFont,theScratch->dropoutScratch.size);
					 
		error = fs_ContourScan3(key, glyphElement,/* theGlyphRecord, */
					glyphImage);
#if 1
		if( error==scalerError_NoError)
			{
				/* We have a successful result, so setup the return values. */				
				/* And transfer ownership of the image buffer to the caller. */
				scHint->right = glyphImage->bounds.xMax;
				scHint->left = glyphImage->bounds.xMin;
				scHint->bottom  = 	glyphImage->bounds.yMax;
				scHint->top= 	glyphImage->bounds.yMin;


				scHint-> fLeft26Dot6= glyphImage->topLeft.x>>10; /* fixed to 26.6 */
				scHint-> fTop26Dot6= glyphImage->topLeft.y>>10; /* fixed to 26.6 */
				
				scHint->rowBytes = glyphImage->rowBytes;
				scHint->baseAddr =theScratch->actualBitmap.ptr; 
					theScratch->actualBitmap.ptr=0L; /* scratch no longer owns it
														and is not responsible */
				*scHintPtr= scHint;
			}
		else
			theScalerResult=0; /* no error, let t2k handle the result. */
			/* note: this pathleads to a release of the memory, immediately following.
			   however, the routine itself does not return an error. */
			  
				
#endif		

		/* Cleanup excess allocations. */		
		ReleasePerFontMemory(key->theFont,theScratch->dropoutScratch.ptr);
 			theScratch->dropoutScratch.ptr = nil;
 			
		/* in the case where the bitmap is being returned to the client,
				the bitmap is never released because it has been set to zero.
				The font caching mechanism is required to eventually release it. 
		*/
		ReleasePerFontMemory(key->theFont,theScratch->actualBitmap.ptr);
 			theScratch->actualBitmap.ptr = nil;
 			 
		ReleasePerFontMemory(key->theFont,theScratch->scanConvertScratch.ptr);
 			theScratch->scanConvertScratch.ptr = nil;
  	}
 			

			/* Copy the onCurve flags.	*/
			CopyBytes ( glyphElement->onCurve,theGlyphT2K->onCurve,  points); 		
			/* x->x, y->y	*/
	 		CopyBytes ( glyphElement->x, theGlyphT2K->x,pointBytesLongWithPhantom); /* tt-> T2K	*/
 			CopyBytes ( glyphElement->y, theGlyphT2K->y,   pointBytesLongWithPhantom); /* tt-> T2K	*/
#ifdef UIDebug
{
	int i=1;
 	if (gridMagnify && (glyphElement->glyphIndex==25) )
 		i=i;
 }
#endif
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
