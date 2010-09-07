/*
 * @(#)HintTran.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * TTHintTran.c, 
 * Replaces FontScaler.c's TTScalerNewTransform. Also adds a Close routine.
 * It also splits the transform routine into an allocation part and 
 * the part that sets up the transform values.
 */

 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */

#include "HintTran.h" 

/* Spot Size must be set before creating a transformation. It can be restored afterwards. */
/* The transformation routine */
long	useSpotSize=USESPOTSIZE; 			/* Set Default Spot Size */


#ifdef ENABLE_TT_HINTING

/* MTE: This routine was originally designed to perform many activities */
/*		1. Still Done.	Allocate maximal storate ( twilight, storage, ...) */
/*		2. Still Done.	Allocate maximal storate ( twilight, storage, ...) */
#define kMinimumPPEMWhereHintsStillWork		9
 	 		
/* Allocate data structures for the transform, including structures needed */
/*		by the hint machine.	 		 */
static perTransformation  * TTHintTransformAllocation(  perFont * aPerFont )
{
	perVariation * aPerVari=&aPerFont->theVary	;			/* Specify the context as a variation */
	fsg_SplineKey		key;			/* Scaler internal execution state */
	tt_int32	       	perTransBlockSize;
	perTransformation*	theTrans;
 	tt_int32		sbitSpaceSize = 0,sbitHeadSize = 0;
	perTransformation   *localPerTransform;
 	tt_int32 cvtBytes, storageBytes, twilightBytes;
 	scalerError		theScalerResult= scalerError_NoError;
        tt_int32 stackBytes;
   
     
	/*  Allocate necessary storage.
	 	Also, setup pointers to each of the buffers. The tthinting/GX version
	 		Did'nt setup the pointers (only offsets. Why? Because in GX
	 		pointer blocks move around and need to be reset at various times.	 
	*/
		InitTheKeyByVary(&key, aPerVari);

#ifdef SBIT_SUPPORT
			sbitSpaceSize = (TheFont(&key)->usefulBitmaps)? sbit_CalculateTransformSpaceNeeded(&memContext): 0;
			sbitHeadSize = (TheFont(&key)->usefulBitmaps)? sizeof(sbitTransHeader): 0;
#endif
 	 	perTransBlockSize = 
	 	   sizeof(perTransformation)
				+(cvtBytes =  TheFont(&key)->cvtCount * (tt_int32)sizeof(F26Dot6))
				+(storageBytes=  TheFont(&key)->storageSize)
				+(twilightBytes=  TheFont(&key)->twilightZoneSize)
				+(stackBytes=  TheFont(&key)->stackSize)
				+ sbitSpaceSize
				+ sbitHeadSize;
		/*theTrans->twilightBytes=twilightBytes;  // MTE: we need these to clear to zero. */
#ifdef check_cvt_access_direction
			size += TheFont(&key)->cvtCount * sizeof(char);
#endif
				
	 	theTrans = (perTransformation*) 
	 		GetPerFontMemory( key.theFont, perTransBlockSize);
	     theTrans->storageBytes=storageBytes; /* MTE: we need these to clear to zero. */

	    theTrans->xPerVaryContext = (struct	perVariation * )aPerVari;
 	 	theTrans->scaledCvt.offset = sizeof(perTransformation);
		theTrans->storage.offset =  theTrans->scaledCvt.offset + cvtBytes;
 		theTrans->stackZone.offset = theTrans->storage.offset + storageBytes;
				theTrans->stackBytes=stackBytes;
		theTrans->twilightZone.offset = theTrans->stackZone.offset + stackBytes;
		theTrans->sbit.offset = theTrans->twilightZone.offset + twilightBytes;
		theTrans->sbitHead.offset = theTrans->sbit.offset + sbitSpaceSize;

#ifdef check_cvt_access_direction
				theTrans->cvtFlags.offset = theTrans->sbitHead.offset + sbitHeadSize;
				FillBytes((char*)theTrans + theTrans->cvtFlags.offset, TheFont(&key)->cvtCount, 0);
#endif
	
		
	 	/* MTE: now setup the pointers, because they will not change. */
 		/*		We use new data structures so that the debugger can access the values. */
  				theTrans->scaledCvt.ptr
  						=theTrans->scaledCvtTransPtr 
  									= (F26Dot6 *)( (char *) theTrans + theTrans->scaledCvt.offset);
  									
   				theTrans->storage.ptr
  						=theTrans->storageTransPtr
  									= (F26Dot6 *)( (char *)  theTrans + theTrans->storage.offset);
  									
 	   			theTrans->twilightZone.ptr
  						=theTrans->twilightTransPtr
  									=(F26Dot6 *) ( (char *) theTrans + theTrans->twilightZone.offset);
  			
 	   			theTrans->stackZone.ptr
  						=theTrans->stackTransPtr
  									=(F26Dot6 *)( (char *)  theTrans + theTrans->stackZone.offset);

 				/* Setup the remaining allocations. */
  				theTrans->sbit.ptr = (char*)theTrans + theTrans->sbit.offset;
				theTrans->sbitHead.ptr = (char*)theTrans + theTrans->sbitHead.offset;
#ifdef check_cvt_access_direction
					theTrans->cvtFlags.ptr = (char*)t + t->cvtFlags.offset;
#endif
						
	return( theTrans );		/* return new font cookie. */
 }
 	 		
 
void TTScalerTTHintTran(
 	 const scalerTransform *theTransform,	/* Specify a Input Transformation? */
	 perFont *aPerFont,						/* Specify the context  */
 	 perTransformation  *theTrans 			/* point to transform block. */
 	) 	 
{
	perVariation * aPerVari=&aPerFont->theVary	;			/* Specify the context as a variation */
	fsg_SplineKey		key;			/* Scaler internal execution state */
   	perTransformation   *localPerTransform;
      
  	InitTheKeyByVary(&key, aPerVari);
  	FillBytes((char*)theTrans + theTrans->twilightZone.offset, TheFont(&key)->twilightZoneSize, 0);
	/* MTE: Setup some flags and control variables */
	/*	make a guess as to what they should be.. */
 	theTrans->executeInstructions	= true; 
 			/* MTE: always true? BitTestToBoolean(theTransform->flags & applyHintsTransform); */
	theTrans->returnDeviceMetrics= false; 
			/* MTE: always false??BitTestToBoolean(theTransform->flags & deviceMetricsTransform); */
	theTrans->preProgramRan 	= false;
	theTrans->cvtHasBeenScaled 	= false;
	theTrans->verticalOrientation = false; 
		/*  MTE: What should this be?BitTestToBoolean(theTransform->flags & verticalTransform); */
	theTrans->methodToBeUsedIfPossible = outlineWillBeUsed;
	theTrans->methodToBeUsedOtherwise = outlineWillBeUsed;

	/*
	 *	Set baseMap to the concatenation of pointSize € resolution € userMatrix
	*/
	theTrans->pointSize = theTransform->pointSize;
	theTrans->pixelDiameter	= Magnitude(theTransform->spotSize.x, theTransform->spotSize.y);
	
	
	ResetMapping(&theTrans->transState.baseMap);
	theTrans->transState.baseMap.map[0][0] = 
		MultiplyDivide(theTransform->pointSize, theTransform->resolution.x, FIXEDPOINTSPERINCH);
	theTrans->transState.baseMap.map[1][1] = 
		MultiplyDivide(theTransform->pointSize, theTransform->resolution.y, FIXEDPOINTSPERINCH);
		
	MapMapping(&theTrans->transState.baseMap,( const gxMapping *) theTransform->fontMatrixPtr );
	theTrans->globalGS.localParBlock.scanControl = 0;  /* in case we don't execute instructions */
	if( theTrans->transState.baseMap.map[1][1] > ff(254) ) 
		theTrans->imageState = 0xFF;
	else 
		theTrans->imageState = theTrans->transState.baseMap.map[1][1] >> 16;
	
	if (theTrans->executeInstructions)
	{	
		boolean b1,b2,b2a;
		int theMinVal,minFix;
		/* For now, turn off hinting for everything except stretching (without flipping) */
		{
			T2K_TRANS_MATRIX tm;
			tm.t00= (*(theTransform->fontMatrixPtr)).map[0][0];
			tm.t01= (*(theTransform->fontMatrixPtr)).map[0][1];
			tm.t10= (*(theTransform->fontMatrixPtr)).map[1][0];
			tm.t11= (*(theTransform->fontMatrixPtr)).map[1][1];
 		    if (  IsUnhintableMatrix( &tm  ) )
				goto turnOffHinting;
		}

		/* Run numerous tests to see if we should turn off hinting. */
		b1= NonInvertibleMapping(&theTrans->transState.baseMap);
		if (b1)
			goto turnOffHinting;
				
		b2a= BitTestToBoolean(TheFont(&key)->fontFlags & USE_INTEGER_SCALING);
		b2= DecomposeMapping(&theTrans->transState, b2a);
		if (b2)
			goto turnOffHinting;
			
			
 	 	
 	 	theMinVal = TheFont(&key)->minimumPixPerEm; 
 	 	if (theMinVal> kMinimumPPEMWhereHintsStillWork) 
		    theMinVal=kMinimumPPEMWhereHintsStillWork ;  
		minFix=ff(theMinVal)-(0x8000L); /* adjust for fixed point rounding. */
		if (			/* MTE: no perspective, ever ! MxFlags(&theTrans->transState.baseMap) == 	perspectiveState || */
				(theTrans->transState.stretchBase.map[0][0] < minFix )
					||
				(theTrans->transState.stretchBase.map[1][1] < minFix) 
			)
			{
			turnOffHinting:
				theTrans->globalGS.localParBlock.scanControl = 0x101ff;					/* hard-code dropout control ON */
				theTrans->returnDeviceMetrics = theTrans->executeInstructions = false;	/* Do nothing. */
 				goto exit;
			}
			
 	 	
 			if (theTrans->transState.stretchBase.map[0][0] != theTrans->transState.stretchBase.map[1][1])
				theTrans->imageState |= STRETCHED;
			if (theTrans->transState.baseMap.map[1][0] || theTrans->transState.baseMap.map[0][1])
				theTrans->imageState |= ROTATED | STRETCHED;
		
		 	theTrans->globalGS.instrDefCount 	= TheFont(&key)->IDefCount;
			theTrans->globalGS.pointSize		= FixedRound(theTrans->pointSize);
#ifdef use_engine_characteristics_in_hints
				theTrans->globalGS.engine[kGreyEngineDistance]	= 0;
				theTrans->globalGS.engine[kBlackEngineDistance]	= FixedToF26Dot6( (FIXEDSQRT2 - theTrans->pixelDiameter) );
				theTrans->globalGS.engine[kWhiteEngineDistance]	= -theTrans->globalGS.engine[kBlackEngineDistance];
				theTrans->globalGS.engine[kIllegalEngineDistance]	= 0;
#endif
			theTrans->globalGS.variationCoordCount			= TheFont(&key)->axisCount;
			theTrans->globalGS.hasVariationCoord			= TheVari(&key)->hasStyleCoord;
			
			{	register fixed scaleX = theTrans->transState.stretchBase.map[0][0];
				register fixed scaleY = theTrans->transState.stretchBase.map[1][1];
				theTrans->globalGS.cvtScale =/*  MTE: Maximum(scaleX, scaleY); */
						scaleX>scaleY ? scaleX :scaleY;
			}
			SetGlobalGSDefaults(&theTrans->globalGS);  	 
 	}
   exit:;
  }
 
 
 
  
/* Create a gx mapping from the t2k matrix */
static void SetupGXMatrixFromT2K(gxMappingX *gxMap, T2K_TRANS_MATRIX *trans)
{
	ResetMapping((gxMapping *)gxMap);/* MTE: Start with identity */
	gxMap->map[0][0] = trans->t00;
	gxMap->map[0][1] = trans->t01;
	gxMap->map[1][0] = trans->t10;
	gxMap->map[1][1] = trans->t11;
}
			

/* Create a TT Transform from a transform matrix */
static void SetupScalerTransform(
 	scalerTransform *theTransform,
 	TrueTypeTransformData *ttd)
 {
 	theTransform->fontMatrixPtr= &theTransform->fontMatrixData;
 	SetupGXMatrixFromT2K(theTransform->fontMatrixPtr,&ttd-> trans);
	theTransform->flags= applyHintsTransform;	 
	theTransform->pointSize=   ttd->pointSize;
#ifdef use_engine_characteristics_in_hints
	theTransform->spotSize.x= useSpotSize;  
	theTransform->spotSize.y= useSpotSize;  
#else
	theTransform->spotSize.x= USESPOTSIZE;  
	theTransform->spotSize.y= USESPOTSIZE;  
#endif
	theTransform->resolution.x= ff(ttd->xRes);  
	theTransform->resolution.y= ff(ttd->yRes); 
 }

/* Create a TT Transform from a truetype hinting transform matrix */
static T2K_TRANS_MATRIX DefaultTTDTransform={0x10000,0,0,0x10000};
static void SetupScalerDefaultTTD(TrueTypeTransformData *ttd)
{
 		(ttd->trans)=DefaultTTDTransform;
		ttd->xRes=72;
		ttd->yRes=72;
		ttd->pointSize=12<<16;
}  

  
/* Bring up the default variation- it does nothing. */
static void TTScalerTTHintTranDeep(
		perFont * aPerFont,						/* Specify the context  */
		perTransformation  *aPerTransContext,
		TrueTypeTransformData  *ttd
		)
{
	scalerTransform  theScalerTransform;
 	SetupScalerTransform(&theScalerTransform,ttd);
	TTScalerTTHintTran(&theScalerTransform, aPerFont, aPerTransContext);
}


/* Bring up the default variation- it does nothing. */
void TTScalerTTHintTranDefault(
		perFont * aPerFont,						/* Specify the context  */
		perTransformation  *aPerTransContext)
{
	scalerTransform  theScalerDefaultTransform;
	TrueTypeTransformData  ttd;
	SetupScalerDefaultTTD(&ttd);
	TTScalerTTHintTranDeep( aPerFont,aPerTransContext,&ttd);
}

/* Call this once at program end to cleanup the context. */
/*	Currently, there is no cleanup required. */
void TTScalerCloseTransform(perTransformation *aPerTransContext)
{
	if (aPerTransContext)
	{
 		perFont  *aPerFontContext;
		perVariation *aPerVaryContext;
		aPerVaryContext= (perVariation *) aPerTransContext->xPerVaryContext;
		aPerFontContext= (perFont *) aPerVaryContext->xPerFontContext;
 		ReleasePerFontMemory( aPerFontContext, aPerTransContext);
 	}
}
 
/* Now the interface routines which are called by T2K */
void InitTTHintTranForT2K(T2K *	aT2KScaler)
{
	perFont  *aPerFont;
 	perTransformation  * aPerTrans;
	aPerFont=  (perFont *) aT2KScaler->TTHintFontData;
	if (aPerFont)
	{
		/* If we have a font, then we can add a transform. */
		aPerTrans= TTHintTransformAllocation(  aPerFont ); /* allocate memory */
		aT2KScaler->TTHintTranData= (perTransformation  *) aPerTrans;
		TTScalerTTHintTranDefault( aPerFont, aPerTrans); /* Setup a default transform. */
 	}
}

void NewTTHintTranForT2K(T2K *	aT2KScaler)
{
	perFont  *aPerFont;
 	perTransformation  * aPerTrans;
 	TrueTypeTransformData *ttd= &aT2KScaler->ttd;

	aPerFont=  (perFont *) aT2KScaler->TTHintFontData;
	if (aPerFont)
	{
		/* If we have a font, then we can add a transform. */
		aPerTrans=  (perTransformation  *)  aT2KScaler->TTHintTranData; 
 		TTScalerTTHintTranDeep( aPerFont,aPerTrans,&aT2KScaler->ttd);		 
 	}
}
/* Called to release memory for transformation and transform structures for hinting */
void ReleaseTTHintTranForT2K(T2K *	aT2KScaler)
{
 	perTransformation  * aPerTrans=(perTransformation  *)  aT2KScaler->TTHintTranData;
	if (aPerTrans!=0)
	{
		TTScalerCloseTransform(aPerTrans);
 		aT2KScaler->TTHintTranData=0;
	}
}

#endif























