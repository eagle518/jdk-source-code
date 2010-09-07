#if 1
/*
 * @(#)HintScan.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 
/*	 
**
**		Implementation of the routines dispatched to by the Canonical Scaler when the
**	TrueType scaler is being used.
**
**	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.
**
**	Change History:
**
**			03/11/94	rwb [1149958] Fixed 2 memory leaks in TTScalerNewGlyph
**			02/07/94	rwb	add CalculateBounds, change findBitmapSize to 4
**			02/02/94	rwb	initialize GS->instrDefCount from TheFont->IDefCount
**			01/12/94	rwb  support simplification of overlapping contours for type one.  Pass through scalerStreamTypeFlag
**						in ScalerStreamType.
**			12/01/93	rwb  set Trans->imageState to y-pixels-per-em so that SCANCTRL instruction will work properly
**			11/24/93	rwb	Set drop-out-control to 0 for unhinted case.
**			9/20/93 rwb add SWAP's on acnt info
**			9/../93   rwb support acnt table
**	<2>		8/20/92	irr new transformation matrix for embedded bitmaps
**	<1>		03/15/92 ABB	Created
*/

/* Needed to define intptr_t */
#include "gdefs.h"

#include "HintScan.h"
 
  
 /*
 *	This constant is used to set a mimum ppem, above which we don't turn-off hints.
 *	even if the font's minPPEM is greater. The problem is that some fonts set their minPPEM to a huge value
 *	in an attempt to fool QuickDraw into using scaled bitmaps at screen sizes. The GX scaler wants to turn
 *	hints off at very small sizes to avoid them blowing up because projection and/or freedom vectors
 *	go astray. This minimum is a guess that says the hints won't start crapping out until the glyphs are
 *	smaller than this value, even if the font's claims the hints suck at a larger size.
*/
#define kMinimumPPEMWhereHintsStillWork		29


#define ALIGN(x, asize) (((uintptr_t)(x) + ((asize) - 1L)) & ~((asize) - 1L))
 
#define kSbitOnlyUPEM	4614		/* globally defined for all sbit-only fonts */




/*
 * Use various spline key values to determine if dropout control is to be activated
 * for this glyph, and if so what kind of dropout control.
 * The use of dropout control mode in the scan converter is controlled by 3 conditions.
 * The conditions are: Is the glyph rotated?, is the glyph stretched?,
 * is the current pixels per Em less than a specified threshold?
 * These conditions can be OR'd or ANDed together to determine whether the dropout control
 * mode ought to be used.
 * 
 * Six bits are used to specify the joint condition.  Their meanings are:
 * 
 * BIT		Meaning if set
 * 8		Do dropout mode if other conditions don't block it AND
 * 			pixels per em is less than or equal to bits 0-7
 * 9		Do dropout mode if other conditions don't block it AND
 * 			glyph is rotated
 * 10		Do dropout mode if other conditions don't block it AND
 * 			glyph is stretched
 * 11		Do not do dropout mode unless ppem is less than or equal to bits 0-7
 * 			A value of FF in 0-7  means all sizes
 * 			A value of 0 in 0-7 means no sizes
 * 12		Do not do dropout mode unless glyph is rotated	
 * 13		Do not do dropout mode unless glyph is stretched
 * 			
 * In other words, we do not do dropout control if:
 * No bits are set,
 * Bit 8 is set, but ppem is greater than threshold
 * Bit 9 is set, but glyph is not rotated
 * Bit 10 is set, but glyph is not stretched
 * None of the conditions specified by bits 11-13 are true.
 * 
 * For example, 0xA10 specifies turn dropout control on if the glyph is rotated providing
 * that it is also less than 0x10 pixels per em.  A glyph is considered stretched if
 * the X and Y resolutions are different either because of the device characteristics
 * or because of the transformation matrix.  If both X and Y are changed by the same factor
 * the glyph is not considered stretched.
 * 
 */
#ifdef UIDebug					
 extern int isForceDropoutFlag;
#endif

tt_int32 fs_dropOutVal( fsg_SplineKey *key )
{
	 tt_int32 condition = TheGlyph(key)->scanControl;
	 tt_int32 imageState = TheTrans(key)->imageState;
	 
#ifdef UIDebug					
		if(isForceDropoutFlag)
			return(1);
#endif

#ifdef AllowDropoutControl
	{
		DropoutControlFlags 
			masterDropOutControlFlags=
				 (key->memContext)->aT2KScaler->masterDropOutControlFlags;
		/* See if transforms requires disabling of dropout for hint scanner. */	 			
		if( masterDropOutControlFlags & 
				( DisableScannerDropoutFlag<<HintDropoutShift))
			return(0);
		/* See if transforms requires enabling of dropout for hint scanner. */	 			
		if( masterDropOutControlFlags & 
					( EnableScannerDropoutFlag<<HintDropoutShift))
 			return(1);
	}
#endif

 
	if (!(condition & 0x3F00) ) 
		return 0;
	if ((condition & 0xFFFF0000) == NODOCONTROL )
		return 0;
	if ((condition & 0x800) && ((imageState & 0xFF) > (condition & 0xFF)) ) 
		return 0;
	if ((condition & 0x1000) && !(imageState & ROTATED) ) 
		return 0;
	if ((condition & 0x2000) && !(imageState & STRETCHED) ) 
		return 0;
	if ((condition & 0x100) && ((imageState & 0xFF) <= (condition & 0xFF) )) 
		return condition;
	if ((condition & 0x100) && ((condition & 0xFF) == 0xFF )) 
		return condition;
	if ((condition & 0x200) && (imageState & ROTATED) ) 
		return condition;
	if ((condition & 0x400) && (imageState & STRETCHED) ) 
		return condition;
	return 0;
}
 

tt_int32 fs_ContourScan3( 
fsg_SplineKey *key, 
const fnt_ElementType* charData,  
/* const scalerGlyph* theGlyphRecord, */
scalerBitmap* glyphImage)
{
	sc_BitMapData			*bitRecPtr;
	sc_GlobalData			*scPtr;
	tt_int32 					scanControl;
	tt_int16					lowBand, highBand;
	tt_int16 					nx, ny;
	tt_int32					error;

	bitRecPtr = &TheGlyph(key)->bitMapInfo;

	scPtr			= &TheScratch(key)->globalScanData;
	
	nx 				= bitRecPtr->xMax - bitRecPtr->xMin;
	if (nx == 0 ) 
		++nx;
	
	scanControl = fs_dropOutVal( key );
	
 #if 0
	/* set up banding.  Assume highBand is 1 higher than highest scanrow <10>  */	
	highBand = theGlyphRecord->bandingTop;	
	lowBand  = theGlyphRecord->bandingBottom;
		
	/* 	If bandingTop < bandingBottom clip there is no banding by convention  */
	if (highBand <= lowBand )
	{
		highBand = bitRecPtr->yMax;
		lowBand = bitRecPtr->yMin;
	}
	/* check for out of bounds band request  							<10> */
	if (highBand > bitRecPtr->yMax ) highBand = bitRecPtr->yMax;
	if (lowBand  < bitRecPtr->yMin )  lowBand = bitRecPtr->yMin;
#else
		highBand = bitRecPtr->yMax;
		lowBand = bitRecPtr->yMin;
#endif	
	/*	11/16/90 rwb - We now allow the client to turn off DOControl by returning a NIL pointer
		to the memory area used by DOC.  This is done so that in low memory conditions, the
		client can get enough memory to print something.  We also always turn off DOC if the client
		has requested banding.  Both of these conditions may change in the future.  Some versions
		of TT may simply return an error condition when the NIL pointer to memoryarea 7 is
		provided.  We also need to rewrite the scan converter routines that fill the gxBitmap 
		under dropout conditions so that they use noncontiguous memory for the border scanlines
		that need to be present for doing DOC.  This will allow us to do DOC even though we are
		banding, providing there is enough memory.  By preflighting the fonts so that the request
		for memory for areas 6 and 7 from findBitMapSize is based on actual need rather than
		worse case analysis, we may also be able to reduce the memory required to do DOC in all
		cases and particulary during banding.
	*/
	
	/* inhibit DOControl if banding 									<10> */
	if (highBand < bitRecPtr->yMax || lowBand > bitRecPtr->yMin ) 
		scanControl = 0;

	/* Allow client to turn off DOControl                               <10> */
	if (TheScratch(key)->dropoutScratch.ptr == nil ) 
		scanControl = 0;
	
	bitRecPtr->bitMap = (tt_uint32*)TheScratch(key)->actualBitmap.ptr;

	if ( scanControl )					
	{
		char* memPtr		= (char*)TheScratch(key)->dropoutScratch.ptr;
		bitRecPtr->xLines 	= (tt_int16*)memPtr;
		
		memPtr			+= (bitRecPtr->nXchanges+2) * nx * sizeof(tt_int16);
		bitRecPtr->xBase 	= (tt_int16**)ALIGN(memPtr, sizeof(tt_int16*));
		
		ny				= bitRecPtr->yMax - bitRecPtr->yMin;
		if (ny == 0 ) ++ny;
		
		memPtr			= (char*)TheScratch(key)->scanConvertScratch.ptr;
		bitRecPtr->yLines 	= (tt_int16*)memPtr;
		
		memPtr 			+= (bitRecPtr->nYchanges+2) * ny * sizeof(tt_int16);
		bitRecPtr->yBase 	= (tt_int16**)ALIGN(memPtr, sizeof(tt_int16*));
	}
	else
	{
		char*	memPtr;
		
		ny				= highBand - lowBand;
		if (ny == 0 ) ++ny;
		
		memPtr			= (char*)TheScratch(key)->scanConvertScratch.ptr;
		bitRecPtr->yLines 	= (tt_int16*)memPtr;
		
		memPtr 			+= (bitRecPtr->nYchanges+2) * ny * sizeof(tt_int16);
		bitRecPtr->yBase 	= (tt_int16**)ALIGN(memPtr, sizeof(tt_int16*));
	}
	
	error = sc_ScanChar2(charData, scPtr, bitRecPtr, lowBand, highBand, scanControl );
 	if (error)
 		return error;
	{
	  tt_uint16  swapTest=1,test;
	  glyphImage->image		= (char *)bitRecPtr->bitMap;
	  glyphImage->rowBytes		= bitRecPtr->wide >> 3;
	  glyphImage->bounds.xMin      	= bitRecPtr->xMin;
	  glyphImage->bounds.xMax      	= bitRecPtr->xMin + nx;
	  glyphImage->bounds.yMin      	= lowBand;
	  glyphImage->bounds.yMax      	= lowBand + ny;
	  test= ( * ((unsigned char *) &swapTest));
		
	  if (test==1) {
	    /* Do a byteswp */
	    /* we must be running on a swapped machine. */
	    tt_uint32 *bytes32= bitRecPtr->bitMap;
	    tt_uint32 *org=bitRecPtr->bitMap;
	    tt_uint32 numWords= (glyphImage->rowBytes>>2) *  (ny);
	    tt_uint32 nextWord,newWord,i;
	    for (i=numWords;i!=0; i--) 	{
	      nextWord=*bytes32;
	      newWord=(nextWord<<24) +
		( (nextWord<<8)&0x00FF0000) +
		( (nextWord>>8)&0x0000FF00) +
		( (nextWord>>24)&0x0000FF);
	      *bytes32++= newWord;	  
	    }
		
	  }	
	}	
	return scalerError_NoError;
}
/* assume that full bitmap has been requested and this is not a whitespace glyph.
 * that is,
 * glyphImage is not nil
 * noImageGlyph is not set in glyphRecord->flags
 * number of contours >= 1
 */

#define ROUND_SIZE(x, asize) (((x) + ((asize) - 1L)) & ~((asize) - 1L))

 void fs_FindBitMapSize4(fsg_SplineKey *key, const fnt_ElementType* charData, 
 						scalerBitmap *glyphImage)
{
	 sc_BitMapData		*bitRecPtr;
	tt_uint16					scan, byteWidth;
	tt_int32 					 nx;
	tt_int32					doc = fs_dropOutVal( key );

	bitRecPtr	= &TheGlyph(key)->bitMapInfo;
	sc_FindExtrema4(charData, bitRecPtr,doc,key);  /* use precalculated bounding box, eliminate duplicate points */
				
	/* Compute the height of the gxBitmap */	
 	scan	= bitRecPtr->high;
 	byteWidth	= bitRecPtr->wide >> 3;	
	if ( scan == 0 )
		 ++scan;		/* Always at least one scan gxLine */
  	TheScratch(key)->actualBitmap.size =  
  		SHORTMUL(scan, byteWidth);
 
	/* get memory for yLines & yBase in bitMapRecord */
	TheScratch(key)->scanConvertScratch.size =  
	    (tt_int32)scan * 
	    ROUND_SIZE(((bitRecPtr->nYchanges + 2) * sizeof(tt_int16) + sizeof(tt_int16*)), 
		       sizeof(tt_int16*));

	if( doc )
	{
		/* get memory for xLines and xBase - used only for dropout control */			
		nx 		= bitRecPtr->xMax - bitRecPtr->xMin;
		TheScratch(key)->dropoutScratch.size 	
			= nx * ROUND_SIZE(((bitRecPtr->nXchanges+2) * sizeof(tt_int16) + sizeof(tt_int16*)), 
					  sizeof(tt_int16*));
	}
	else TheScratch(key)->dropoutScratch.size = 0;

 	glyphImage->actualBitmapSize = TheScratch(key)->actualBitmap.size;
	glyphImage->scanConvertScratchSize = TheScratch(key)->scanConvertScratch.size;
	glyphImage->dropoutScratchSize = TheScratch(key)->dropoutScratch.size;
 }

 tt_int32 fs_CalculateBounds(
	fsg_SplineKey *key,
	const fnt_ElementType* glyphPtr, 
	scalerBitmap *glyphImage)
{
	 F26Dot6 *v, tv, vmin, vmax, ctr;
	F26Dot6 xmin, xmax, ymin, ymax;
	tt_int32 n;
	fastInt aPoint, startPoint, endPoint,wide;	
	fastInt firstTime = true;
	sc_BitMapData *bbox = &TheGlyph(key)->bitMapInfo;
	tt_int32 numPts = glyphPtr->pointCount;
	
	vmin = vmax = 0;	/* in case all contours are 1 point only */	

	for ( ctr = 0; ctr < glyphPtr->contourCount; ctr++ ) 
	{
		endPoint = glyphPtr->ep[ctr];
		startPoint = glyphPtr->sp[ctr];
		if ( startPoint == endPoint ) continue; /* We need to do this for anchor points for composites */
		v = &(glyphPtr->x[startPoint]); 						
		if( firstTime )
		{
			vmin = vmax = *v;									
			firstTime = false;
		}
		for( aPoint = startPoint; aPoint <= endPoint; ++aPoint )
		{
			tv = *v++;											
			if (tv > vmax ) vmax = tv;
			else if (tv < vmin ) vmin = tv;
		}
	}
	xmin = (vmin + HALFM) >> PIXSHIFT;
	xmax = (vmax + HALF) >> PIXSHIFT;

	vmin = vmax = 0;	/* in case all contours are 1 point only */		
	firstTime = true;
	for ( ctr = 0; ctr < glyphPtr->contourCount; ctr++ ) 
	{
		endPoint = glyphPtr->ep[ctr];
		startPoint = glyphPtr->sp[ctr];
		if ( startPoint == endPoint ) 
			continue; /* We need to do this for anchor points for composites */	
		v = &(glyphPtr->y[startPoint]); 						
		if( firstTime )
		{
			vmin = vmax = *v;									
			firstTime = false;
		}
		for( aPoint = startPoint; aPoint <= endPoint; ++aPoint )
		{
			tv = *v++;											
			if (tv > vmax ) 
				vmax = tv;
			else 
				if (tv < vmin ) 
					vmin = tv;
		}
	}
	ymin = (vmin + HALFM) >> PIXSHIFT;
	ymax = (vmax + HALF) >> PIXSHIFT;
	
	bbox->xMax = (tt_int16)xmax; /* quickdraw gxBitmap boundaries  */
	bbox->xMin = (tt_int16)xmin;  
	bbox->yMax = (tt_int16)ymax;
	bbox->yMin = (tt_int16)ymin;

	bbox->high = (tt_uint16)(ymax - ymin);
	n = xmax - xmin;				/*  width is rounded up to be a long multiple*/
	bbox->wide = ( n + 31) & ~31;		/* also add 1 when already an exact long multiple*/
	if( !( n & 31) )
		 bbox->wide += 32;
	
	if( xmin <= -32768 || ymin <= -32768 || xmax > 32767 || ymax > 32767 )  
		return scalerError_POINT_MIGRATION_ERR;
		
	glyphImage->image			= (char*)nil;
	glyphImage->rowBytes		= bbox->wide >> 3;
	glyphImage->bounds.xMin		= bbox->xMin;
	glyphImage->bounds.yMin		= bbox->yMin;
	glyphImage->bounds.xMax		= bbox->xMax;
	glyphImage->bounds.yMax		= bbox->yMax;
	if (glyphImage->bounds.xMax == glyphImage->bounds.xMin)	
		++glyphImage->bounds.xMax;  /* thin glyph */
	if (glyphImage->bounds.yMax == glyphImage->bounds.yMin)	
		++glyphImage->bounds.yMax;  /* thin glyph */
	if (glyphImage->rowBytes == 0)	
		glyphImage->rowBytes = 4;  	
#if 0						/* thin glyph */
	if (TheTrans(key)->verticalOrientation)
	{
		glyphImage->topLeft.x = IntToFixed(bbox->xMin) - F26Dot6ToFixed(glyphPtr->x[numPts + topSidePhantom]);
		glyphImage->topLeft.y = IntToFixed(bbox->yMax) - F26Dot6ToFixed(glyphPtr->y[numPts + topSidePhantom]);
	}
	else
#endif
	{
		glyphImage->topLeft.x = IntToFixed(bbox->xMin) - F26Dot6ToFixed(glyphPtr->x[numPts + leftSidePhantom]);
		glyphImage->topLeft.y = IntToFixed(bbox->yMax) - F26Dot6ToFixed(glyphPtr->y[numPts + leftSidePhantom]);
	}			
	return false;
}



 
#endif
 

