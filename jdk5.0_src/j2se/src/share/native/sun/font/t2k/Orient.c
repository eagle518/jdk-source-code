/*
 * @(#)Orient.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/* Orientation: this code determines whether black bits are on the left	*/
/* or right of a contour. This is done by picking a "nice" point on 	*/
/*  the curve, and then examining all other curve segments of all		*/
/* contours of the glyph. Its similar to scanline conversion, but		*/
/* no pixels are generated.												*/
/* The primary code design is to be easily tested with					*/
/*  no memory allocation.												*/
/*	The algorithm uses no sorting.										*/
 
//#include <cmath>
#include "Orient.h"

 
#define 	 MAXFINDVALUESPLITS	10
#define 	 MAXSETUPSPLITS 16


static void SplitQuadratic(
			QuadraticBezier *qb, QuadraticBezier *qb1, 
			QuadraticBezier *qb2, double t);

volatile tt_int32 reversals=0,rarelyHappens=0;
#ifdef ORIENTATIONTEST
 	static void RarelyHappens(void)
	{
		rarelyHappens++;
	}
	static void NeverHappens(void)
	{
		BadOrientationState( );
	}
	static void BadState(void)
 	{
		BadOrientationState( );
	}
	
	static void TestIntOffGrid(double d)
	{
		if (  ((double) ((tt_int32) d) ) != d )
			 BadState();
	}
	static void TestBetweenExclusive(double low, double middle, double high)
	{
		if (  (middle<=low) || (middle>=high))
			 BadState();
	}
	static void TestBetweenInclusive(double low, double middle, double high)
	{
		if (  (middle<low) || (middle>high))
			 BadState();
	}
	static void TestTweenClipInclusive(double v0, double v1, double v2)
	{
		if (v0<=v2)
		 TestBetweenInclusive(v0, v1, v2);
		else
		 TestBetweenInclusive(v2, v1, v0);
	}
 
	static void GotReversal(void)
	{
		reversals++;
	}
#endif

/* This routine sets up the description of the input glyph, so 	that 	*/
/* it can be passed around.												*/
void SetOrientBlock( 
	ScanInputBlock *sib,
	short numberOfContours, short *startPtr, 
	short *endPtr, 
	tt_int32 arrayType,void *xPtr, void *yPtr, 
	unsigned char *onCurvePtr, 
			       char curveType )
{
  	sib->numberOfContours= numberOfContours;
 	sib->startPtr= startPtr;
	sib->endPtr= endPtr;
	sib->arrayType=arrayType;
	sib->xArray= (void *) xPtr;
	sib->yArray= (void *) yPtr;
  	sib->curveType= curveType;
 	sib->onCurvePtr= onCurvePtr;
}

/* The first step, in checking orientation for a specific contour, 		*/
/*	is to find a useful curve segment.  								*/
/* This routine initializes information about the the segment. 			*/
/*	In particular, the contour  index and the index of a specific X,Y 	*/
/* vertex point which defines the curve segment. 						*/
/* The routine also initializes counters which are used to record 		*/
/* winding counts. This routine does not initialize						*/
/* the coordinates of the specific scanPoint (x,y), 					*/
/*	which is the location of the exact point which is					*/
/* under evaluation. The (x,y) values are initialized somewhat after 	*/
/* this routine is called.												*/

static void InitScanPointStructure(
	ScanPoint *scanPoint, tt_int32 thisContourIndex, tt_int32 thisPointIndex)
{
	/* Hold off on the initialization of the x,y scan position. 		*/
	/* double x;			 x position of check point. 				*/
	/* double y;			 y position of check point. 				*/
	
	
	/* Record the contour and specific vertex. Because it will			*/
	/* not be involved int he winding counts.							*/
	scanPoint->thisContourIndex=thisContourIndex;
	scanPoint->thisPointIndex=thisPointIndex;

	/* The direction is the sign of  y-derivative at the scan point. 	*/
	/*  1=up, -1 = down, 0 not allowed 		  							*/
	scanPoint->direction=0;	/* direction of curve at point position. 	*/
						
	scanPoint->leftSideUpards=
		scanPoint->leftSideDownwards=
			scanPoint->rightSideUpwards=
				scanPoint->rightSideDownwards=
					scanPoint->sameXUp=
					scanPoint->sameXDown=
			0;
}
 
 /* The input points are assumed to be separated by at least 1/4		*/
 /* of a grid point, which is assurred for integer inputs to entire 	*/
/*	process.															*/ 
#define  	offgridMultiply  8.
#define   	offgridRestore (1./offgridMultiply )

 
/* Find a point that is off the grid (e.g, not an integer ) 			*/
/* or 1/2 or 1/4 an integer. But off the grid by a lot.					*/	
/* The easiest way off the grid is to find points on the grid.			*/
/* Motivation: by picking a point which is off the y-grid, it guarantees*/
/* that the resulting point cannot match an end point of any constructed*/
/* spline. Even endpoints contructed from other splines.				*/
/* note that input values y0 and y1 do  need to be some type of grid.	*/
/*   																*/
static double GetOffGrid(double y0, double y1)
 {
 	double use0, use1;
 	double on0, on1, halfDiff,  offGrid,midway,offGridSet,midPosition,
 			final,temp,yy0, yy1;
 	
 	/* We will always work with in scaled mode. The input points		*/
 	/*In general, the input points should be integers after scaling.	*/
 	/* but this is not really necessary.								*/
 	
 	/* Standardize input with us0<use1 */
 	if (y1<y0)
 		{ 
 			yy0=y1; 
 			yy1=y0;
 		}
	else
		 { 
 			yy0=y0; 
 			yy1=y1;
 		}

 	use0= yy0*offgridMultiply;
 	use1= yy1*offgridMultiply;
 	
#ifdef ORIENTATIONTEST
				TestIntOffGrid(use0);
				TestIntOffGrid(use1);
#endif 	
  	
 	/* Look for two identical values: it should not happen. */
 	if (use0==use1)
 	{
#ifdef ORIENTATIONTEST
		BadState( );
#endif
 		return(y1);	 
 	}				 
 	
 	/* Determine the midpoint, and the offset to correct grid.  */
 	/* This value is chosen small enough that the final point	*/
 	/* still lies between use0 and use1.						*/
	halfDiff= .5 * (use1-use0);
 	midway= use0+halfDiff;
 	if (halfDiff>.5)
 		offGridSet= .25;
 	else
 		offGridSet= halfDiff * .5;
 	
 	/* Now determine where the midway point with respect		*/
 	/*	to the scaled grid.	It might land on an exact integer,	*/
 	/* and its perfectly okay.									*/							
	on0=floor(midway);
 	on1=ceil(midway);
 	if (on0==on1)
 	{
  		/* Its on an exact grid point. Its easy to go off grid! */
 		offGrid= midway-offGridSet; 
 		/* or +offGridSet, it doesn't matter*/
 	}
 	else
 	{
 		/* we now find the midpoint between on0 and on1			*/
 		/* and compare it against the midway point.				*/

 		midPosition= .5 * (on0+on1);
 		/* goto midway, and move towards furthest away */
   		if  (midway<midPosition)
   			offGrid= midway+offGridSet;
   		else
   			offGrid= midway-offGridSet;
 	}
	/* Scale the value back to the original scale. 			*/
 	final=offGrid *offgridRestore;
#ifdef ORIENTATIONTEST
 	TestBetweenExclusive( yy0,  final,  yy1);
#endif
 	return(final);
 }
 
/* see if the y value is outside  range of the convex hull */
static tt_int32 IsOutsideConvexHull( QuadraticBezier *qb, double y)
{
	return(  (
					( ( y< qb->y0)  && ( y< qb->y1) && ( y< qb->y2)  )
							||
					( ( y> qb->y0) && ( y>qb->y1) && ( y> qb->y2 )  )
		   )     );
 }

  
 static double EvaluateQuadraticAtForAxis(
		double x0, double x1, double x2, double t);
 
 
/* Find the x value for a specified y-axis value.			*/
/* generally used only on monotonic splines.				*/
#ifdef ORIENTATIONTEST
volatile tt_int32 abc=0;
#endif

static  double GetSplineValueOnY( 
	double y, 				/* the y-axis position.			*/
	QuadraticBezier *orgQB, /* original bezier.				*/
	tt_int32 maxSubdivisions, 	/* Maximum subdivisions.		*/
	tt_int32 *direction,		/* return monotonic direction.	*/
	double *t				/* return the t value. 			*/
	)
{
	QuadraticBezier *qb,qbData;
	double t0 , t2 , midt, finalt;
	double midY, midX, finalX;
	int i,aDirection;
	
	qb= &qbData;
	qbData= *orgQB; /* Copy data, because its destroyed. */

	/* return the direction and setup ascending spline. */
	if (qb->y0>qb->y2)
	{
		aDirection=  -1 ;
		/* reverse quadratic. */
		qb->y0= orgQB->y2; qb->x0= orgQB->x2; 
		qb->y2= orgQB->y0; qb->x2= orgQB->x0; 
		t0=1.;t2=0.;
	}
	else
	{
 		aDirection=  1 ;
		t0=0.;
 		t2=1.;
 	}
 	*direction= aDirection;
 	/* init qb pointer.		*/
	
#ifdef ORIENTATIONTEST
	/* check input parameters. */
 	TestBetweenInclusive( qb->y0,  qb->y1,  qb->y2);
 	TestBetweenInclusive( qb->y0,  y,  qb->y2);
#endif

	
	/* If it is an endpoint, then we are already done.		*/
	if (y==qb->y0)
		return(qb->x0);
		
	if (y==qb->y2)
		return(qb->x2);
	
	/* Loop thru all subdivisions. It could try to 			*/
	/* terminate early, as do some scanline conversions		*/
	/* algorithms, but it is essential to get the right		*/
	/* answer, to a very close (very tiny) tolerance.		*/
	for (i=0;i<maxSubdivisions;i++)
	{
		/* subdivide, exactly in half. 	*/
		midt = .5 * (t0+t2);
		midX = .25 * (qb->x0+qb->x2) + .5 * qb->x1;
		midY = .25 * (qb->y0+qb->y2) + .5 * qb->y1;
		
		/* if the new y-midpoint is an exact match			*/
		/*	then return midX, the x middle point.			*/

		if (y==midY) {
		  *t=midt;
		  return midX;
 		}
 			
 		/* Otherwise, the y value lies in 1 of the two.		*/
 		/* split halves.  				      	*/

 		if (y<midY) {
			t2=midt;
			qb->x1= .5 * (qb->x0+qb->x1);
			qb->y1= .5 * (qb->y0+qb->y1);
			qb->x2= midX; 
			qb->y2= midY;
		}
			else
			{
				t0=midt;
		 		qb->x1= .5 * (qb->x2+qb->x1);
				qb->y1= .5 * (qb->y2+qb->y1);
				qb->x0= midX; 
				qb->y0= midY;
			}
		/* and loop until done. */
	};
	/* we have gone far enough, interpolate 				*/
	/* note the denominator. Its non-zero, because 			*/
	/* of monotonicity.										*/
	{
		double multiplier;
		multiplier= ((y-qb->y0)/(qb->y2-qb->y0));
		finalX=qb->x0+ multiplier* (qb->x2-qb->x0);
		finalt=t0+ multiplier * (t2-t0);
	}

#ifdef ORIENTATIONTEST
	/* check input parameters. */
 	TestTweenClipInclusive( qb->x0,  finalX,  qb->x2);
	TestTweenClipInclusive( 0.,  finalt,  1.);
 	if ( orgQB->y1!= orgQB->y0 && 	orgQB->y1!= orgQB->y2)
 	{
   		abc++;	
 	}
#endif
	*t= finalt;
 	return ( finalX );
} 

/* 	 Assuming that the control point, y1, is not between y0 and y2		*/
/* 		find the t-value which splits the quadratic into a balanced		*/
/*		quadratic plus a leftover "overhang" quadratic.					*/
static tt_int32 SplitQuadraticAtEndPoint(
	QuadraticBezier *qb, 			/* input  quadratic */
	QuadraticBezier *qbBalanced,	/* balanced  quadratic */
	QuadraticBezier *qbOverhang,			/* extra overhang quadratic */
	tt_int32 maxSubdivisions)
{
	double qb1diff, qb1abs, qb2diff, qb2abs;
	double t,newXValue;
 	tt_int32 direction;
	 


	if (qb->y0==qb->y2)
	{
#ifdef ORIENTATIONTEST
		BadState();								
#endif
	 	return(1);			 
 	}
 	
 	qb1diff= qb->y1- qb->y0;
 	qb2diff= qb->y1-qb->y2;
 	
 	/* the differences must be non-zero with the same sign. */
 	if (		(qb1diff==0)
 					|| 
 				(qb2diff==0) 
 					|| 
 				( (qb1diff<0) && (qb2diff>0) )
 					|| 
 				( (qb1diff>0) && (qb2diff<0) )
 		)
 	{
#ifdef ORIENTATIONTEST
		BadState();		/* Really bad, should never get here. */					
#endif
		return(1);				
	}

	/* find absolute value of differences. */
	if (qb1diff<0)
	{
		qb1abs=-qb1diff;
		qb2abs=-qb2diff;
	}
	else
	{
		qb1abs=qb1diff;
		qb2abs=qb2diff;
	}
	/* is it an exact match? no, because qb->y0!=qb->y2 		*/
	/* Is the zero derivative in the first or second quadratic  */
	if (qb1abs>qb2abs)
	{
		/* find a match in the first quadratic by using end of second. */
		/* 																
		 					 		X					
							 	X       X  		 
					match-->  M          y2
						|	 X
			overhang	|	X
						|  y0
		*/
		t = 	(qb->y0 - qb->y2)/(qb->y0 - 2*qb->y1 + qb->y2);
		SplitQuadratic( qb, qbOverhang, qbBalanced, t);  					  		
#ifdef ORIENTATIONTEST
		TestTweenClipInclusive( .9999 * qbOverhang->y2, qb->y2, 1.00001*qbOverhang->y2);
		TestTweenClipInclusive( .9999 * qbBalanced->y0, qb->y2, 1.00001* qbBalanced->y0);
#endif
		qbOverhang->y2 = qbBalanced->y0= qb->y2; /* make it an exact match */
  	}
	else
	{
		/* find a match in the second quadratic by using end of first. */
#if 0									
		/* 																
		 					 		X					
							 	X       X  		 
					match-->  M          y0
						|	 X
			overhang	|	X
						|  y2
		 
		*/
#endif

		t = 	(2*(qb->y0 - qb->y1))/(qb->y0 - 2*qb->y1 + qb->y2);
 		SplitQuadratic( qb, qbBalanced, qbOverhang, t);  
#ifdef ORIENTATIONTEST
		TestTweenClipInclusive( .9999 * qbOverhang->y0, qb->y0, 1.00001 * qbOverhang->y0 );
		TestTweenClipInclusive( .9999 * qbBalanced->y2, qb->y0, 1.00001 * qbBalanced->y2 );
#endif
		qbOverhang->y0= qbBalanced->y2=qb->y0; /* make it an exact match */
		
	}
#ifdef ORIENTATIONTEST
		TestTweenClipInclusive( .0,  t,  1.);
#endif
	return(0);

}


/* 	GetXCurveRelationship compares the scanPoint "x" value against 		*/
/*		another x-value, the curveX, which lies on a curve.				*/
static tt_int32 GetXCurveRelationship(double x,double curveX)
 	{
 	/* now find out the relationship. */
	if (x<curveX)
		return(XisLEFTofCurve);
	if (x>curveX)
		return(XisRIGHTofCurve);
	else 
	 	{
			/* This result indicates that the chosen scanPoint lies on the intersection */
			/* of another curve- which is extremely rare for curves, but sometimes		*/
			/* hinting very small fonts causes two vertical lines to align on top of 	*/
			/* each other- producing both black to black or white to white tranisitons. */
	#ifdef ORIENTATIONTEST
			RarelyHappens();
	#endif
			return(XisAMBIGUOUS);
		} 
	}


/* Assumption: y-axis must be monotonic!!						*/
/* Determine if the curve x value is to the right or left 		*/
/*    of the x,y position. 										*/
/* Use only on monotonic splines. y must be part of the qb. 	*/
/* 	qb is destroyed. 											*/
 	 
static tt_int32 CheckLeftOrRight( 
	double x, double y, QuadraticBezier *qb, 
	tt_int32 maxSubdivisions, tt_int32 *direction)
{
	double theX, curveX,t;
	tt_int32 aDirection,dir,result;
	/* return direction of quadratic-remember its monotonic. 	*/
	aDirection= qb->y0>qb->y2 ? -1 :+1;
	*direction= aDirection;
	
#ifdef ORIENTATIONTEST
	TestTweenClipInclusive(qb->y0, qb->y1, qb->y2);
#endif
	
	/* Check for instant resolution. 	This occurs when the	*/
	/* y value is on the endpoint of the curve.					*/
	if (y==qb->y0)
		curveX= qb->x0;
	else 
	if (y==qb->y2)
		curveX= qb->x2;
	else
	{
 	/* look for clipping solution. This is much faster than 	*/
	/*  running the subdivision check.							*/
		if ( (qb->x0<x) && (qb->x1<x) && (qb->x2<x) )
			curveX= qb->x0;	/* or qb->x1 or qb->x2 */
		else if ( (qb->x0>x) && (qb->x1>x) && (qb->x2>x) )
			curveX= qb->x0;	/* or qb->x1 or qb->x2 */
 		else
			/* otherwise do a quick run.  */
 			curveX= GetSplineValueOnY( y, qb, maxSubdivisions, &dir, &t);
 	}
 	result=GetXCurveRelationship(x,curveX);
 	 return(result);
 }

    
/* This routine finds a non-grid y-scanline  position. 						*/
/*  That is, suppose we find a line segment (defined by its second vertex).	*/
/*	The next step is to find the best point on the line segment. This point	*/
/* is the scanPoint, and completes the initialization described in the		*/
/* 	earlier routine, InitScanPointStructure.								*/
/*  We wish to choose a point, double precision, which does not lie on		*/
/*	on the y-axis grid. This will avoid problems when the winding			*/
/* 	crossings are determined.												*/
/*   			*/
/* Assumption: yPrev != yThis, checked prior to calling this routine. 		*/	
/*		That is, the windings are not attempted from a line segment			*/
/*		which has no horizontal change.										*/
		
		
		
static void GetNonGridXPointOnLineSegment(
	tt_int32 xPrev, tt_int32 yPrev, tt_int32 xThis, tt_int32 yThis,
	ScanPoint *scanPoint)
 {
 	tt_int32 direction;
	double doubleRatio;
	double yPrevDouble,yThisDouble,xPrevDouble,xThisDouble,yLoc;

	/* Rremember direction !=0. yThis must not equal yPrev! 				*/
	if (yThis>yPrev)
		direction= 1;
	else
		direction= -1;
	/* Save the direction. */
	scanPoint->direction=direction;
 	 
	/* First find a offgrid y-axis value between start and end.				*/
 	yPrevDouble= ( double) yPrev;
	yThisDouble=  (double) yThis;
 	scanPoint->y= yLoc = GetOffGrid( yPrevDouble, yThisDouble);
 	
 	/* now interpolate the x-value. */
 	doubleRatio= (yLoc-yPrevDouble)/(yThisDouble-yPrevDouble);
 	xPrevDouble= (double) xPrev;
 	xThisDouble=  (double) xThis;
	scanPoint->x = xPrevDouble+doubleRatio* (xThisDouble-xPrevDouble);
#ifdef ORIENTATIONTEST
	 TestTweenClipInclusive(xPrevDouble, scanPoint->x, xThisDouble);
#endif
 
 }
	


	
		/* Forward static declarations. */		
		static void XGetQuadraticBezier(QuadraticBezier *qb,
			tt_int32 xPrev, tt_int32 yPrev, tt_int32 onCurvePrev,
			tt_int32 xThis, tt_int32 yThis,  
			tt_int32 xNext, tt_int32 yNext, tt_int32 onCurveNext );	
		static tt_int32 EvaluateCurveType(QuadraticBezier *qb);
		static tt_int32 BalancedSetupYAxisScan( 
			QuadraticBezier *qb, ScanPoint *scanPoint );
		static tt_int32 SingleSetupYAxisScan(
			QuadraticBezier *qb, ScanPoint *scanPoint, double yScanValue);
			
/* Try to setup a point on a quadratic spline.								*/
/* Fails if there is no y-axis change.										*/
/* return non-zero if it couldn't find a good scan point.					*/
/* This is the quadratic version of "GetNonGridXPointOnLineSegment"			*/

static tt_int32 GetNonGridXPointOnQuadratic(
	CurveInput *ci, ScanPoint *scanPoint)		
{		
	QuadraticBezier 	quadraticBezier,*qb;
	double 				midT;
	tt_int32 				yPrevDiff,yNextDiff,someYChange,xPrevDiff,
						xNextDiff,someXChange;
 	tt_int32 			result,zeroTForm;
 	double			useY, zeroDerivY;
 	qb= &quadraticBezier;
 	yPrevDiff= ci->yThis- ci->yPrev;
	yNextDiff=  ci->yThis- ci->yNext;				
	someYChange= (yPrevDiff) || (yNextDiff );
	if (someYChange==0)
		return(1);		/* fails, no y-axis change. */
		
	xPrevDiff=  ci->xThis- ci->xPrev;
	xNextDiff=  ci->xThis- ci->xNext;				
	someXChange= (xPrevDiff) || (xNextDiff );

	/* Get double precision version of spline. */
	XGetQuadraticBezier(
		qb,ci->xPrev, ci->yPrev, ci->onCurvePrev, 
		ci->xThis, ci->yThis,  ci->xNext, ci->yNext, ci->onCurveNext);	
		
	/* determine when y-derivative is zero. */
 	/* find when the y-derivative changes direction. */
	zeroTForm= EvaluateCurveType(qb);
	/* Depending upon y-derivative, create the scanPoint. */	
	switch (zeroTForm)
	{
 		case YDerivZeroAlwaysNoT:  
#ifdef ORIENTATIONTEST
				BadState( );
#endif
 				result=1; /* Never occurs because someYChange !=0 */;
 				break;			
 				 				
		case YDerivZeroY0EqualY2:
				/* All scan lines are intersected twice. */
				if ( someXChange )
					result= (BalancedSetupYAxisScan(qb, scanPoint));
 				else			/*  not a useful curve for processing. 		*/
 					result= (1);/* Because the curve doubles back on itself.*/
 				break;			
					 					
		case YDerivZeroAtT1:
		case YDerivZeroAtT0:
 		case YDerivMonotonicAnyT: 
				/* All scan lines are crossed only once. */
				useY= GetOffGrid( qb->y0, qb->y2);	
				/* setup grid point. 					*/	
				result=( SingleSetupYAxisScan(qb,  scanPoint, useY));
			break;			

		case YDerivZeroControlOutside:
				if ( someXChange )
				{  
					QuadraticBezier qbBalanced, qbOverhang; 
					tt_int32 result=0;
					result= SplitQuadraticAtEndPoint(
						qb, 			/* input  quadratic */
						&qbBalanced,	/* balanced  quadratic */
						&qbOverhang,	/* extra overhang quadratic */
						MAXSETUPSPLITS);
					if (result!=0)
						{
#ifdef ORIENTATIONTEST
							BadState();
							return(1);
#endif
						}
					/* We have two curves: a balanced curve, and an overhang.		*/
					/* we can use either to setup.									*/
					if (1) /* Always true	*/
					{
						/* This case is like any other monotonic case.				*/
						useY= GetOffGrid(qbOverhang.y0, qbOverhang.y2);	
						if (!IsOutsideConvexHull( &qbOverhang, useY))
							result=( SingleSetupYAxisScan(&qbOverhang,  scanPoint, useY));
					}
					else
					{
						/* Never use this path, because it may generate control	*/
						/* points which are not on the grid or 1/2 the grid.	*/
						/* Too bad, its less code to test. 						*/
#ifdef ORIENTATIONTEST
						BadState();
#endif
 						result=  BalancedSetupYAxisScan(&qbBalanced, scanPoint);
					}
 				}
 				else			/*  not a useful curve for processing. 		*/
 					result= (1);/* Because the curve doubles back on itself.*/
 				break;			
 			
	 					
 	 			
		default:
#ifdef ORIENTATIONTEST
			BadState( );
#endif
			result=1;	 /* Should never reach here. */	
	}
	return(result);
 }	
			
 
 

	/* Perform a quadratic bezier evaluation for one axis, for any t. 		*/
	/* Although faster methods are available, this method is stable.	*/
	static double EvaluateQuadraticAtForAxis(
		double x0, double x1, double x2, double t)
	{
		double leg1, leg2,result;
		leg1= (x0+(t * (x1-x0))) ;
		leg2= (x1+(t * (x2-x1))) ;
		result= leg1 + t * (leg2-leg1);	
		 return ( result );			
 	}

	/* Perform a quadratic bezier evaluation for both axes, for any t 		*/ 
	static void EvaluateQuadraticAt(
		QuadraticBezier *qb, double t, double *x, double *y )
	{
		*x=  EvaluateQuadraticAtForAxis(qb->x0,qb->x1,qb->x2,t);
		*y=  EvaluateQuadraticAtForAxis(qb->y0,qb->y1,qb->y2,t);
	}
	
	 	
	/* The quadratic bezier definition is determined by the onCurveFlag.		*/
	/* That is, it alters the position of the endPoints of the quadratic curve.	*/
	/* Its important to keep discrete results. 									*/
	/* For example, this function returns a value which is						*/
	/* 		either on a grid point, or exactly 1/2 of a grid point. 			*/
	static double GetQuatraticControlPointOneAxis(
		tt_int32 onCurveFlag, tt_int32 endPoint, tt_int32 middleControl)
	 { 
	 	double result;
	 	if (onCurveFlag)
	 		result= (double) endPoint;
	 	else
	 		result= ( .5 * ( 
	 							((double) middleControl) +
	 							((double) endPoint)
	 						)
	 					 );
	 	return(result);
	 }


 /* This routine sets up the  QuadraticBezier structure which defines the		*/
 /* 		quadratic bezier curve in terms of two endPoint vertices and 		*/
 /*			an off-curve control point: the Traditional form.					*/
static void XGetQuadraticBezier(QuadraticBezier *qb,
	tt_int32 xPrev, tt_int32 yPrev, tt_int32 onCurvePrev,
	tt_int32 xThis, tt_int32 yThis,  
	tt_int32 xNext, tt_int32 yNext, tt_int32 onCurveNext )
{
	double x0,  y0,   x2,   y2;
	qb->x0=  GetQuatraticControlPointOneAxis(onCurvePrev, xPrev, xThis);
	qb->y0=  GetQuatraticControlPointOneAxis(onCurvePrev, yPrev, yThis);
	qb->x1= (double) xThis;
	qb->y1= (double) yThis;
	qb->x2= GetQuatraticControlPointOneAxis(onCurveNext, xNext, xThis);
	qb->y2= GetQuatraticControlPointOneAxis(onCurveNext, yNext, yThis);
}




	/* Setup the x,y values and direction for the scan point. 					*/
 	/* This routine evaluates the quadratic to find the x,y position, and also	*/
	/* 		determines the direction of the y-axis.								*/
	/*	Assumption: monotonic y-axis.											*/
	static tt_int32 SingleSetupYAxisScan(
		QuadraticBezier *qb, ScanPoint *scanPoint, double yScanValue)
	{
		double  xValue;
	  	tt_int32  direction;
	  	double t;
	  	scanPoint->y= yScanValue;
		scanPoint->x=GetSplineValueOnY( 
			yScanValue, qb, MAXSETUPSPLITS, 
			&direction,&t);
		scanPoint->direction=direction;
		return(0);
	}
	
	static tt_int32 UpdatePoint(ScanPoint *scanPoint, tt_int32 side, tt_int32 direction)
	{
#ifdef ORIENTATIONTEST
				if (direction==0)
					BadState();
#endif
		if (side==XisAMBIGUOUS)
		{
			/* the method of choosing the scanPoint should eliminate		*/
			/*  any chance of an ambiguos update. However, some fonts.		*/
			/*  may have peculiar intersections, and such.					*/
			/* Or perhaps the rounding process has done something silly.	*/
#ifdef ORIENTATIONTEST
			RarelyHappens();
#endif
 			if (direction>0)
 					scanPoint->sameXUp++;
 				else
					scanPoint->sameXDown++;
		}
		else
			if (side==XisLEFTofCurve) /* left side */
			{ 
				if (direction>0)
 					scanPoint->rightSideUpwards++;
 				else
					scanPoint->rightSideDownwards++;
			}
			else /* right side */
			{
				if (direction > 0)
 					scanPoint->leftSideUpards++;
 				else
					scanPoint->leftSideDownwards++;
			}
		return(0);
	
	}


	
  	/* Use the "useY" value, which lies on the qb curve, and find two x-values	*/
	/*			on the curve at useY scanline. Pick one for updating, and 		*/
	/*			use the other to initialize the winding.						*/
	/*		Assume that y0==y2 !!!												*/
	/* return non-zero if something goes wrong. 								*/	
	static tt_int32 BalancedSetupYAxisScan( 
		QuadraticBezier *qb, ScanPoint *scanPoint )
	{
		tt_int32 direction1, side2,  direction2, setup,result;
		double useY,zeroDerivY;
		QuadraticBezier qb1,qb2;
		
#ifdef ORIENTATIONTEST
		if (qb->y0!=qb->y2)
			BadState();
#endif

		zeroDerivY=qb->y0 + ( qb->y1-qb->y0)*.5; /* BECAUSE IT IS BALANCED! */
		/* now choose a point about 1/2 way. */
		useY=  GetOffGrid(qb->y0, zeroDerivY); 
		SplitQuadratic( qb, &qb1, &qb2, .5);
		result=SingleSetupYAxisScan( &qb1, scanPoint, useY);
#ifdef ORIENTATIONTEST
		if (result!=0)
			BadState();
#endif
		if (result==0)
		{
  			side2= CheckLeftOrRight(
  						scanPoint->x,scanPoint->y,  &qb2, 
						MAXFINDVALUESPLITS, &direction2);
			result=UpdatePoint( scanPoint, side2,  direction2);
#ifdef ORIENTATIONTEST
			if (result!=0)
				BadState();
#endif
		}
		
		return( result );
 	}
	 	 
	 	 
  	/* Assumptions are made about the original y-vertices, namely,				*/
	/*		that they are descrete: from integers or half grid points. 			*/
	/* 		This ensures that the "denominator" is never unusually small.		*/							
	static tt_int32 EvaluateCurveType(QuadraticBezier *qb)
	{
		tt_int32 result;
		
		double numerator,denominator;
		if (qb->y0==qb->y1) 
		{
			/* first 2 points are equal. */
			if  (qb->y2==qb->y1)
			{
				/* All 3 are equal */
				result=(YDerivZeroAlwaysNoT);	
			}
			else
			{
				/* first 2 points are equal. */
				result=(YDerivZeroAtT0);
			}
		}
		else
		if (qb->y2==qb->y1) 
		{
				/* last two points are equal. */
				result= (YDerivZeroAtT1);
		}
		else
		if (qb->y0==qb->y2)
			{
				/* first and last point on the same scan line. 	*/
				
				result= (YDerivZeroY0EqualY2);				
			}
		else /* NO Control vertices are equal. */
		{
		 	/* See above for equal to endpoints. */
			if (   ( (qb->y0 > qb->y1)  && (qb->y1 > qb->y2))
						||
					( (qb->y0 < qb->y1)  && (qb->y1 < qb->y2))	
				)
				{
				 	result=(YDerivMonotonicAnyT);
				}
			else 
			{
				/* Otherwise, the control point is in, and the 
				YDeriv==0 can be calculated and is useful. */
				result=(YDerivZeroControlOutside);				
			}	
		}
		/* We have completed analysis of he curve segment. */
		return(result);	
	}
		
		
		
		
     /* get the three points which define a curve segment.		*/
     /* This routine performs 2 actions. 						*/
     /*	All points are converted to double precision.			*/
     /*  The first and last endpoints are found, depending		*/
     /*		upon the onCurve values)							*/
     
	 static void SetupCurveSegment(
		 CurveInput *ci,
	 	tt_int32 prevIndex, tt_int32 thisIndex, tt_int32 nextIndex,
	 	tt_int32 arrayType, void *xP, void *yP,
	 	 unsigned char *onCurve)
	 	{     	 
	 		if (arrayType==Orient32BitArray)
	 		{
	 			tt_int32 *xPtr=(tt_int32 *) xP;
	 			tt_int32 *yPtr= (tt_int32 *) yP;
				ci->xPrev= xPtr[prevIndex];
				ci->yPrev= yPtr[prevIndex];
				ci->onCurvePrev= onCurve[prevIndex];
				 
				/* obtain current points. */
				ci->xThis= xPtr[thisIndex];
				ci->yThis= yPtr[thisIndex];
				ci->onCurveThis= onCurve[thisIndex];

				ci->xNext= xPtr[nextIndex];
				ci->yNext= yPtr[nextIndex];
				ci->onCurveNext= onCurve[nextIndex];
			}
			else
				 		{
	 			tt_int16 *xPtr16=(tt_int16 *) xP;
	 			tt_int16 *yPtr16= (tt_int16 *) yP;
				ci->xPrev= xPtr16[prevIndex];
				ci->yPrev= yPtr16[prevIndex];
				ci->onCurvePrev= onCurve[prevIndex];
				 
				/* obtain current points. */
				ci->xThis= xPtr16[thisIndex];
				ci->yThis= yPtr16[thisIndex];
				ci->onCurveThis= onCurve[thisIndex];

				ci->xNext= xPtr16[nextIndex];
				ci->yNext= yPtr16[nextIndex];
				ci->onCurveNext= onCurve[nextIndex];
			}

		}
		
	/* interpolate from x0 to x1, based on the ratio.	*/	
	static double GetInterpolatedAxis(double x0, double x1, double ratio)
	{
		return(x0+ ratio * (x1-x0));
	}			
			
			
/* Check winding for specified segment. Return non-zero if inconsistency found. */
static tt_int32 CheckLineSegment(CurveInput *ci, ScanPoint *scanPoint)
{	
	tt_int32 	yThis =ci->yThis,	yPrev=ci->yPrev, xThis=ci->xThis, xPrev=ci->xPrev;
	double 	yThisD,	yPrevD, xThisD, xPrevD,x,y;
	tt_int32 direction,side, result;
	double theXValue, theYValue,useRatio, numerator, denominator;

	/* convert to double precision. */
	yThisD= (double) yThis;	
	yPrevD= (double) yPrev; 
	xThisD= (double)xThis; 
	xPrevD= (double)xPrev; 	
	x= (double)scanPoint->x; 
	y= (double)scanPoint->y;
	
	/* Make y sure its within range of the actual curve. 			*/
	/* Otherwise, it has no effect on the winding. 		 			*/
	/* Note: the value of Y, being off the grid, will never equal 	*/
	/*		the endpoints or control points. 						*/
	if (
			(  ( y< yThisD) && ( y< yPrevD)  )
			||
			(  ( y>yThisD) && ( y> yPrevD )  )
		)
	{		 	 
		result=0; /* not in range. So its a quick success. */
		goto exit;
	}
	
	
 	/* Find the x-position for specified scan line.		 	*/
	numerator= (y-yPrevD);
	denominator = (yThisD-yPrevD);
	/* the denominator is not super small, because,			*/
	/* 	line segments always have a some yChange. 			*/
	
 		
	useRatio = numerator/denominator;
	theXValue = GetInterpolatedAxis(xPrevD, xThisD, useRatio);
	theYValue = GetInterpolatedAxis(yPrevD, yThisD, useRatio);
 	
	if (denominator>0)
		direction = 1;
	else 
		direction = -1;
	
	side= GetXCurveRelationship(x,theXValue);
	
	result=		UpdatePoint(scanPoint,side, direction);
exit:
 	return ( result );
}



/* Change a quadratic into two quadratics by splitting the curve at t. */
static void SplitQuadratic(
	QuadraticBezier *qb, QuadraticBezier *qb1, QuadraticBezier *qb2, double t)
{
	double splitX, splitY;
	double xMid1, yMid1;
	double xMid2, yMid2;
	
	EvaluateQuadraticAt(qb, t, &splitX, &splitY);

	/* create the first bezier. */
	qb1->x0 = qb->x0;
	qb1->y0 = qb->y0;
	
	qb1->x1 = GetInterpolatedAxis(qb->x0, qb->x1, t);
	qb1->y1 = GetInterpolatedAxis(qb->y0, qb->y1, t);
	
	qb1->x2 =  splitX;
	qb1->y2 =  splitY;
	

	/* create the second bezier. */
	qb2->x0 =  splitX;
	qb2->y0 =  splitY;
	
	qb2->x1 = GetInterpolatedAxis(qb->x1, qb->x2, t);
	qb2->y1 = GetInterpolatedAxis(qb->y1, qb->y2, t);
	
	qb2->x2 = qb->x2;
	qb2->y2 =qb->y2;
}

 		 /* For a given monotonic curve, update the winding. 				*/
		 /* called only when it is known that scanPoint->y is within		*/
		 /* the quadratic y-axis positions: therefore it always performs	*/
		 /* an updated winding, or returns an error. 						*/

		static tt_int32 UpdateMonotonicQuadratic(
			QuadraticBezier *qb, ScanPoint *scanPoint)
		 {
			double  x=scanPoint->x, y=scanPoint->y ;
			tt_int32  direction, side,result;
			  	
			side= CheckLeftOrRight(x, y, qb, MAXFINDVALUESPLITS, &direction);	
			result=UpdatePoint(scanPoint,side, direction);
			return ( result );
		}


		/* Subroutine call for "UpdateQuadraticCurve".						*/
		/* Update a balanced quadratic curve, when we already 	know		*/
		/* 		that the scanPoint scan line intersects the hull (twice)	*/
		static tt_int32 UpdateBalancedQuadraticCurve(
		QuadraticBezier *qb, ScanPoint *scanPoint) 
		{
			QuadraticBezier qb1, qb2; 
			double theX, midY;
			tt_int32 result;
			/* To make it safe, we split it evenly into 2 quadratics  */
			/*   and update them separately.						  */	
			SplitQuadratic(qb, &qb1, &qb2, .5);
			/* make sure the y value is within range. */	
			midY= qb1.y2; 	/* or qb2->y0, they are equal. */
			/* The midpoint extreme should never be the same as 		*/
			/* the scan point, but even if it is, the later code		*/
			/* will udpate it correctly. 								*/
#ifdef ORIENTATIONTEST
			if (midY==scanPoint->y)
					BadState();
#endif
			/* eventhough the scanPoint y-axis value intersects			*/
			/* the convex "hull" of the control points, it does not		*/
			/* necessarily intersect the curve. Detect the extraneous	*/
			/* cases, and exit early.									*/
			if (midY>qb->y0)
			{
				/* qb1 is ascending. */
				if (scanPoint->y>midY) 
				{
					result=0;	/* nothing to do. */
					goto exit;
				}
			}
			else if (midY<qb->y0)
			{
				/* qb1 is descending. 	*/
				if  (scanPoint->y<midY) 
				{
					result=0;	/* nothing to do. */
					goto exit;
				}
			}
		 	/* Otherwise it must intersect both curves. */
			if (
					UpdateMonotonicQuadratic(&qb1, scanPoint)
					||
					UpdateMonotonicQuadratic(&qb2, scanPoint)
				)
					result=1;
			else
				result=0;
		exit:
			return(result);
		}


		/* Subroutine call for "UpdateQuadraticCurve".						*/
		/* Update a quadratic curve, when we already know					*/
		/* 		that the scanPoint scan line intersects the hull.	*/

		static tt_int32 UpdateControlOutsideQuadratic( QuadraticBezier *qb,
				ScanPoint *scanPoint) 
		{
			QuadraticBezier qbBalanced, qbOverhang; 
			tt_int32 do1=0, do2=0;
			double   midY;
			tt_int32 result=0;
			result= SplitQuadraticAtEndPoint(
					qb, 			/* input  quadratic */
					&qbBalanced,	/* balanced  quadratic */
					&qbOverhang,	/* extra overhang quadratic */
					MAXFINDVALUESPLITS);
			if (result==0)
			{
				if (!IsOutsideConvexHull( &qbBalanced, scanPoint->y))
					result=UpdateBalancedQuadraticCurve(&qbBalanced,  scanPoint);
				if (result==0)
				{
					if (!IsOutsideConvexHull( &qbOverhang, scanPoint->y))
						UpdateMonotonicQuadratic(&qbOverhang, scanPoint);
				}			
			}	
			return(result);
 		}

/* We must update the point for a quadratic bezier curve. 			*/
/* This routine decides if the scanPoint winding is affected by		*/
/* the quadratic. If it is, then the scanPoint windings are			*/
/* udpated.															*/
/* Return zero for success, non-zero for error. 					*/
static tt_int32  UpdateQuadraticCurve(CurveInput *ci, ScanPoint *scanPoint)
{	
	double 	yThisD,	yPrevD,xThisD,xPrevD;
	tt_int32 direction,yPrevDiff, xPrevDiff,yNextDiff, xNextDiff;
	double theXValue, theYValue,useRatio,midT;
	double y=scanPoint->y,x=scanPoint->x;
	tt_int32  someYChange,someXChange,zeroTForm;
	QuadraticBezier quadraticBezier, *qb;
	tt_int32 result=0;
	/* determine the x,y changes. */
		yPrevDiff= ci->yThis-ci->yPrev;
		yNextDiff= ci->yThis-ci->yNext;				
		someYChange= (yPrevDiff) || (yNextDiff );
		
		xPrevDiff= ci->xThis-ci->xPrev;
		xNextDiff= ci->xThis-ci->xNext;				
		someXChange= (xPrevDiff) || (xNextDiff );
	
	qb= &quadraticBezier;
	if (someYChange==0)	
	{		 	 
		result=0; /* If no change, then no winding. */
		goto exit;
	}
		
	/* Obtain the bezier curve by direct conversion. */
	XGetQuadraticBezier(
		qb,ci->xPrev, ci->yPrev, ci->onCurvePrev, 
		ci->xThis, ci->yThis,  ci->xNext, ci->yNext, ci->onCurveNext);	
	
	/* Make y sure its within range of the actual curve. 			*/
	/* Otherwise, it has no effect on the winding. 		 			*/
	/* Note: the value of Y, being off the grid, will never equal 	*/
	/*		the endpoints or control points. 						*/
 	if (IsOutsideConvexHull(  qb,   y))
	{		 	 
		result=0; /* not in range. */
		goto exit;
	}
 	
	/* find when the y-derivative changes direction. */
	zeroTForm= EvaluateCurveType(qb);
	switch (zeroTForm)
	{
		case YDerivZeroAlwaysNoT:   
				result=0;		/* handled earlier */
 				break;
 										
		case YDerivZeroY0EqualY2:
				/* We know that qb is balanced (y0==y2) and 		*/
				/* that the scanPoint needs winding update. 		*/
				result=
					UpdateBalancedQuadraticCurve(qb, scanPoint );
				break;

		case YDerivZeroAtT0:	
	 	case YDerivZeroAtT1:	 
		case YDerivMonotonicAnyT: 
			   	result=  UpdateMonotonicQuadratic(qb, scanPoint);
			 	break;
				
				
		case YDerivZeroControlOutside: 
			   	result=  UpdateControlOutsideQuadratic(qb, scanPoint);
			 	break;
			 	
 		default: 
#ifdef ORIENTATIONTEST
					BadState();
#endif
					result=0;
					break;

	}
	
exit:	
	return(result);
}
				 
 	
/*  	ScanCheckContour takes the scanPoint specification, 	*/
/*  		and updates the windings across all	*/					      
/* 		curves and lines within all contours. 	 */
/* 	Note that the process calculates windings to the left 	    */
/* 		and to the right. Upon completion	       	*/
/* 		these must always match, or it was a bad test.	 */
/* 		     						*/
/* 	The ScanCheckContour checks all curves  which have a scan 	*/
/* 		line which matches the scanPoint,      	*/
/* 	with the exception of the curve associated with the scanPoint itself.	*/			
/* 	Assumption: the y-axis value of the scanPoint must		*/ 
/* 		not lie on grid, or half grid.		       	*/
/* 					*/
/* 	return non-zero if there is an error.			*/

static tt_int32 ScanCheckContour( ScanPoint *scanPoint, ScanInputBlock *sib)
{
	tt_int32 startContourIndex, 
			numContourIndices,
				contourIndex=scanPoint->thisContourIndex;
	tt_int32 startPointIndex, endPointIndex, pointIndex;
	tt_int32 usePrevIndex, prevIndex,  nextIndex;
	tt_int32 direction, result=0;
	double x=scanPoint->x, y=scanPoint->y;
	double useRatio, numerator, denominator, denomMult,theXValue;
	CurveInput ci;
 	/* Setup for determining contours to look at. */
	startContourIndex= 0;
	numContourIndices= sib->numberOfContours;

	/* Check all contours. */
	for (contourIndex= startContourIndex; 
			contourIndex<numContourIndices;
				contourIndex++)
	{
		startPointIndex= sib->startPtr[contourIndex];
		endPointIndex= sib->endPtr[contourIndex];
		/* A Contour must have at least 3 points. */
		if ((endPointIndex-startPointIndex + 1)<3)
			continue;
		
		/* for the first point, the previous index is set to the last point,	*/
		/* because all curves wrap-around.										*/
		prevIndex= endPointIndex;
		
		/* Now loop through all points to define curve segments.				*/
		for (pointIndex=startPointIndex; 
				pointIndex<=endPointIndex; 
					pointIndex++)
		{
			usePrevIndex = prevIndex; 		/* Save this value. */
			prevIndex = pointIndex;			/* update this so we can "continue"  */

			/* skip over the segment we are analyzing. */
			if ( 
				(contourIndex==scanPoint->thisContourIndex)
				&&
				(pointIndex==scanPoint->thisPointIndex)
	     	 ) continue;
	     	 
			/* we need next point, too */
			if(pointIndex==endPointIndex)
				nextIndex=startPointIndex;
			else
				nextIndex= pointIndex+1;
    	 
	     	/* get the three points which define a curve segment.*/
			SetupCurveSegment(
				&ci,usePrevIndex, pointIndex, nextIndex,
					sib->arrayType, sib->xArray,  sib->yArray, sib->onCurvePtr);
 			/* See if we can handle it as a segment. */
			if (ci.onCurveThis)
			{
				 if (
				 		(ci.onCurvePrev == 0)
				 		||
				 		(ci.yThis == ci.yPrev)
				 	)
				 	continue; /* Special case, no curve segment at all. */
				 /* Otherwise, use this segment. */
				result= CheckLineSegment( &ci, scanPoint);
				if (result!=0)
					goto exit; /* terminate with bad result. */
			}
			else
			{
				result= UpdateQuadraticCurve( &ci, scanPoint);
				if (result!=0)
					goto exit; /* terminate with bad result. */
			}
		}
	}
exit:
		return(result);
}
	
/* Compute the winding counts to determine black bits. */
 static tt_int32 GetNonZeroWinding(ScanPoint *sp)
 {
 	tt_int32 direction, leftUp, leftDown,rightUp,rightDown,
 			sameXUp, sameXDown;
 	tt_int32 leftSideSum,rightSideSum,sameXSum;
 	direction= sp->direction;
 	leftUp = sp->leftSideUpards;
 	leftDown= sp->leftSideDownwards;
 	rightUp = sp->rightSideUpwards;
 	rightDown= sp->rightSideDownwards;
	sameXUp = sp->sameXUp;
 	sameXDown= sp->sameXDown;
 	sameXSum= sameXUp-sameXDown;
 	
 	leftSideSum= leftUp-leftDown;
 	rightSideSum= rightUp-rightDown;
 	/* make sure that all the directional crossings add to zero. */
 	/*  This must always be true, or the algorithm has failed. 	*/
 	if (( leftSideSum+rightSideSum+direction+sameXSum)!= 0)
 		return(InconsistentWinding);
 	
 	/* It still is a big problem, but its not necessarily 		*/
 	/*		an algorithmic problem.								*/
 	if (sameXUp || sameXDown)
		return(SameXAmbiguity);
		
 	if (leftSideSum!=0)
 	{
 		if ((leftSideSum+ direction)==0)
 			return(direction>0?LeftSideBlack:RightSideBlack);
 		else 
 			return(BothSideBlack);
 	}
 	else
 	{
 		if ((leftSideSum+direction)==0)
 			return(BothSideWhite);
 		else
 			return(direction>0?RightSideBlack:LeftSideBlack);
 	}
 }
 
 				
/*  Make sure that y is nowhere near a grid or half grid, etc. 					*/					 
/* In order to determine the scanning orientation, likely canditate is chosen. 	*/
/* Assumptions:																	*/
/*		The closed curve is non-intersecting- e.g. no mobius strips.			*/			  	
/* Result Codes:																*/	
 
/* Assumptions:																	*/
/*		The closed curve is non-intersecting- e.g. no mobius strips.			*/

/* Note: if the contour check reveals that the expectedDirection is confirmed,	*/
/*			then the routine immediately returns.								*/
/* if the check reveals an incorrect direction, then a second attempt is made.	*/
/*		if the second attempt also reveal an incorrect direction,				*/
/*		then the direction is reversed. 										*/
/*	therefore, the obvious bias is to return the direction unchanged.			*/			  	

 tt_int32 FindNonZeroWindingCounts( 
 		 ScanInputBlock *sib, 	 
 		 tt_int32 contourIndex,			/* contour to be checked. 				*/
 		 tt_int32 compTest			/* debug comparison testing.					*/
   		 )
	{
		tt_int32 startPointIndex, endPointIndex,prevIndex,usePrevIndex,nextIndex;
		tt_int32 windings,thisPointIndex,result ,windingType;
		ScanPoint  			scanPoint;
 		CurveInput ci;
 		tt_int32 gotOneReversal=0;
 		
		startPointIndex= sib->startPtr[contourIndex];
		endPointIndex= sib->endPtr[contourIndex];
		/* A Contour must have at least 3 points. */
		if ((endPointIndex-startPointIndex + 1)<3)
			goto exit;		/* Could not be checked. */
	
	
		/* otherwise find a nice place to count the winding values. */
  		prevIndex=endPointIndex;		
		for (thisPointIndex=startPointIndex; 
			thisPointIndex<= endPointIndex; 
				thisPointIndex++)
 		{
			usePrevIndex = prevIndex; 	/* Save this value. */
			prevIndex = thisPointIndex;	/* update this so we can "continue" */

			/* we need next point too.		*/
			if(thisPointIndex==endPointIndex)
				nextIndex=startPointIndex;
			else
				nextIndex= thisPointIndex+1;
			
	     	/* get the three points which define a curve segment.*/
	     	InitScanPointStructure(&scanPoint,  contourIndex,  thisPointIndex);
			SetupCurveSegment(&ci,usePrevIndex, 
				thisPointIndex, nextIndex,
				sib->arrayType,
				sib->xArray, sib->yArray, sib->onCurvePtr);
 			/* See if we can handle it as a line segment. */
			if (ci.onCurveThis)
			{
				 if (
				 		(ci.onCurvePrev == 0)
				 		||
				 		(ci.yThis == ci.yPrev)
				 	)
				 	continue; /* Special case, no curve segment at all. */
				 /* Otherwise, use setup this segment- no errors possible. */
				GetNonGridXPointOnLineSegment( 
					ci.xPrev, ci.yPrev, ci.xThis, ci.yThis, &scanPoint);
				result= ScanCheckContour(&scanPoint,  sib);
 			}
			else
			{
 				result= GetNonGridXPointOnQuadratic(&ci, &scanPoint);
 				if (result==0)
 					result= ScanCheckContour(&scanPoint,  sib);
 				else
 					continue; /* else, skip this curve for checking. */			
 			}	 
 			
 			/* If consistent, then determine windings. */
			if (result==0)
			{
				/* we have a result for windings. */
				windingType= GetNonZeroWinding( &scanPoint);
				switch (windingType)
				{
					case LeftSideBlack:
						if (gotOneReversal)
						 	goto reverseDirection;
						else
						 {
						 	gotOneReversal++;
						 	continue; /* try to confirm it. */
						 }
		   
 					case RightSideBlack:	/* Normal case, quick exit. :*/
 							goto exit;
 							
					case InconsistentWinding: 
											   
#ifdef ORIENTATIONTEST
							if (compTest==0)
								BadState( );
#endif
							goto exit;
					case BothSideBlack:
					case BothSideWhite:
#ifdef ORIENTATIONTEST
							if (compTest==0)
								BadState( );
#endif
							goto exit;
							
					case SameXAmbiguity:
#ifdef ORIENTATIONTEST
							if (compTest==0)
								RarelyHappens( );
#endif
							continue;
 							
 					default: /* fall thru. */ ;
#ifdef ORIENTATIONTEST
 								BadState( );
							goto exit;
#endif
 						
				}							 
				goto exit;
			}
			else
			{
				/* No result is very rare, and implies a strange  font
					outline (self intersections, etc). 
					Therefore, handle it as a regular case- do
					not change the winding. 
				*/
#ifdef ORIENTATIONTEST
				BadState( );
#endif
				goto exit;
			}

		}
exit:
   		return(0);
  reverseDirection:
#ifdef ORIENTATIONTEST
	GotReversal();
#endif

   	return(1);
  	
}



































