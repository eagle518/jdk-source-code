/*
 * @(#)HintFont.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 
#ifndef	NewFontIncludes
#define	NewFontIncludes

#include "Hint.h"
 
extern perScaler globPerScaler;
 
 /* Allocate a block from the scaler.  This routine is used internally.*/
sfntClass 		*GetSFNTClassFromPerFont(perFont *aPerFont);
tsiMemObject 	*GetMemObjectFromPerFont(perFont *aPerFont);
T2K 		 	*GetT2KFromPerFont(perFont *aPerFont);

    /* Allocate a block from the scaler.  This routine is used internally.*/
  tsiMemObject * GetPerFontMemoryAllocator(perFont	*aPerFont) ; 		
void * GetPerFontMemory(perFont	*aPerFont, size_t memSize); 
void ReleasePerFontMemory(perFont	*aPerFont,void *block) ; 
/* Key Processing.
 	The whole "key" structured  appears to be related to the fact that the
 		original developer didn't know how to create forward referencing in
 		data structures. 
 	Therefore a "key" was developed that kept track
 		of these interlinked data structures.  
 	Its cumbersome, and should someday be eliminated.
*/
  
	/* Initialize a redundant data structure of pointers. */
	void InitTheKeyByScaler( fsg_SplineKey *key);
	
	/* Initialize a redundant data structure of pointers. */
	void InitTheKeyByFont(  fsg_SplineKey *key, perFont * aPerFont);

	/* Initialize a redundant data structure of pointers. */
 	void InitTheKeyByVary(fsg_SplineKey *key, perVariation * aPerVary);
 
	/* Initialize a redundant data structure of pointers. */
	void InitTheKeyByTrans(fsg_SplineKey *key, perTransformationPtr aPerTran);



/* When a new TTHint font is opened by T2K, additional setup is required
	by the TTHint code.
*/

	scalerError NewTTSHintcalerFont(T2K *	aT2KScaler, perFont **aPerFontPtr);

	/*  Call this once when the font is no longer needed.. */
	void TTScalerCloseFont(perFont *aPerFont);



/* The variation record is very simple, because we don't allow variations. */
	void  TTScalerNewVariation1Dot1( 
		perFont  *aPerFontContext, perVariation **aPerVaryContext);
 
	/* Bring up the default variation- it does nothing. */
	void TTScalerNewVariationDefault(
		perFont  *aPerFontContext,  perVariation **aPerVaryContext);

	/* Call this once at program end to cleanup the context.
	//	Currently, there is no cleanup required.*/
	
	void TTScalerCloseVary(perVariation *aPerVaryContext);
 
 
 		/* High level calls from T2K to prepare for TrueType Hinting */

 	void NewTTHintFontForT2K(T2K *	aT2KScaler);
	void ReleaseTTHintFontForT2K(T2K *	aT2KScaler);

#endif
