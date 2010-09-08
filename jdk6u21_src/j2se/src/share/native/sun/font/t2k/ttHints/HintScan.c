/*
 * @(#)HintScan.c    1.10 03/12/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

#include "cpu.h" //for fastint 

/* copy macros as temporal solution - straightforward including cause compilation issues */
#define F26Dot6ToFixed(x)        ((x) << 10)
#define IntToFixed(a)       ((Fixed)(a) << 16)

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
 
tt_int32 fs_ContourScan3(tsiScanConv *t, sc_BitMapData *bitRecPtr, scalerBitmap* glyphImage, tt_int32 dropoutMode)
{
	tt_int16					lowBand, highBand;
	tt_int16 					nx, ny;
	tt_int32					error;

	nx 				= bitRecPtr->xMax - bitRecPtr->xMin;
	if (nx == 0 ) 
		++nx;
    
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
	/* check for out of bounds band request                              <10> */
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
	if (highBand < bitRecPtr->yMax || lowBand > bitRecPtr->yMin) {
		dropoutMode = DOCONTROL_DISABLED;
	}

	if (dropoutMode != DOCONTROL_DISABLED) {
		char*  memPtr = (char*) bitRecPtr->xLines;
		memPtr			+= (bitRecPtr->nXchanges+2) * nx * sizeof(tt_int16);
		bitRecPtr->xBase 	= (tt_int16**) ALIGN(memPtr, sizeof(tt_int16*));

		ny				= bitRecPtr->yMax - bitRecPtr->yMin;
		if (ny == 0 ) ++ny;
        
		memPtr = (char *) bitRecPtr->yLines;
		memPtr 			+= (bitRecPtr->nYchanges+2) * ny * sizeof(tt_int16);
		bitRecPtr->yBase     = (tt_int16**)ALIGN(memPtr, sizeof(tt_int16*));
	} else {
		char*	memPtr;
        
		ny				= highBand - lowBand;
		if (ny == 0 ) ++ny;
        
		memPtr = (char *) bitRecPtr->yLines;
		memPtr 			+= (bitRecPtr->nYchanges+2) * ny * sizeof(tt_int16);
		bitRecPtr->yBase 	= (tt_int16**)ALIGN(memPtr, sizeof(tt_int16*));
	}
	
	error = sc_ScanChar2(t, bitRecPtr, lowBand, highBand, dropoutMode);
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

void fs_FindBitMapSize4(sc_BitMapData *bitRecPtr, tsiScanConv *t, 
						scalerBitmap *glyphImage, tt_int32 dropoutMode)
{
	tt_uint16					scan, byteWidth;
	tt_int32 					 nx;

	sc_FindExtrema4(t, bitRecPtr, dropoutMode);  /* use precalculated bounding box, eliminate duplicate points */
				
	/* Compute the height of the gxBitmap */	
 	scan	= bitRecPtr->high;
 	byteWidth	= bitRecPtr->wide >> 3;	
	if ( scan == 0 )
		 ++scan;		/* Always at least one scan gxLine */

	glyphImage->actualBitmapSize = ((tt_uint32) scan) * ((tt_uint32) byteWidth);
 
	/* get memory for yLines & yBase in bitMapRecord */
	glyphImage->scanConvertScratchSize =
	    (tt_int32)scan * 
	    ROUND_SIZE(((bitRecPtr->nYchanges + 2) * sizeof(tt_int16) + sizeof(tt_int16*)), 
		       sizeof(tt_int16*));

	if (dropoutMode != DOCONTROL_DISABLED) {
		/* get memory for xLines and xBase - used only for dropout control */			
		nx 		= bitRecPtr->xMax - bitRecPtr->xMin;

		/* for consistency ensure nx != 0 */
		if (nx == 0) {
		  nx++;
		}

		glyphImage->dropoutScratchSize 
			= nx * ROUND_SIZE(((bitRecPtr->nXchanges+2) * sizeof(tt_int16) + sizeof(tt_int16*)), 
					  sizeof(tt_int16*));
	} else {
		glyphImage->dropoutScratchSize = 0;
	}
}

tt_int32 fs_CalculateBounds(sc_BitMapData *bbox, register tsiScanConv *t, scalerBitmap *glyphImage)
{
	 F26Dot6 *v, tv, vmin, vmax, ctr;
	F26Dot6 xmin, xmax, ymin, ymax;
	tt_int32 n;
	fastInt aPoint, startPoint, endPoint;
	fastInt firstTime = true;
	tt_int32 numPts = 0;
	
	if (t->numberOfContours > 0) {
	    numPts = t->endPoint[t->numberOfContours-1];
	}
	
	vmin = vmax = 0;	/* in case all contours are 1 point only */	

	for(ctr = 0; ctr < t->numberOfContours; ctr++) 
	{
		endPoint = t->endPoint[ctr];
		startPoint = t->startPoint[ctr];
		if ( startPoint == endPoint ) continue; /* We need to do this for anchor points for composites */
		v = &(t->x[startPoint]);
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
	for(ctr = 0; ctr < t->numberOfContours; ctr++) {
		endPoint = t->endPoint[ctr];
		startPoint = t->startPoint[ctr];
		if ( startPoint == endPoint ) 
			continue; /* We need to do this for anchor points for composites */	
		v = &(t->y[startPoint]);
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
		
	glyphImage->image			= NULL;
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
		/* numPts is array index of the last point of the last glyph contour. 
		   So we need to add 1 to get index of leftSidePhantom. */
		glyphImage->topLeft.x = IntToFixed(bbox->xMin) - F26Dot6ToFixed(t->x[numPts+1]); 
		glyphImage->topLeft.y = IntToFixed(bbox->yMax) - F26Dot6ToFixed(t->y[numPts+1]); 
	}				
	return false;
}
