/*
 * @(#)Orient.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef __T2K_Orientation__
#define __T2K_Orientation__

#include "OrientDB.h"
/* code is triggered by "ORIENTBOLD_STYLES" in "config.h" */
  

/* specify a double precision quadratic bezier. */
typedef struct
{
	double x0; double y0;
	double x1; double y1;
	double x2; double y2;
} QuadraticBezier;

/* specify accumulated scanning resutls for some specific point. 			*/
/* This result includes enough information for even-odd or non-zero winding	*/
typedef struct
{
	double x;			/* x position of check point. */
	double y;			/* y position of check point. */
	tt_int32  thisContourIndex;
	tt_int32  thisPointIndex;

	tt_int32 direction;	/* direction of curve at point position. */
						/*  1=up, -1 = down, 0 not allowed 		  */

	tt_int32 leftSideUpards; 		/* Init to zero. */
	tt_int32 leftSideDownwards;	/* Init to zero. */
	tt_int32 rightSideUpwards;		/* Init to zero. */
	tt_int32 rightSideDownwards;	/* Init to zero. */
	tt_int32 sameXUp;			/* Init to zero. */
	tt_int32 sameXDown;			/* Init to zero. */
} ScanPoint;


#define InconsistentWinding 	0
#define LeftSideBlack	 		1
#define RightSideBlack	 		2
#define BothSideBlack	 		3
#define BothSideWhite	 		4
#define SameXAmbiguity	 		5


 	 
/* The following code is used to determine the type of quadratic bezier curve. 	*/
/* Most importantly, if the there is a y-axis minimum or maximum between 0<t<1	*/
/*		then it returns that point. 											*/

#define		YDerivZeroAlwaysNoT 			1
			/* YDerivZeroAlwaysNoT: there is no change in yAxis at all.	*/
			
#define		YDerivZeroY0EqualY2			2
			/* The y-Axis changes for the control point.						*/
			/* 		but y-axis end points are equal.							*/
			
#define		YDerivMonotonicAnyT				3
			/* The y axis changes monotonically, AND no y-axis control points	*/
			/* are equal.														*/
			/* That is, the middle control point is between the other two.		*/
			
#define 	YDerivZeroControlOutside	4
  			/* the y-axis values all differ, but the middle control point, y1,	*/
  			/*   does not lie between y0 and y2.								*/
  			
#define		YDerivZeroAtT0					5
			/* the y0 and y1 values are equal, and not equal to y2				*/

#define		YDerivZeroAtT1					6
			/* the y2 and y1 values are equal, and not equal to y0				*/
			

typedef struct
{
	tt_int32 	xPrev;
	tt_int32 	yPrev;
	tt_int32	onCurvePrev;

	tt_int32	xThis;
	tt_int32	yThis;
	tt_int32	onCurveThis;

	tt_int32	xNext;
	tt_int32	yNext;
	tt_int32	onCurveNext;
} CurveInput;


/* Result Codes for FindNonZeroWindingCounts	*/	
#define	CouldNotBeCheckedWindingResult 	0
/* Otherwise return the new winding value. */




/* See CheckLeftOrRight. 														*/
/* For some x,y location, which lies on a common scan line with a 				*/
/*   y-axis monotonic bezier curve,	determine whether the point lies to the		*/
/*	the left or right of the curve segment. This routine does not care			*/
/*	whether the bezier is ascending or descending.								*/
/* An ambiguous result means that the curve passed, essentially, right 			*/
/*	through the point. An ambiguous case is usually triggered by a vertical		*/
/*	curve segment.																*/

#define 	XisLEFTofCurve	-1
#define 	XisAMBIGUOUS  	0
#define 	XisRIGHTofCurve  1


/* The following definitions for curve type should match those in T2KSC.c */
#define CurveTypeQuadraticBezier 2
#define CurveTypeCubicBezier 3

/* The ScanInputBlock contains the same information as input to tsi_NewScanConv,
	but the field lengths are optimized to modern computers. Except, greyScaleLevel 
	is dropped.
 */

typedef struct
{
	tt_int32 numberOfContours;  /* # contours in glyph. */
	short *startPtr; 	    /* start of shorts which define indices in contour. */
		short *endPtr; 		/* end of short indices. */
	tt_int32 arrayType;		/* 0 for long, 1 for short. */
	void *xArray; 			/* x-coordinate in 26.6 or  short */
	void *yArray; 			/* y-coordinate in 26.6 or  short */
 	unsigned char *onCurvePtr; 	/* point to flags indicating onCurve points. */
 	tt_int32 curveType;			/* Specify bezier type. if all points are off curve, */
 							/* then linear */
} ScanInputBlock;
 

/* Specify the array type. ;*/
#define Orient32BitArray 0
#define Orient16BitArray 1

void SetOrientBlock( ScanInputBlock *sib,short numberOfContours, short *startPtr, 
	short *endPtr,  tt_int32 arrayType, void *xPtr, void *yPtr, 
	unsigned char *onCurvePtr,  char curveType );

tt_int32 FindNonZeroWindingCounts(ScanInputBlock *sib,tt_int32 contourIndex,tt_int32 compTest);

#endif










