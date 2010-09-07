/*
 * @(#)shapet.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * SHAPET.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */

#ifndef __T2K_SHAPET__
#define __T2K_SHAPET__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
// #include "config.h"

#include "tt_prvt.h"
// #include "dtypes.h"
#include "Orient.h"
 
#ifdef ALGORITHMIC_STYLES
/*
 * bold multiplier == 1.0 means do nothing.
 
 see StyleFuncPtr, PreStyleFuncPtr, StyleMetricsFuncPtr
 */
void tsi_SHAPET_BOLD_GLYPH( GlyphClass *glyph, tsiMemObject *mem, short UPEM, 
 	 ContourData *cd,			/* orientation information */
 F16Dot16 params[] );
 
tt_int32 tsi_SHAPET_BOLD_METRICS( hmtxClass *hmtx, tsiMemObject *mem, short UPEM, F16Dot16 params[] );

/* StylePostFunc routine format.														*/
/* Called: StylePostFunc routine is called after hinting and stretching, but prior to	*/
/*			rotation, skew and, and flipping. The routine is called to act on the 		*/
/*			entire glyph, and NOT on each subcomponent.	 Therefore, bolding and italic,	*/
/*			or whatever style you provide, will be rotated and flipped after the style	*/
/*			is applied.																	*/
/* This version of StylePostFunc incorporates three new parameters. "curveType" and		*/
/*			and "*onCurve" are usually not needed. The "ContourData" parameter			*/
/*			provides information on the topology including which side of a line			*/
/*			segment has black bits.	This information is critical to proper bolding		*/
/*			but is normally not needed when performing italic processing.				 
 
 		short	contourCount:			number of contours in the character.
 		
		short 	pointCount,				total number of points in the contours, not	
										including sidebearing points.
										
		short	*sp,					sp[contourCount] Start point array for each contour. 
		short	*ep,  					ep[contourCount] End point array for each contour.
 		F26Dot6 *xx,					the x array, with phantom, x-position for adjustment.
 		F26Dot6 *yy,					the y array, with phantom, x-position for adjustment.
 		F26Dot6 *x,						temp usage x-position array, without sidebearings.
 		F26Dot6 *y,						temp usage y-position array, without sidebearings.
 		
	 	tsiMemObject *mem, 				use for additional memory allocation.
	 	F16Dot16 xPixelsPerEmFixed,
	 	F16Dot16 yPixelsPerEm16Dot16, 
	 	
 		short curveType,			  2 (truetype quadratic) or 3(postscript cubic)  
	 	uint8 *onCurve,				  onCurve[pointCount] indicates if a point 
	 									is on or off the curve, it should be 
	 									true or false. During hinting, these values
	 									may have changed.
	 									
	 	ContourData *cd,			 orientation information- see OrientDB.h .
	 	
 	 	F16Dot16 params[] 			 passed parameters for styling.
 */

/* see StyleFuncPost */
void tsi_SHAPET_BOLD_GLYPH_Hinted( 
	short	contourCount,	 short 	pointCount,  short	*sp,  short	*ep, 
 	F26Dot6 *xx,  F26Dot6 *yy,  F26Dot6 *x,  F26Dot6 *y, 
	tsiMemObject *mem, F16Dot16 xPixelsPerEmFixed,F16Dot16 yPixelsPerEm16Dot16, 
 	 short curveType, tt_uint8 *onCurve,		
  	 ContourData *cd,
 	 F16Dot16 params[] );

/* see StyleFuncPost */
void tsi_SHAPET_Italic_GLYPH_Hinted( 
		short	contourCount,	 short 	pointCount,  short	*sp,  short	*ep, 
 	F26Dot6 *xx,  F26Dot6 *yy,  F26Dot6 *x,  F26Dot6 *y, 
	tsiMemObject *mem, F16Dot16 xPixelsPerEmFixed,F16Dot16 yPixelsPerEm16Dot16, 
 	 short curveType, tt_uint8 *onCurve,		
  	 ContourData *cd,
 	 F16Dot16 params[]);

/* see StyleFuncPost */
void tsi_SHAPET_BoldItalic_GLYPH_Hinted( 	short	contourCount,	 short 	pointCount,  short	*sp,  short	*ep, 
 	F26Dot6 *xx,  F26Dot6 *yy,  F26Dot6 *x,  F26Dot6 *y, 
	tsiMemObject *mem, F16Dot16 xPixelsPerEmFixed,F16Dot16 yPixelsPerEm16Dot16, 
 	 short curveType, tt_uint8 *onCurve,		
  	 ContourData *cd,
 	 F16Dot16 params[]);
 	 
/* if StyleFunc is set then this routine returns changes the hmtx table and returns the amount of adjusment. */
#endif /* ALGORITHMIC_STYLES */

typedef struct {
	/* private */
	tsiMemObject *mem;
	/* public */
} SHAPETClass;

/* returns: +1 if counter clockwise, -1 if clockwise, or 0 if indeterminate */
 int FindContourOrientation( F26Dot6 *x, F26Dot6 *y, tt_int32 count);		
/* returns: +1 if counter clockwise, -1 if clockwise, or 0 if indeterminate */
 int FindContourOrientationShort ( tt_int16 *x, tt_int16 *y, tt_int32 count);

 void	AccumulateGlyphContours(ContourData *cd, GlyphClass *glyph );

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_SHAPET__ */
