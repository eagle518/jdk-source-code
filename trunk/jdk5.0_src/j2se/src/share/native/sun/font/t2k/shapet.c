/*
 * @(#)shapet.c	1.20 04/05/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * SHAPET.c
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
#include "syshead.h"

#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "glyph.h"
#include "autogrid.h"
#include "shapet.h"
#include "util.h"
#include "FixMulDiv.h"


#ifdef ALGORITHMIC_STYLES
#define FIXROUND( x )		(tt_int16)(((x) + 0x8000L) >> 16)	
/*
 *
 *
 * (1) x2 + dx2 * t2 = x1 + dx1 * t1
 * (2) y2 + dy2 * t2 = y1 + dy1 * t1
 *
 *  1  =>  t1 = ( x2 - x1 + dx2 * t2 ) / dx1
 *  +2 =>  y2 + dy2 * t2 = y1 + dy1/dx1 * [ x2 - x1 + dx2 * t2 ]
 *
 *     => t2 * [dy1/dx1 * dx2 - dy2] = y2 - y1 - dy1/dx1*(x2-x1)
 *     => t2(dy1*dx2 - dy2*dx1) = dx1(y2 - y1) + dy1(x1-x2)
 *     => t2 = [dx1(y2-y1) + dy1(x1-x2)] / [dy1*dx2 - dy2*dx1]
 *     => t2 = [dx1(y2-y1) - dy1(x2-x1)] / [dx2*dy1 - dy2*dx1]
 *     t2 = Num/Denom
 *     =>
 *	    Num   = (y2 - y1) * dx1 - (x2 - x1) * dy1;
 *		Denom = dx2 * dy1 - dy2 * dx1;
 *
 */
static void ComputeIntersection( short line1_pt1_x, short line1_pt1_y, short line1_pt2_x, short line1_pt2_y,
								 short line2_pt1_x, short line2_pt1_y, short line2_pt2_x, short line2_pt2_y,
								 short *x, short *y )
{
 	tt_int32 dx1, dy1, dx2, dy2, x1, y1, x2, y2;
	tt_int32 Num,Denom, t;
	
	
	dx1 = line1_pt2_x - line1_pt1_x;	dy1 = line1_pt2_y - line1_pt1_y;
	dx2 = line2_pt2_x - line2_pt1_x;	dy2 = line2_pt2_y - line2_pt1_y;
 
 	x1 = line1_pt1_x; y1 = line1_pt1_y;
 	x2 = line2_pt1_x; y2 = line2_pt1_y;

	Num   = (y2 - y1) * dx1 - (x2 - x1) * dy1;
	Denom = dx2 * dy1 - dy2 * dx1;

	if ( Denom != 0 ) { /* 3/19/98 changed 0.0 to 0 ---Sampo */
		t = util_FixDiv( Num, Denom );
		*x = (short)( x2 + util_FixMul( dx2, t ) );
		*y = (short)( y2 + util_FixMul( dy2, t ) );
	} else {
		*x = (short)((line1_pt2_x+line2_pt1_x)/2);
		*y = (short)((line1_pt2_y+line2_pt1_y)/2);
	}
}

		#define FIXROUND( x )		(tt_int16)(((x) + 0x8000L) >> 16)	
		/* The following routine takes a bold multiplier,m, where 1.0<m<~3. 		*/
		/*  values of 2 or greater tend to be excessively bold, and therefore ugly. */
		/* Empirically, values of about 1.3 to 1.5 appear best. 					*/
		/* The routine returns an amount by which the UPEM (x-axis) is changed.		*/
		/* Because "abstract" bolding applies to both sides of the character,		*/
		/* the value returned is always even, so that it may be split into 2.		*/
		static F16Dot16 GetDeltaMetric(F16Dot16 multiplier, F16Dot16 UPEMFixed)
		{
			F16Dot16 temp;
			/* semantics-> ((mult * UPEM)-UPEM)/6 */
			temp=FIXROUND(  (util_FixMul( multiplier, UPEMFixed ) - UPEMFixed)/6);
			/* Why do we go to an even value? I don't know */
			temp= ( temp + 1 )& (~1);
			/* And its always positive. */
			if (temp<0)
				temp=0;
			return(temp);
		}
		
		static double oneSixthFixed= ((double)1.0)/(  ((double)6.0)* ((double) 65536.0 ));
		static double oneSixth = ((double)1.0)/(  ((double)6.0) );
 		static tt_int32 GetDoubleDeltaMetric(F16Dot16 multiplier, double units)
		{
			/* semantics-> ((mult * UPEM)-UPEM)/6 */
			double doubleResult= ((  ((double) multiplier) * units)-units*65536.) * oneSixthFixed;
			return( (tt_int32) (doubleResult+.5));
 		}
		
		static void ApplyDeltaToHMTX( hmtxClass *hmtx,short delta)
		{
			tt_int32 i, limit;
			limit = hmtx->numGlyphs;
			for ( i = 0; i < limit; i++ ) {
				hmtx->aw[i] = (tt_uint16)(hmtx->aw[i] + delta);
			}
		}
  		
	/* 		Find an appropriate width adjustment, and apply it to the hmtx table.	*/
  	/* 	For usage, see StyleMetricsFunc */	
 	tt_int32 tsi_SHAPET_BOLD_METRICS( register hmtxClass *hmtx, tsiMemObject *mem, short UPEM, F16Dot16 params[] )
	{
		tt_int16 delta;
		F16Dot16 multiplier = params[0];
#if 0	 
		F16Dot16 UPEMFixed=((F16Dot16) UPEM)<<16;
 		UNUSED(mem);	
		delta= GetDeltaMetric( multiplier, UPEMFixed);
#else
		delta= GetDoubleDeltaMetric(multiplier, ((double) UPEM) );
#endif
 		ApplyDeltaToHMTX(hmtx, delta);
 		return(delta);
   	}
 
   
#endif /* ALGORITHMIC_STYLES */


 /*
 * multiplier == 1.0 means do nothing
 */
/* Similar to void tsi_SHAPET_BOLD_GLYPH, except that it operates on the hinted values,
	and UPEM is replaced by pixelsPerEmFixed the in F26Dot6 point format (e.g. 1/64 of a pixel) of the number pixels per em.
	*/
	
 		/* Adjust all x values by the change in the advance. */
		static void AdjustBoldAlignmentValues(
			tt_int32 countLessPhantoms, F26Dot6 *x, F26Dot6 *y, F26Dot6 boldLeftOffsetDot6,  F26Dot6 boldIntegerAdvanceDot6)
		{
			tt_int32 n=countLessPhantoms,i;
 			for (i=0;i<n;i++)
  				*x++= *x + boldLeftOffsetDot6;
 			x++; /* skip over phantom for left side bearing- should remain 0,0 */
 			* x = *x+boldIntegerAdvanceDot6; /* add integer advance. */
		} 

		/* if (b1-a1) * (b2-a2)<0, then return true (sign flip); implement without multiply */
		static int isVectorFlip(F26Dot6 a1, F26Dot6  b1, F26Dot6 a2, F26Dot6  b2)
		{
			F26Dot6 temp1,temp2;
			temp1=(b1-a1);
			temp2=(b2-a2);
			if ( (temp1==0) || (temp2==0) )
				return(false);					/* sign does not flip for zero. */
			if (temp1<0)
				 return(temp2>0);				/* return true if different signs. */
			else
				 return(temp2<0);				/* return true if different signs. */
		}
		
		typedef double F26DotDouble;
		double maxVertexMove= 65000. * 256.0;
		/* return true if nearly parallel */
		static int ComputeIntersectionDouble( 
								F26DotDouble line1_pt1_x, F26DotDouble line1_pt1_y, F26DotDouble line1_pt2_x, F26DotDouble line1_pt2_y,
								 F26DotDouble line2_pt1_x, F26DotDouble line2_pt1_y, F26DotDouble line2_pt2_x, F26DotDouble line2_pt2_y,
								 F26Dot6 *x, 	/* past original value, return new */
								 F26Dot6 *y, 	/* past original value, return new */
								 F26DotDouble maxMove 
								 )
			{
			 	F26DotDouble dx1, dy1, dx2, dy2, x1, y1, x2, y2;
				F26DotDouble numerator,denominator, t,absDenom;
				
				
				dx1 = line1_pt2_x - line1_pt1_x;	dy1 = line1_pt2_y - line1_pt1_y;
				dx2 = line2_pt2_x - line2_pt1_x;	dy2 = line2_pt2_y - line2_pt1_y;
			 
			 	x1 = line1_pt1_x; y1 = line1_pt1_y;
			 	x2 = line2_pt1_x; y2 = line2_pt1_y;

				numerator   = (y2 - y1) * dx1 - (x2 - x1) * dy1;
				denominator = dx2 * dy1 - dy2 * dx1;
				absDenom= denominator>0.0 ? denominator : -denominator;
				if ( absDenom > 1.0  ) 
				{ 
					double xDiff, xDiffAbs, yDiff, yDiffAbs;
					t =  numerator / denominator;
	 				xDiff= dx2 * t;
					yDiff= dy2 * t;
					if ( 
							(xDiff<maxVertexMove) && ((- xDiff)<maxVertexMove) &&
							(yDiff<maxVertexMove) && ((- yDiff)<maxVertexMove)){
					
							*x = (F26Dot6)( x2 +  xDiff ) ;
							*y = (F26Dot6)( y2 +  yDiff ) ;
							return(false);
							} 		
				}	
				/* Otherwise, perform a simple average */	
				*x = (F26Dot6)((line1_pt2_x+line2_pt1_x)*.5);
				*y = (F26Dot6)((line1_pt2_y+line2_pt1_y)*.5);
				return(true);
 			}
 			
#ifdef UIDebug
 extern 		int isBoldReverseContourFlag ;
 #endif 			
 		
/* used for teting. */
#ifdef ORIENTBOLD_STYLES	
#ifdef UIDebug
	/* this is only turned on in the debugging environment. */
#define xCompareSetsOfContourData 
#endif
#endif

/* 	n = glyph->ep[glyph->contourCount-1] + 1; */
/* The BOLDINTERIORSHIFT controls the amount of additional bolding on the interior areas of a glyph. */
/*  This type of bolding is not even necessary, value of 2 (1/4 the bolding) has marginal affect. */
/* The key concept is that bolding is integer-symmetric. That is, all bolding effects causes movement of */
/* outline points by about an integer amount (split leftside and rightside).  */
 #define BOLDINTERIORSHIFT 1
/* Note:  BOLDINTERIORSHIFT lessens the amount of boldness added to the interior edges (except for very 	*/
/*			very small sizes). When set to "1", the interior is shifted by 1, to yeild 1/2 the boldness.	*/
/*			However, the exterior is still full strength, so that the effect around interiors is 3/4.		*/
/*			For stem items (no interior areas) the boldness remains at full strength.						*/
 
static double PixelFixedDot6 = (64./65536.);
void tsi_SHAPET_BOLD_GLYPH_Hinted( 
	short	contourCount,	/* number of contours in the character */
	short 	pointCount,		/* number of points in the characters + 0 for the sidebearing points */
	short	*sp,			/* sp[contourCount] Start points */
	short	*ep,  			/* ep[contourCount] End points */
 	F26Dot6 *xx,			/* the x array, with phantom */
 	F26Dot6 *yy,			/* the y array, with phantom */
 	F26Dot6 *x,				/* the x array, doesn't need phantom */
 	F26Dot6 *y,				/* the y array, doesn't need  phantom */
 	
	tsiMemObject *mem, 
	F16Dot16 xPixelsPerEmFixed,  
	F16Dot16 yPixelsPerEmFixed, 
								/*  ORIENTBOLD_STYLES */
	 short curveType,			/* 2 (quadratic) or 3(cubic) */
	 tt_uint8 *onCurve,			/* onCurve[pointCount] indicates 
	 								if a point is on or off the curve, 
	 								it should be true or false */
	 ContourData  *cd,			/* Specify orientation of contours */

	F16Dot16 params[] )
{
	register tt_int32 ctr, A, B, C, n;
	F26Dot6 xA, xB, xC, yA, yB, yC, orig_xB, orig_yB;
	F26Dot6 line1_pt1_x, line1_pt1_y;
	F26Dot6 line1_pt2_x, line1_pt2_y;
	F26Dot6 line2_pt1_x, line2_pt1_y;
	F26Dot6 line2_pt2_x, line2_pt2_y;
	F26Dot6 rot1Dx, rot1Dy;
	F26Dot6 rot2Dx, rot2Dy;
#if 0
	F26Dot6 *x,  *y;
#endif
	int backupSize= sizeof(tt_int32);

	tt_int32 CounterClockWiseWinding; /* If clockwise, then less adjustment, because its probablyinterior */
	tt_int32 winding;	/* positive for counter-clockwise, negative for clockwise. */
	tt_int32 pointsWithinContour;
	F26Dot6 maxMove  ;
	F16Dot16 multiplier = params[0];
	F16Dot16 boldLinearAdvanceFix ;
	F26Dot6 boldLinearAdvanceDot6, boldIntegerAdvanceDot6;
	tt_int32 boldIntegerAdvance;
	tt_int32 leftSideIntegerMove,rightSideIntegerMove,leftSideIntegerMoveDot6,rightSideIntegerMoveDot6;
	tt_int32 leftSideIntegerMoveDot6Temp,rightSideIntegerMoveDot6Temp;
 	tt_int32 leftSideReverseInteger, rightSideReverseInteger,localOrient, globalOrient;
 
							/* ORIENTBOLD_STYLES */
	tt_int32 cdIndexOffset; /* offset to first contour for processing. */ 
  	/* Determine the maximum amount of pixel movement, fixed point */
	/* These valus should be calculated once when the transform is initialized. */
	boldLinearAdvanceDot6  = 
#if 0
			GetDeltaMetric(multiplier, xPixelsPerEmFixed<<6);
#else
			GetDoubleDeltaMetric(multiplier, ((double) xPixelsPerEmFixed) * PixelFixedDot6);
#endif
#if 0
 	if (boldLinearAdvanceDot6<32)	/* If less than 1/2 pixel */
 	      boldIntegerAdvance=0;		/* then 0, can't draw any bold */
 	else
 #endif
 		boldIntegerAdvance=	  ((boldLinearAdvanceDot6+32)>>6);
   	
	boldIntegerAdvanceDot6= boldIntegerAdvance<<6;
	maxMove=boldIntegerAdvanceDot6;
	
	/* Divide the integer into left and right side advance. */
	/* NOTE: right side advance may exceed left side by 1 pixel, but see */
	/* This is going to be reversed interior values. */
  	leftSideIntegerMove= boldIntegerAdvance>>1;
	leftSideIntegerMoveDot6= leftSideIntegerMove<<6;
	
	rightSideIntegerMove= boldIntegerAdvance-leftSideIntegerMove;
	rightSideIntegerMoveDot6=rightSideIntegerMove<<6;
#if 1		
			/* For interior curves, we shrink the amount of emboldening. */
 			leftSideReverseInteger= (leftSideIntegerMove>>BOLDINTERIORSHIFT)  ;
			rightSideReverseInteger= (rightSideIntegerMove>>BOLDINTERIORSHIFT)+(rightSideIntegerMove&1 ) ;
#else
			/* For interior curves, we only make up for the assymmetry */
 			leftSideReverseInteger= 0 ;
			rightSideReverseInteger= boldIntegerAdvance&1 ;
#endif									
	/* eg. moveX= leftSideIntegerMove or rightSideIntegerMove */
	n = pointCount;
  	if (boldIntegerAdvance != 0) {
  		/* if there is an advance. */
 		AdjustBoldAlignmentValues(n,  xx,  yy, leftSideIntegerMoveDot6,   boldIntegerAdvanceDot6);
		if (n == 0) {
		    return;
		}
		/* Save the original points. */	
	#if 0	
		x = (tt_int32 *)tsi_AllocMem( mem, (n+n) * backupSize );
		y = &x[n];
 	#endif
		for ( A = 0; A < n; A++ ) {
			x[A] = xx[A];
			y[A] = yy[A];
		}
		
		/* ORIENTBOLD_STYLES */
		/* the contour data stores all contours for all component glyphs. 	*/
		/* to find the needed contours, use the most recent entries.		*/
		cdIndexOffset=	cd->current-contourCount; /* first relevant index.  */
#ifdef ORIENTBOLD_STYLES
#ifdef CompareSetsOfContourData	
 			{
 				ScanInputBlock sib; 				
 			 	SetOrientBlock( &sib, contourCount, sp,ep,
			 			Orient32BitArray, xx, yy, onCurve, curveType);
	 			  	/* Setup all contour windings before modifying any of them. */
			 	for ( ctr = 0; ctr < contourCount; ctr++ ) 
			 	{
			 		tt_int32 	contourStart=sp[ctr], 
			 				contourEnd=ep[ctr],  
			 				contourPoints;
			 		tt_int32 expectedLocal, expectedGlobal,badCount=0;
			 		contourPoints=contourEnd-contourStart+1;
			 		localOrient=0; globalOrient=0;
			 		
			 		
						n =  ep[ctr];
						A = n;
						xB = xx[A]; yB = yy[A];
						B = sp[ctr];
						orig_xB = xx[B]; orig_yB = yy[B];
						xC = orig_xB; yC = orig_yB;
						pointsWithinContour= n-B+1; 
						
					/* Some contours may be oriented opposite of the standard.	*/
			 		if (curveType==2)	/* only quadratics for now. */
						winding= FindContourOrientation( &xx[B], &yy[B], pointsWithinContour);
					else
						winding=0;
						
			 		if (curveType==2)	/* only quadratics for now. */
						globalOrient=FindNonZeroWindingCounts( &sib, ctr,1);
					else
						globalOrient=0;
					localOrient= (winding>0);
			 		GetContourDataSetQuick( cd,ctr+cdIndexOffset, &expectedLocal, &expectedGlobal);
			 		if ( 
			 			( 
			 				(expectedLocal!=localOrient) 
			 						&&
			 				(winding!=0) /* degenerate winding- skip error*/
			 			)
			 						|| 
			 			(expectedGlobal!=globalOrient)
			 		)
			 			badCount++;
				}
			}
			
#endif
#endif		


		for ( ctr = 0; ctr < contourCount; ctr++ ) {
			n =  ep[ctr];
			A = n;
			xB = xx[A]; yB = yy[A];
			B = sp[ctr];
			orig_xB = xx[B]; orig_yB = yy[B];
			xC = orig_xB; yC = orig_yB;
			pointsWithinContour= n-B+1; 
#ifdef ORIENTBOLD_STYLES
			if (curveType==2)
			{					
				GetContourDataSet(cd,ctr+cdIndexOffset, &localOrient, &globalOrient);
				if (globalOrient)
					CounterClockWiseWinding= ! localOrient;
				else 
					CounterClockWiseWinding=localOrient;
			}
			else
			{
				winding= FindContourOrientation( &xx[B], &yy[B], pointsWithinContour);
	 			CounterClockWiseWinding= (winding>0);
	 			globalOrient=0;
			}
#else
			winding= FindContourOrientation( &xx[B], &yy[B], pointsWithinContour);
			globalOrient=0;
	 		CounterClockWiseWinding= (winding>0);
#endif

#ifdef UIDebug
			if (isBoldReverseContourFlag)
				globalOrient = ! globalOrient;
#endif

			if (CounterClockWiseWinding)
			{
				/* counter Clockwise implies interior */
				if (leftSideReverseInteger==0 && rightSideReverseInteger==0 )
					continue; /* nothing to do here! */
				leftSideIntegerMoveDot6Temp  = leftSideReverseInteger  << 6;
				rightSideIntegerMoveDot6Temp = rightSideReverseInteger << 6;
 			}
			else
			{
				/* different bolding strength for interior and exterior curves may cause artefacts
				   (see 4848450 for example). 
				   On other hand current multiplier is rather large (compared to MS word) and 
				   we get some overlapped glyph contours even on medium size glyphs. 
				   So, we use decreased bolding strength for exterior contours too.
				   Note: adjustment of alignment values was performed assuming full strength bolding 
				   because we want to preserve metrics */
				leftSideIntegerMoveDot6Temp  = leftSideReverseInteger   << 6;
				rightSideIntegerMoveDot6Temp = rightSideReverseInteger  << 6;
				if (leftSideIntegerMoveDot6Temp==0 && rightSideIntegerMoveDot6Temp==0 )
						continue; /* nothing to do here! */
			}
 			/* globalOrient  also means that the direction of bolding is reversed. */
			if (globalOrient)
			{
				leftSideIntegerMoveDot6Temp=-leftSideIntegerMoveDot6Temp;
				rightSideIntegerMoveDot6Temp=-rightSideIntegerMoveDot6Temp;
			}

			for ( ; B <= n; B++ ) {
				xA = xB; yA = yB;
				xB = xC; yB = yC;
			
				C = B + 1; 
				if ( C > n ) {
				 	/* C = glyph->sp[ctr]; */
					xC = orig_xB; yC = orig_yB; /* can't read x[B] since it has been reset */
				} else {
					xC = xx[C]; yC = yy[C];
				}
				
				line1_pt1_x = xA;	line1_pt1_y = yA;
				line1_pt2_x = xB;	line1_pt2_y = yB;

				line2_pt1_x = xB;	line2_pt1_y = yB;
				line2_pt2_x = xC;	line2_pt2_y = yC;
				
				/* rotate anti-clockwise 90 degrees */
				rot1Dx =  -(line1_pt2_y - line1_pt1_y);	/* -dy */
				rot1Dy =  (line1_pt2_x - line1_pt1_x);		/* +dx */
				
				rot2Dx =  -(line2_pt2_y - line2_pt1_y);	/* -dy */
				rot2Dy =  (line2_pt2_x - line2_pt1_x);		/* +dx */

 				if ( rot1Dx > 0 ) {
					line1_pt1_x =  (line1_pt1_x + rightSideIntegerMoveDot6Temp);
					line1_pt2_x =  (line1_pt2_x + rightSideIntegerMoveDot6Temp);
				} else if ( rot1Dx < 0 ) {
					line1_pt1_x =  (line1_pt1_x - leftSideIntegerMoveDot6Temp);
					line1_pt2_x =  (line1_pt2_x - leftSideIntegerMoveDot6Temp);
				}

				if ( rot2Dx > 0 ) {
					line2_pt1_x =  (line2_pt1_x + rightSideIntegerMoveDot6Temp);
					line2_pt2_x =  (line2_pt2_x + rightSideIntegerMoveDot6Temp);
				} else if ( rot2Dx < 0 ) {
					line2_pt1_x =  (line2_pt1_x - leftSideIntegerMoveDot6Temp);
					line2_pt2_x =  (line2_pt2_x - leftSideIntegerMoveDot6Temp);
				}
				
				if ( line1_pt2_x == line2_pt1_x ) {
					xx[B] = line1_pt2_x;
				} else {
#if 0
				 	short tempX, tempY;
#else
					F26Dot6   tempX, tempY;
#endif
					/* compute the intersection of line1 and line2 */
					tempX=xB;
					tempY=yB;
					ComputeIntersectionDouble( 
						(F26DotDouble)line1_pt1_x, (F26DotDouble)line1_pt1_y,
								(F26DotDouble) line1_pt2_x, (F26DotDouble)line1_pt2_y,
						(F26DotDouble)line2_pt1_x,(F26DotDouble) line2_pt1_y, 
								(F26DotDouble)line2_pt2_x,(F26DotDouble) line2_pt2_y, 
							&tempX, &tempY, 
							(double) maxMove); /* maxMove is a positive integer, non-zero, for robustness */
					xx[B]= tempX;
					yy[B]= tempY;
					{
						tt_int32 dx, dy, abx, aby, dist;						
						dx = xx[B] - xB;
						dy = yy[B] - yB;						
						abx = dx > 0 ? dx : -dx;
						aby = dy > 0 ? dy : -dy;
						dist = abx > aby ? abx + (aby>>1) : aby + (abx>>1);
						if ( dist > maxMove ) {
#if 0
							dx = dx * maxMove / dist;
							dy = dy * maxMove / dist;
#else
							dx=MultiplyDivide(dx, maxMove, dist);
							dy=MultiplyDivide(dy, maxMove, dist);
#endif
							xx[B] =  (xB + dx);
							yy[B] =  (yB + dy);
						}						
					}
				}
				A = B;
			}
		}
		
			
		/* Repair the outlines */
		for ( ctr = 0; ctr < contourCount; ctr++ ) {
			n =  ep[ctr];
			A = n;
			for ( B =  sp[ctr]; B <= n; B++ ) {
 				if (isVectorFlip(x[A], x[B],xx[A],xx[B]))
 						xx[B] = xx[A];
 				if (isVectorFlip(y[A], y[B],yy[A], yy[B]))
 						yy[B] = yy[A];
				A = B;
			}
		}
 		for ( ctr = 0; ctr <  contourCount; ctr++ ) {
			tt_int32 sum, count;
			n =  ep[ctr];
			A = n;
			sum = xx[A]; count = 1;
			for ( B =  sp[ctr]; B <= n; B++ ) {
				if ( x[B] == x[A] ) {
					sum += xx[B]; count++;
					continue; /*****/
				}
				if ( count > 1 ) {
					sum /= count;
					xx[A] 	= (short)sum;
					xx[--B] = (short)sum;
					while( count-- > 2 ) {
						xx[--B] = (short)sum;
					}
				}
				A = B;
				sum = xx[A]; count = 1;
			}
		}
 	#if 0	
		tsi_DeAllocMem( mem,  x );
	#endif
	}
}



				
		/* Determine Italic Multiplier, based on "em" ratio */
 		static F16Dot16 AdjustItalicMultiplier(
			F16Dot16 originalItalicMultiplier,  /* Multiply times y and add to x */
 			F16Dot16  xPerEm,					/* the x array, with phantom */
 			F16Dot16   yPerEm					/* the y array, with phantom */
  			)
 		 {
 				F16Dot16 italicMultiplier=originalItalicMultiplier;  /* 0 if do nothing */
 				if (xPerEm!=yPerEm)
  					italicMultiplier= MultiplyDivide(italicMultiplier,xPerEm,yPerEm);  
  				return(italicMultiplier);
 		 }


		/* The following routine multiplies a fixed point times F26Dot6, with rounding. */
		/* The multiplier is assumed to 4 times too large, for rounding */
		static F26Dot6 F26Dot6Fix28MulRoundSlant( F26Dot6 x, F26Dot6 y, F16Dot16 multiplier )		
		{
			F26Dot6 mulResult,roundResult,result;
			mulResult=util_FixMul( y,  multiplier );
			roundResult= (mulResult+2)>>2;
			result= x + roundResult; /* add in v */
			return(result);
		}
				
		/* Italic linearly modifies  p.x based on the value of p.y  */
		/* Italic does not affect phantom points.					*/
		/* Apply Italic operates on a simple vector of points. 		*/
		static  void ApplyItalicContour(
			F16Dot16 italicMultiplier,  /* Multiply times y and add to x */
 			F26Dot6 *xValues,			/* the x array, with phantom */
 			F26Dot6 *yValues,			/* the y array, with phantom */
 			tt_int32	count				/* number of points to be operated on. */
 			)
 		{
 			tt_int32 index;				/* loop through all points. */
 			for (index=0;index<count;index++)
  				*xValues++=F26Dot6Fix28MulRoundSlant(*xValues, *yValues++,italicMultiplier);
  			
   		}
				
 /*  Zero Multiplier (param[0[)  means do nothing. */
void tsi_SHAPET_Italic_GLYPH_Hinted( 
	short	contourCount,	/* number of contours in the character */
	short 	pointCount,		/* number of points in the characters + 0 for the sidebearing points */
	short	*sp,			/* sp[contourCount] Start points */
	short	*ep,  			/* ep[contourCount] End points */
 	F26Dot6 *xx,			/* the x array, with phantom */
 	F26Dot6 *yy,			/* the y array, with phantom */
 	F26Dot6 *x,				/* NOT USED, may pass 0 */
 	F26Dot6 *y,				/* NOT USED, may pass 0 */
	tsiMemObject *mem, F16Dot16 xPixelsPerEmFixed,F16Dot16 yPixelsPerEmFixed,
								/*  ORIENTBOLD_STYLES */
	 short curveType,			/* 2 (quadratic) or 3(cubic) */
	 tt_uint8 *onCurve,			/* onCurve[pointCount] indicates if a point is on or off the curve, 
	 								it should be true or false */
	 ContourData *cd,			/* orientation information */

	 F16Dot16 params[] )
{
	F16Dot16 italicMultiplier=   AdjustItalicMultiplier(  params[0],  xPixelsPerEmFixed, 	yPixelsPerEmFixed);
	ApplyItalicContour( italicMultiplier, xx,	yy,	pointCount);
}

/* Apply both bold and italic. */
void tsi_SHAPET_BoldItalic_GLYPH_Hinted( 
	short	contourCount,	/* number of contours in the character */
	short 	pointCount,		/* number of points in the characters + 0 for the sidebearing points */
	short	*sp,			/* sp[contourCount] Start points */
	short	*ep,  			/* ep[contourCount] End points */
 	F26Dot6 *xx,			/* the x array, with phantom */
 	F26Dot6 *yy,			/* the y array, with phantom */
 	F26Dot6 *x,				/* the x array, doesn't need phantom */
 	F26Dot6 *y,				/* the y array, doesn't need  phantom */
	 tsiMemObject *mem, F16Dot16 xPixelsPerEmFixed,F16Dot16 yPixelsPerEm16Dot16, 
								/*  ORIENTBOLD_STYLES */
	 short curveType,			/* 2 (quadratic) or 3(cubic) */
	 tt_uint8 *onCurve,			/* onCurve[pointCount] indicates if a point is on or off the curve, it should be true or false */
	 ContourData *cd,			/* orientation information */
 	 
	 F16Dot16 params[ ]   )
{
	F16Dot16 multiplier;
	multiplier= params[0];
	if (multiplier!=0x10000)		/* Identity means do nothing. */
 		tsi_SHAPET_BOLD_GLYPH_Hinted( 
			contourCount, pointCount, sp, ep,  xx,	 yy, x, y, 
				mem, xPixelsPerEmFixed, yPixelsPerEm16Dot16,  
								/*  ORIENTBOLD_STYLES */
								curveType, onCurve,	cd,	
								 
				 (&params[0] ));

	multiplier= params[1];
	if (multiplier!=0)
 		tsi_SHAPET_Italic_GLYPH_Hinted( 
			contourCount, pointCount, sp, ep,  xx,	 yy, x, y, 
				mem, xPixelsPerEmFixed, yPixelsPerEm16Dot16,
								/*  ORIENTBOLD_STYLES */
								curveType, onCurve,	cd,
 			 (F26Dot6 *) (&params[1] ));

}


 /* Note: in addition to a indeterminant "zeroMode" vector angles,  */
 /* there are exactly 16 possible "angle" modes. */
 /* the "even" modes involve an perfectly precise angle: either an  */
 /* 		an exact axis or 45¡ diagonal.     */
 /* the "odd" modes specify an angle within some ¹/4  radian (45¡) sector of a circle. */
 typedef enum{
 	zeroMode=-1,
  	posX=0, 		/* 0 angle */
 	oct0=1,
 	xyQ1=2,			/*  x==y==|x|==|y| */
 	oct1=3,
 	posY=4,			/* 90 angle */
 	oct2=5,
  	xyQ2=6,			/*  -x==y==|x|==|y| */
 	oct3=7,
 	negX=8,			/* 180 angle */
 	oct4=9,
	xyQ3=10,		/*  -x==-y==|x|==|y| */
 	oct5=11,
   	negY=12,		/* 270 angle */
	oct6=13,
 	xyQ4=14,		/*  x== -y ==|x|==|y| */
	oct7=15	 
 } AnalyzeVectorMode;
 
   
/* This structure returns the angle mode (one of the above enumerations) and */
/*    The original dx,dy which exactly specifies the angle.  				*/
/*  Results are enumerated about. */ 
typedef struct{ 
 	tt_int32  mode; 
 	tt_int32 dx;
	tt_int32 dy;
}  VectorAnalysis;
 

/* Return true if the {dx,dy} vector is other than {0,0} */
/* From the {dx,dy} the vector is classified into 1 of 16 possible modes. */
/* Winding value is returned in the "va" VectorAnalysis structure.  */   
/* The logic to calculate the angle looks complicated, but it involves only */
/*  a couple of binary compares to identify the mode. */
 static int AnalyzeVector(VectorAnalysis *va,  tt_int32 dx,  tt_int32 dy)
 {
	tt_int32 mode,isValid;
	tt_int32 absX, absY;
	va->dx=dx; va->dy=dy;
 	if (dx==0)
 	{
 		if (dy==0)
  			mode=zeroMode; 		
 		else
 		{
  			if (dy>0)
 			  	mode=posY;
 			 else
 			 	mode=negY;
 		}
 	}
 	else
 		if (dy==0)
 		{
 			 /* its along the x-axis. */
 			if (dx>0)
 			  	mode=posX;
 			 else
 			  	mode=negX;
 		}
 		else
 			{
 				/* else its not along either axis. */
 				absX= (dx>0)? dx : -dx; 
 				absY= (dy>0)? dy : -dy; 
 				if (absX==absY)
 				{
 					/* its along a diagonal. */
  					if (dx>0)
  					{
  						if (dy>0)
   							mode=xyQ1;
   						else
   							mode=xyQ4;
    					
  					}
  					else
  					{
  						/* dx<0 */
  						if (dy>0)
  							mode=xyQ2;
  						else
  							mode=xyQ3;
   					}
 				}
 				else
 				{
 					if (absX>absY)
 					{
 						/* Its oct 0, 3, 4, or 7 */
 						if  (dx>0)
 						{
 							/* oct 0 or 7 */
 							if (dy>0)
 								mode= oct0;
 							else
 								mode= oct7;
 						
 						}
 						else
 						{
							/* oct 3 or 4 */
 							if (dy>0)
 								mode= oct3;
 							else
 								mode= oct4;
						}
 					}
 					else
 					{
 						/* Its oct 1, 2, 5, or 6 */
 						if  (dx>0)
 						{
 							/* oct 1 or 6 */
 							if (dy>0)
 								mode= oct1;
 							else
 								mode= oct6;
 						
 						}
 						else
 						{
							/* oct 2 or 5 */
 							if (dy>0)
 								mode= oct2;
 							else
 								mode= oct5;
						}
 					}
  				}
 			
 			}
 	va->mode=mode;
 	isValid= mode!=zeroMode;
 	return(isValid);
 }




/* analyze the angular difference between two non-zero vectors. 							*/
/* return the winding (the "mode" number of transitions ala AnalyzeVectorMode 				*/
/* when a bad winding occurs (and it might occur) then some doubt exists correct value 		*/
/*  However, in practice it should rarely, if ever, cause the sign of the winding to  be 	*/
/* incorrect. The final winding should be 0 Mod 16, unless winding errors occur; even then, */
/*		the result is probably correct */
/* Note: only valid VectorAnalysis structures should be passed. 0 ² mode < 16 				*/
/* 	The routine works by finding the difference between the modes of two vectors. 			*/
/*	Almost always, this is done, by a simple subtraction. A descision is made 				*/
/* about whether the angle is clockwise or counter-clockwise (clockwise is a negative 		*/
/*  result). In the special case where the two vectors point in almost opposite directions, */
/*	a quick crossproduct determines whether to go clockwise or counter-clockwise. 			*/
/*  The final result is a winding number between [-8,8].									*/
/* Note that the routine should never be passed a mode="-1". Such cases (a zero length		*/
/*		vector), must be removed by a higher level routine. 								*/
/* Also, although the algorithm calculates results into 1 of 16 modes, the exact angle		*/
/*		is always preserved in the dx,dy vector, therefore, absolutely no precision is lost.*/
  static int AnalyzeAngle( VectorAnalysis *va1, VectorAnalysis *va2 )
 {
  	double crossProduct;
  	tt_int32 winding=0;
 	tt_int32 direction; /* + is counter-clockwise, - is clockwise, 0 is no mode change. */
 	tt_int32 vec1Mode= va1->mode;
 	tt_int32 vec2Mode= va2->mode;
 	tt_int32 oppositeVec1,isOpposite;
 	 double fx1,fx2;
 	double  fy1,fy2;
  	if (vec1Mode==vec2Mode)
 	{ 
 		/* no change in winding. Both ector*/
   		winding=0;	
 	}
  	else
  	{
  		/* we moved, to some degree */
	   	oppositeVec1= (vec1Mode+8) % 16;
	  	isOpposite= (vec2Mode==oppositeVec1);
	 	if (isOpposite)
	 	{
	 		/* it circled approximately Pi radians, some care */
	 		/* is required to determine if it was positive or negative rotation. */
	 		if  (vec1Mode&1)
	 		{
	 			/*resulting mode includes a range of values, could be  plus or minus */
	 			/* The cross product tells us the "Sin" of the angle, positive or negative */
   				fx1= (double) va1->dx;
   				fy1= (double) va1->dy;
  				fx2= (double) va2->dx;
   				fy2= (double) va2->dy;
   				/* a double precision floating point value will almost always have enough
   					precision to return an exact result. 
   				*/
   	 			crossProduct= ((fx1 *   fy2)  -   (fy1 *   fx2));
	 			if (crossProduct==0.0)
	 				winding=0;		/* well, we just can't tell +Pi or -Pi */
	 			else
	 			if (crossProduct<0.0)
	 				winding=-8;		/* negative cross => clockwise */
	 			else
	 				winding= 8;		/* positive cross => counter-clockwise */
	 		}
	 		else
	 			{
	 				/* We don't know what it is  without looking further */
	 				/* so we don't know whether to return +8 or -8.  So return 0; rarely happens. */
	 				winding=0;
 	  			}
	 	}
	 	else
 	 		/* its not in the opposite mode, so we can determine the winding without */
	 		/* needing a cross-product. 												 */
	 		winding= (   (   (vec2Mode-vec1Mode +24) % 16 ) - 8 );
    	}
 	return(winding);
 }
 
 
 /* Determine the orientation of a contour. 
 	Special rules:
 		1) For zero movement, ignore the point.
 		2) Always wrap-around to the first point. (using 1)
 		3) Assume all rotations <180 degrees.
 		4) Ignore 180 degree turn, which shouldn't occur.
 */


 /* return a winding number. ==0 implies no winding. >0 is counter-clockwise, <0 clockwise */
int FindContourOrientation( F26Dot6 *x, F26Dot6 *y, tt_int32 count)
 {
 	 tt_int32 winding =0; /* accumulate winding */
   	 VectorAnalysis  vaFirst,vaLast,vaNext;
  	 tt_int32 index,firstIndex,nextIndex;
 	 tt_int32 result=0,   xFirst, yFirst, xLast,yLast, xNext, yNext;
 	 tt_int32 x1Diff, y1Diff, dx,dy;
 	 /* If too few vectors to determine orientation, then exit.*/
 	if (count<3)
 	{
 		result=0;
 		goto exit;
 	
 	}
 	
 	/* Set Start Point for the first vector. */ 
  	xFirst= x[(count-1)] ;
 	yFirst= y[(count-1)];
 
 	/* find the first non-zero change. */
 	for (index=0;index<(count-1);index++)
 	{
 		xLast= x[index];
 		yLast= y[index];
 		x1Diff= xLast-xFirst;
 		y1Diff= yLast-yFirst;
 		if ( AnalyzeVector(&vaFirst, x1Diff, y1Diff ) )
 			{
 				/* We found a non-zero vector. */
  				nextIndex=index+1;
 				goto gotone;
 			}
 	}
 	result=0;  /* all points are the same- no orientation. */
 	goto exit;
gotone:
	vaLast=vaFirst;
	/* see if we can find another non-zero vector. */
  	for (index=nextIndex;index< count;index++)
 	{
 		xNext= x[index];
 		yNext= y[index];
 		dx=    xNext-xLast;
 		dy=    yNext-yLast;
 		if ( AnalyzeVector(&vaNext, dx, dy ) )
 		{
 		 	winding+=AnalyzeAngle( &vaLast, &vaNext);
 		 	xLast=xNext;
 		 	yLast=yNext;
 		 	vaLast=vaNext;
  		}
    			/* else  completely skip if there is no dx or dy */
 	}
 	/* we have one more angle... */
 	winding+=AnalyzeAngle( &vaLast , &vaFirst );
 	result=winding;
  exit:
 	return(result);
}


 /* Same as the above routine, but the input data is short rather than F26Dot6 */
/* If this routine is used,then cross products (see AnalyzeVector) don't need */
/*  to be double- they can be long multiplies. ).	*/
int FindContourOrientationShort ( tt_int16 *x, tt_int16 *y, tt_int32 count)
 {
 	 tt_int32 winding =0; /* accumulate winding */
   	 VectorAnalysis  vaFirst,vaLast,vaNext;
  	 tt_int32 index,firstIndex,nextIndex;
 	 tt_int32 result=0,   xFirst, yFirst, xLast,yLast, xNext, yNext;
 	 tt_int32 x1Diff, y1Diff, dx,dy;
 	 /* If too few vectors to determine orientation, then exit.*/
 	if (count<3)
 	{
 		result=0;
 		goto exit;
 	
 	}
 	
 	/* Set Start Point for the first vector. */ 
  	xFirst= (tt_int32) x[(count-1)] ;
 	yFirst= (tt_int32) y[(count-1)];
 
 	/* find the first non-zero change. */
 	for (index=0;index<(count-1);index++)
 	{
 		xLast= (tt_int32) x[index];
 		yLast= (tt_int32) y[index];
 		x1Diff= xLast-xFirst;
 		y1Diff= yLast-yFirst;
 		if ( AnalyzeVector(&vaFirst, x1Diff, y1Diff ) )
 			{
 				/* We found a non-zero vector. */
  				nextIndex=index+1;
 				goto gotone;
 			}
 	}
 	result=0;  /* all points are the same- no orientation. */
 	goto exit;
gotone:
	vaLast=vaFirst;
	/* see if we can find another non-zero vector. */
  	for (index=nextIndex;index< count;index++)
 	{
 		xNext=(tt_int32) x[index];
 		yNext=(tt_int32) y[index];
 		dx=    xNext-xLast;
 		dy=    yNext-yLast;
 		if ( AnalyzeVector(&vaNext, dx, dy ) )
 		{
 		 	winding+=AnalyzeAngle( &vaLast, &vaNext);
 		 	xLast=xNext;
 		 	yLast=yNext;
 		 	vaLast=vaNext;
  		}
    			/* else  completely skip if there is no dx or dy */
 	}
 	/* we have one more angle... */
 	winding+=AnalyzeAngle( &vaLast , &vaFirst );
 	result=winding;
  exit:
 	return(result);
}
 

	/*  ORIENTBOLD_STYLES */
	/* Now that we have the glyph, we can check the contours. 			*/
	/* Call this routine on the raw glyph, before scaling and all else. */	
void	AccumulateGlyphContours( 
	ContourData *cd, 		/* Update this contour data. */
	GlyphClass *glyph  		/* This is the glyph,with oox,ooy 	*/
							/* containing data.				*/
 )
	{
		ScanInputBlock sib;
		short contourCount;
		short pointCount;
		if ((glyph->contourCount>0)&& cd->initializedContour && cd->active )
		{
 			tt_int32 startIndex=0, endIndex=0, i, 
					pointCount=glyph->pointCount, 
					contourCount=glyph->contourCount,
					ctr, pointIndex, startPoints, endPoints;
 			SetOrientBlock( &sib, contourCount,
				glyph->sp,glyph->ep,Orient16BitArray,glyph->oox,glyph->ooy,
				glyph->onCurve, (char)glyph->curveType);
 			/* now process these points. */
 			{
 				tt_int32 localOrient=0; 
				tt_int32 globalOrient=0;
				tt_int32 ctr;
				tt_int32 pointsWithinContour;
				tt_int32 winding;	/* positive for counter-clockwise, negative for clockwise. */
	
 				for (ctr=0;ctr< contourCount; ctr++)
 				{
 					startPoints = glyph->sp[ctr];
 					endPoints = glyph->ep[ctr];
   					pointsWithinContour= endPoints-startPoints+1; 
			
#ifdef ORIENTBOLD_STYLES					
					/* Some contours may be oriented opposite of the standard.	*/
					winding= FindContourOrientationShort( 
						&glyph->oox[startPoints], &glyph->ooy[startPoints], pointsWithinContour);
	 				if (glyph->curveType==2)	/* only quadratics for now. */
						globalOrient=FindNonZeroWindingCounts( &sib, ctr,0);
					localOrient= (winding>0);
	 				SetContourDataSet(cd,ctr+cd->current,localOrient, globalOrient);
#else
	 				SetContourDataSet(cd,ctr+cd->current,0, 0);
#endif
 				}
  			}
   			cd->current+= contourCount;
 		}	
	}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
