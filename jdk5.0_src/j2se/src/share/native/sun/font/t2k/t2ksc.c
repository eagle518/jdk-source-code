/*
 * @(#)t2ksc.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2KSC.C
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
#include "t2ksc.h"
#include "util.h"


#ifdef OLDSORTTEST
static int COMPARE_GT( register tsiScanConv *t, tt_int32 i, tt_int32 j ) {
	/* an x edge is greater than a y edge */
	if ( t->edgeData[i] & IS_XEDGE ) {
		/* x edge */
		if ( t->edgeData[j] & IS_YEDGE ) return true; /*****/
		if ( t->xEdge[i] > t->xEdge[j] ) return true; /*****/
		if ( t->xEdge[i] < t->xEdge[j] ) return false; /*****/
		return ( t->yEdge[i] > t->yEdge[j] ); /*****/
	} else {
		/* y edge */
		if ( t->edgeData[j] & IS_XEDGE ) return false; /*****/
		if ( t->yEdge[i] > t->yEdge[j] ) return true; /*****/
		if ( t->yEdge[i] < t->yEdge[j] ) return false; /*****/
		return ( t->xEdge[i] > t->xEdge[j] ); /*****/
	}
}
#endif

static void ShellSort( register tsiScanConv *t )
{
	register tt_int32 step, h, N, i;
	register tt_int32 *xEdge = t->xEdge;
	register tt_int32 *yEdge = t->yEdge;
	register char *edgeData = t->edgeData;

	N = t->numEdges;

	/* Find initial h */
	for ( h = 1; h <= N/9; h = 3*h+1 )
		;
	/* Loop through decreasing h. In the last pass h is 1 */
	for ( ; h > 0; h /= 3 ) {
		for ( step = h; step < N; step++ ) {
			register char ei;
			register tt_int32 xi,yi;
			char bTmp = edgeData[step];
			tt_int32 xTmp = xEdge[step];
			tt_int32 yTmp = yEdge[step];
		
			if ( bTmp & IS_XEDGE ) {
				for ( i = step - h; i >= 0; i -= h ) {
					/* if ( COMPARE_GT( tmp, i ) break; */
					ei = edgeData[i];
					xi = xEdge[i];
					yi = yEdge[i];
					if ( ( ei & IS_YEDGE ) || ( xTmp > xi ) || (( xTmp == xi ) && yTmp > yi ) ) break; /*****/
					xEdge[i + h]	= xi;
					yEdge[i + h] 	= yi;
					edgeData[i + h]	= ei;
				}
			} else {
				for ( i = step - h; i >= 0; i -= h ) {
					/* if ( COMPARE_GT( tmp, i ) break; */
					ei = edgeData[i];
					xi = xEdge[i];
					yi = yEdge[i];
					if ( (ei & IS_YEDGE) && (( yTmp > yi ) || (( yTmp == yi ) && xTmp > xi) ) ) break; /*****/
					xEdge[i + h]	= xi;
					yEdge[i + h]	= yi;
					edgeData[i + h]	= ei;
				}
			}
			xEdge[i + h]	= xTmp;
			yEdge[i + h]	= yTmp;
			edgeData[i + h]	= bTmp;
		}
	}

#ifdef OLDSORTTEST
	for ( i = 0; i < N-1; i++ ) {
		if ( COMPARE_GT( t, i, i+1 ) ) {
			printf("Bogus Sort\n");
			assert( false );
		}
	}
#endif
}

#ifdef USE_NON_ZERO_WINDING_RULE

static void DoNonZeroWindingRule( register tsiScanConv *t )
{
	tt_int32 i, j, limit = t->numEdges - 1; /* [0,1,2.....,limit] */
	register int windingCount;
	register char *edgeData = t->edgeData;
	tt_int32 *xEdge, *yEdge;
	int deleteUsed = false;
	
	windingCount = 0;
	/* Do the "y-edges */
	for ( i = 0; i < limit; i++  ) {	/* Stop one short of the last index since we may set i+1 */
		if ( edgeData[i] & IS_XEDGE ) {
			assert( windingCount == 0 );
			break; /*****/
		}
		/* This is a "y-edge". Draw from x[i] to x[i+1] */
		if ( edgeData[i] & POS_YEDGE ) {
			windingCount++;
		} else {
			windingCount--;
		}
		if ( windingCount > 1 || windingCount < -1 ) {
			edgeData[i]   |= DELETE_EDGE;
			edgeData[i+1] |= DELETE_EDGE;
			deleteUsed	   = true;
		}
	}
	windingCount = 0;	
	/* Do the "x-edges */
	for ( /* i = i */; i < limit; i++  ) {	/* Stop one short of the last index since we may set i+1 */
		/* This is an "x-edge". Draw from y[i] to y[i+1] */
		if ( edgeData[i] & POS_XEDGE ) {
			windingCount++;
		} else {
			windingCount--;
		}
		if ( windingCount > 1 || windingCount < -1 ) {
			edgeData[i]   |= DELETE_EDGE;
			edgeData[i+1] |= DELETE_EDGE;
			deleteUsed	   = true;
		}
	}
	if ( deleteUsed ) {
		xEdge = t->xEdge;
		yEdge = t->yEdge;
		for ( j = i = 0; i <= limit; i++  ) {
			if ( !(edgeData[i] & DELETE_EDGE) ) {
				if ( i != j ) {
					xEdge[j]	= xEdge[i];
					yEdge[j]	= yEdge[i];
					edgeData[j]	= edgeData[i];
				}
				j++;
			}
		}
		#ifdef OLD
			if ( t->numEdges != j ) {
				printf("!!!!!!!!! Reducing edgeCount from %d to %d\n", (int)t->numEdges , (int)j );
			}
		#endif
		t->numEdges = j;
	}
}
#endif /* USE_NON_ZERO_WINDING_RULE */


static void ReAllocEdges( register tsiScanConv *t )
{
	register tt_int32 i, k = t->maxEdges + (t->maxEdges>>1);
	register tt_int32 *xEdge;
	register tt_int32 *yEdge;
	register char *edgeData;
	
	
	xEdge 		= (tt_int32*) tsi_AllocMem( t->mem, k * (sizeof( char ) + sizeof( tt_int32 )*2 ) );
	assert( xEdge != NULL );
	yEdge   	= &xEdge[k];
	edgeData 	= (char *)&yEdge[k];
	
	for ( i = 0; i < t->numEdges; i++ ) {
		xEdge[i]	= t->xEdge[i];
		yEdge[i]	= t->yEdge[i];
		edgeData[i]	= t->edgeData[i];
	}
	tsi_DeAllocMem( t->mem, (char *)t->xEdge );
	
	t->xEdge 	= xEdge;
	t->yEdge 	= yEdge;
	t->edgeData	= edgeData;
	t->maxEdges = k;
}




static void drawLine( register tsiScanConv *t, tt_int32 x0, tt_int32 y0, tt_int32 x2, tt_int32 y2 )
{
	tt_int32 xA, xB, yA, yB, center, dist;
#ifdef USE_NON_ZERO_WINDING_RULE
	char edgeType;
#endif
	
	/* Ensure we go on odd coordinates to avoid singularities
	   with multiple points at pixelcenters */
	x0 |= 1;
	y0 |= 1;
	x2 |= 1;
	y2 |= 1;
	
	/* Do X Edges - enforce XA < XB */
	if ( x0 < x2 ) {
		xA = (tt_int32)x0; yA = (tt_int32)y0;
		xB = (tt_int32)x2; yB = (tt_int32)y2;
		#ifdef USE_NON_ZERO_WINDING_RULE
			edgeType = POS_XEDGE;
		#endif
	} else {
		xA = (tt_int32)x2; yA = (tt_int32)y2;
		xB = (tt_int32)x0; yB = (tt_int32)y0;
		#ifdef USE_NON_ZERO_WINDING_RULE
			edgeType = NEG_XEDGE;
		#endif
	}

	/* if xA is in rightside of pixel, then round the center 
	   to the middle of the next pixel. */
	center = xA & ~63; center += 32;
	if ( center < xA ) center += 64;
	/* from A to B, a positive value, or zero */
	dist = xB - xA;
	if ( dist != 0 ) {
		tt_int32 y, yStep;
		
		if ( dist >= 32768 || yB >= 32768 || yB <= -32768 || yA >= 32768 || yA <= -32768 ) {
			while ( center <= xB ) {
				if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
				t->xEdge[t->numEdges] = center;
				t->yEdge[t->numEdges] = yA + util_FixMul( yB - yA, util_FixDiv( center - xA, dist ) );
				#ifdef USE_NON_ZERO_WINDING_RULE
					t->edgeData[t->numEdges] = edgeType;
				#else
					t->edgeData[t->numEdges] = POS_XEDGE;
				#endif
				t->numEdges++;
				center += 64;
			}
		} else {
#ifdef OLD
			for ( shift = 0; dist > 32768;  ) {	/* Avoid overflow problems */
				dist >>= 1;
				shift++;
			}
			y = yB * ((center - xA) >> shift) + yA * (( xB  - center ) >> shift);
			yStep = (64 * (yB - yA)) >> shift;
#endif
			y = yB * (center - xA) + yA * (xB - center);
			yStep = (yB - yA) << 6;
			while ( center <= xB ) {
				if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
				t->xEdge[t->numEdges] = center;
				t->yEdge[t->numEdges] = y / dist;
				y += yStep;
				#ifdef USE_NON_ZERO_WINDING_RULE
					t->edgeData[t->numEdges] = edgeType;
				#else
					t->edgeData[t->numEdges] = POS_XEDGE;
				#endif
				t->numEdges++;
				center += 64;
			}
		}
	} else if ( center <= xB ) { /* Distance equals 0 */
		if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
		t->xEdge[t->numEdges] = center;
		t->yEdge[t->numEdges] = yA;
		#ifdef USE_NON_ZERO_WINDING_RULE
			t->edgeData[t->numEdges] = edgeType;
		#else
			t->edgeData[t->numEdges] = POS_XEDGE;
		#endif
		t->numEdges++;
	}
	/* else xB>center,  throw the edge away ? */
	/* Do Y Edges */
	if ( y0 < y2 ) {
		yA = (tt_int32)y0; xA = (tt_int32)x0;
		yB = (tt_int32)y2; xB = (tt_int32)x2;
		#ifdef USE_NON_ZERO_WINDING_RULE
			edgeType = POS_YEDGE;
		#endif
	} else {
		yA = (tt_int32)y2; xA = (tt_int32)x2;
		yB = (tt_int32)y0; xB = (tt_int32)x0;
		#ifdef USE_NON_ZERO_WINDING_RULE
			edgeType = NEG_YEDGE;
		#endif
	}
	/* from A to B */
	center = yA & ~63; center += 32;
	if ( center < yA ) center += 64;
	dist = yB - yA;
	if ( dist != 0 ) {
		tt_int32 x, xStep;

		if ( dist >= 32768 || xB >= 32768 || xB <= -32768 || xA >= 32768 || xA <= -32768 ) {
			/* Numbers too big for simple integer math */
			while ( center <= yB ) {
				if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
				t->xEdge[t->numEdges] = xA + util_FixMul( xB - xA, util_FixDiv( center - yA, dist ) );
				t->yEdge[t->numEdges] = center;
				#ifdef USE_NON_ZERO_WINDING_RULE
					t->edgeData[t->numEdges] = edgeType;
				#else
					t->edgeData[t->numEdges] = POS_YEDGE;
				#endif
				t->numEdges++;
				center += 64;
			}
		} else {
#ifdef OLD
			for ( shift = 0; dist >= 32768;  ) { /* Avoid overflow problems */
				dist >>= 1;
				shift++;
			}
			x = xB * ((center - yA) >> shift) + xA * (( yB  - center ) >> shift);
			xStep = (64 * (xB - xA)) >> shift;
#endif
			x = xB * (center - yA) + xA * (yB  - center);
			xStep = (xB - xA) << 6;
			while ( center <= yB ) {
				if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
				t->xEdge[t->numEdges] = x / dist;
				x += xStep;
				t->yEdge[t->numEdges] = center;
				#ifdef USE_NON_ZERO_WINDING_RULE
					t->edgeData[t->numEdges] = edgeType;
				#else
					t->edgeData[t->numEdges] = POS_YEDGE;
				#endif
				t->numEdges++;
				center += 64;
			}
		}
	} else if ( center <= yB ) {
		if ( t->numEdges >= t->maxEdges ) ReAllocEdges(t);
		t->xEdge[t->numEdges] = xA;
		t->yEdge[t->numEdges] = center;
		#ifdef USE_NON_ZERO_WINDING_RULE
			t->edgeData[t->numEdges] = edgeType;
		#else
			t->edgeData[t->numEdges] = POS_YEDGE;
		#endif
		t->numEdges++;
	}
}



static void drawParabola( register tsiScanConv *t, tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 x2, tt_int32 y2 )
{
	tt_int32 dX, dY, error;
	tt_int32 count;
	
	tt_int32 midX, midY;
	tt_int32 min, max;
	tt_int32 Arr[16*7], *wp;

	wp = &Arr[0];
	assert( t->xEdge != NULL );

	dX		= ((x0 - x1 - x1 + x2 + 2) >> 2); if ( dX < 0 ) dX = -dX;
	dY		= ((y0 - y1 - y1 + y2 + 2) >> 2); if ( dY < 0 ) dY = -dY;
	error	= (dX > dY ? dX : dY);
	for ( count = 0; error > 1; count++ ) {
		error >>= 2; /* We could solve this directly instead... */
	}
	
	for (;;) {
		/* See if we do an x straddle */
		if ( x0 < x1 ) {
			min = x0; max = x1;
		} else {
			min = x1; max = x0;
		}
		if ( x2 < min ) {
			min = x2;
		} else if ( x2 > max ) {
			max = x2;
		}
		/* Set min to the next half grid >= min */
		min += 31; min &= ~63; min += 32;
		
		if ( max < min) {
			/* See if we do a y straddle */
			if ( y0 < y1 ) {
				min = y0; max = y1;
			} else {
				min = y1; max = y0;
			}
			if ( y2 < min ) {
				min = y2;
			} else if ( y2 > max ) {
				max = y2;
			}
			/* Set min to the next half grid >= min */
			min += 31; min &= ~63; min += 32;
		}
		
		if ( max >= min ) {
			if ( count > 0 ) {
				midX = ((x0 + x1 + x1 + x2 + 2) >> 2);
				midY = ((y0 + y1 + y1 + y2 + 2) >> 2);
				count--;
				/* InnerDrawParabola( t, x0, y0, ((x0+x1+1)>>1), ((y0+y1+1)>>1), midX, midY, count); */
				
				*wp++ = midX;
				*wp++ = midY;
				*wp++ = ((x1+x2+1)>>1);
				*wp++ = ((y1+y2+1)>>1);
				*wp++ = x2;
				*wp++ = y2;
				*wp++ = count;
				/* InnerDrawParabola( t, midX, midY, ((x1+x2+1)>>1), ((y1+y2+1)>>1), x2, y2, count); */
				x1 = ((x0+x1+1)>>1);
				y1 = ((y0+y1+1)>>1);
				x2 = midX;
				y2 = midY;
				continue; /*****/
			} else {
				drawLine( t, x0, y0, x2, y2 );
			}
		}
		if ( wp > Arr ) {
			count	= *(--wp);
			y2 		= *(--wp);
			x2		= *(--wp);
			y1		= *(--wp);
			x1		= *(--wp);
			y0		= *(--wp);
			x0		= *(--wp);
			
			continue; /*****/
		}
		break; /*****/
	}
	
}

#ifdef T1_OR_T2_IS_ENABLED

static void draw3rdDegreeBezier( register tsiScanConv *t, tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 x2, tt_int32 y2, tt_int32 x3, tt_int32 y3 )
{
	tt_int32 dX, dY, error;
	tt_int32 count;
	tt_int32 midX, midY;
	tt_int32 min, max;
	tt_int32 Arr[16*9], *wp;

	wp = &Arr[0];
	assert( t->xEdge != NULL );
	/* dZ = (z0+3*z1+3*z2+z3)/8 - (4*(z0+z3)/8), => */
	dX		= (( 3*(x1+x2-x0-x3) + 4) >> 3); if ( dX < 0 ) dX = -dX;
	dY		= (( 3*(y1+y2-y0-y3) + 4) >> 3); if ( dY < 0 ) dY = -dY;
	error	= (dX > dY ? dX : dY);
	for ( count = 0; error > 1; count++ ) {
		error >>= 2; /* We could solve this directly instead... */
	}
	
	
	for (;;) {
		/* See if we do an x straddle */
		if ( x0 < x1 ) {
			min = x0; max = x1;
		} else {
			min = x1; max = x0;
		}
		if ( x2 < min ) {
			min = x2;
		} else if ( x2 > max ) {
			max = x2;
		}
		if ( x3 < min ) {
			min = x3;
		} else if ( x3 > max ) {
			max = x3;
		}
		/* Set min to the next half grid >= min */
		min += 31; min &= ~63; min += 32;
		
		/* bug if ( min < max ) */
		if ( max < min) {
			/* See if we do a y straddle */
			if ( y0 < y1 ) {
				min = y0; max = y1;
			} else {
				min = y1; max = y0;
			}
			if ( y2 < min ) {
				min = y2;
			} else if ( y2 > max ) {
				max = y2;
			}
			if ( y3 < min ) {
				min = y3;
			} else if ( y3 > max ) {
				max = y3;
			}
			
			/* Set min to the next half grid >= min */
			min += 31; min &= ~63; min += 32;
		}
		
		if ( max >= min ) {
			if ( count > 0 ) {
				tt_int32 xB, yB, xC, yC;
				
				midX = ((x0 + 3*(x1+x2) + x3 + 4) >> 3);
				midY = ((y0 + 3*(y1+y2) + y3 + 4) >> 3);
				count--;
				xC = (x2+x3+1)>>1;
				yC = (y2+y3+1)>>1;
				xB = (xC + ((x1+x2+1)>>1) + 1 ) >> 1;
				yB = (yC + ((y1+y2+1)>>1) + 1 ) >> 1;
				/* mid, B, C, 3  */
				
				*wp++ = midX;
				*wp++ = midY;
				*wp++ = xB;
				*wp++ = yB;
				*wp++ = xC;
				*wp++ = yC;
				*wp++ = x3;
				*wp++ = y3;
				*wp++ = count;

				/* 0, B, C, mid */
				xB = (x0+x1+1)>>1;
				yB = (y0+y1+1)>>1;
				xC = (xB + ((x1+x2+1)>>1) + 1 ) >> 1;
				yC = (yB + ((y1+y2+1)>>1) + 1 ) >> 1;
				
				
				x1 = xB;
				y1 = yB;
				x2 = xC;
				y2 = yC;
				x3 = midX;
				y3 = midY;
				continue; /*****/
			} else {
				drawLine( t, x0, y0, x3, y3 );
			}
		}
		if ( wp > Arr ) {
			count	= *(--wp);
			y3 		= *(--wp);
			x3		= *(--wp);
			y2 		= *(--wp);
			x2		= *(--wp);
			y1		= *(--wp);
			x1		= *(--wp);
			y0		= *(--wp);
			x0		= *(--wp);
			
			continue; /*****/
		}
		break; /*****/
	}
	
}

#endif /* T1_OR_T2_IS_ENABLED */

#ifdef T1_OR_T2_IS_ENABLED
static void Make3rdDegreeBezierEdgeList( register tsiScanConv *t, char greyScaleLevel )
{
    short startPoint, lastPoint, ctr, gMul;
    short ptA, ptB;
	tt_int32 Ax, Bx, Ay, By;
	register tt_int32 *x = t->x;
	register tt_int32 *y = t->y;

	tt_int32 pointCount;
	
	assert( t->xEdge != NULL );
	gMul = greyScaleLevel;
	if ( gMul < 1 ) gMul = 1;
	t->gMul = gMul;


	for ( ctr = 0; ctr < t->numberOfContours; ctr++ ) {
		ptA = startPoint = t->startPoint[ctr];
		lastPoint = t->endPoint[ctr];
		pointCount = lastPoint - startPoint + 1;

		while ( !t->onCurve[ ptA ] ) {
			ptA = (short)(ptA + 1);
			assert ( ptA <= lastPoint );
		}
		assert( t->onCurve[ ptA ] );
		Ax  = x[ ptA ];	Ay  = y[ ptA ];
		Ax *= gMul;		Ay *= gMul;
		
		while ( pointCount > 0) {
			ptB = (short)(ptA + 1);
			if ( ptB > lastPoint ) ptB = startPoint;
			Bx  = x[ ptB ];	By  = y[ ptB ];
			Bx *= gMul;		By *= gMul;
			if ( t->onCurve[ ptB ] ) {
				/* A straight line. */
				drawLine( t, Ax, Ay, Bx, By );
				ptA = ptB; Ax = Bx; Ay = By;
				pointCount--;
			} else {
				tt_int32 Cx, Dx, Cy, Dy;
				short ptC, ptD;
				/* A 3rd degree bezier. */
				ptC = (short)(ptB + 1);
				if ( ptC > lastPoint ) ptC = startPoint;
				ptD = (short)(ptC + 1);
				if ( ptD > lastPoint ) ptD = startPoint;
				
				assert( !t->onCurve[ ptC ] );
				assert( t->onCurve[ ptD ] );
				Cx  = x[ ptC ];	Cy  = y[ ptC ];
				Cx *= gMul;		Cy *= gMul;
				Dx  = x[ ptD ];	Dy  = y[ ptD ];
				Dx *= gMul;		Dy *= gMul;
				draw3rdDegreeBezier( t, Ax, Ay, Bx, By, Cx, Cy, Dx, Dy );
				ptA = ptD; Ax = Dx; Ay = Dy;
				pointCount -= 3;
			}
		}
	}
	ShellSort(t);
#ifdef USE_NON_ZERO_WINDING_RULE
	DoNonZeroWindingRule( t );
#endif
	if ( gMul > 1 ) {
		tt_int32 i, N = t->numEdges; /* Set N equal to numEdges */
		tt_int32 *xEdge = t->xEdge;
		tt_int32 *yEdge = t->yEdge;
		
		
		for ( i = 0; i < N; i++ ) {
			xEdge[i] /= gMul;
			yEdge[i] /= gMul;
		}
	}
}
#endif /* T1_OR_T2_IS_ENABLED */


static void MakeEdgeList( register tsiScanConv *t, char greyScaleLevel )
{
    short startPoint, lastPoint, ctr, gMul;
    short ptA, ptB, ptC;
	tt_int32 Ax, Bx, Cx, Ay, By, Cy;
	register tt_int32 *x = t->x;
	register tt_int32 *y = t->y;
	
	assert( t->xEdge != NULL );
	gMul = greyScaleLevel;
	if ( gMul < 1 ) gMul = 1;
	t->gMul = gMul;
	ctr = 0;
SC_startContour:

	;
#ifdef OLD
	for ( ;ctr < t->numberOfContours; ctr++ )
	the "ctr++" is unreachable !
#endif
	while ( ctr < t->numberOfContours ) {
		ptA = startPoint = t->startPoint[ctr];
		lastPoint = t->endPoint[ctr];
		if ( t->onCurve[ ptA ] ) {
			Ax  = x[ ptA ];	Ay  = y[ ptA ];
			Ax *= gMul; 	Ay *= gMul;	
			ptB = -1; Bx = By = 0;
		} else {
			Bx  = x[ ptA ]; By  = y[ ptA ];
			Bx *= gMul;		By *= gMul;	
			ptB = lastPoint;
			if ( t->onCurve[ ptB ] ) {
				Ax  = x[ ptB ]; Ay = y[ ptB ];
				Ax *= gMul; 	Ay *= gMul;	
			} else {
				Ax = ((x[ ptB ]*gMul + Bx + 1) >> 1);
				Ay = ((y[ ptB ]*gMul + By + 1) >> 1);
			}
			ptC = ptB; ptB = ptA; ptA = ptC; /* SWAP A, B */
		}

		for (;;) {
/* SC_AOnBOff: */
			while ( ptB >= 0 ) {	
				ptC = (short)(ptB + 1); /* ptC = NEXTPT( ptB, ctr ); */
				if ( ptC > lastPoint ) ptC = startPoint;
				Cx = x[ ptC ];	Cy = y[ ptC ];
				Cx *= gMul;		Cy *= gMul;	
				if ( ! t->onCurve[ ptC ]) {
					drawParabola( t, Ax, Ay, Bx, By, ((Bx + Cx + 1) >> 1), ((By + Cy + 1) >> 1) );
					if ( ptC == startPoint ) {ctr++; goto SC_startContour;}	/********************** continue SC_startContour */
					Ax = ((Bx + Cx + 1) >> 1); Ay = ((By + Cy + 1) >> 1);
					Bx = Cx; By = Cy; ptA = ptB; ptB = ptC;
					continue; /********************** continue SC_AOnBOff */
				}
				drawParabola( t, Ax, Ay, Bx, By, Cx, Cy );
				if ( ptC == startPoint ) {ctr++; goto SC_startContour;}	/********************** continue SC_startContour */
				ptA = ptC; Ax = Cx; Ay = Cy;
				break; /***********************/
			}
/* SC_AOn:	*/		
			for (;;) {
				ptB = (short)(ptA + 1); /* ptB = NEXTPT( ptA, ctr ); */
				if ( ptB > lastPoint ) ptB = startPoint;
				Bx = x[ ptB ];	By = y[ ptB ];
				Bx *= gMul;		By *= gMul;	
				if ( t->onCurve[ ptB ] ) {
					drawLine( t, Ax, Ay, Bx, By );
					if ( ptB == startPoint ) {ctr++; goto SC_startContour;}		/********************** continue SC_startContour */
					ptA = ptB; Ax = Bx; Ay = By;
					continue;  /********************** continue SC_AOn */
				}
				if ( ptB == startPoint ) {ctr++; goto SC_startContour;}			/**********************continue SC_startContour */
				break; /***********************/
			}
		}
	}
	ShellSort(t);
#ifdef USE_NON_ZERO_WINDING_RULE
	DoNonZeroWindingRule( t );
#endif
	if ( gMul > 1 ) {
		register tt_int32 i, N = t->numEdges; /* Set N equal to numEdges */
		register tt_int32 *xEdge = t->xEdge;
		register tt_int32 *yEdge = t->yEdge;
		
		
		for ( i = 0; i < N; i++ ) {
			xEdge[i] /= gMul;
			yEdge[i] /= gMul;
		}
	}
} /* end MakeEdgeList() */


tsiScanConv *tsi_NewScanConv( tsiMemObject *mem, short numberOfContours, short *startPtr, short *endPtr,
				              tt_int32 *xPtr, tt_int32 *yPtr, char *onCurvePtr, char greyScaleLevel, char curveType )
{
	register tsiScanConv *t = (tsiScanConv *)tsi_AllocMem( mem, sizeof( tsiScanConv ) );
	t->mem = mem;


	t->numberOfContours	= numberOfContours;
	t->startPoint		= startPtr;
	t->endPoint			= endPtr;
	
	t->x				= xPtr;
	t->y				= yPtr;
	
	t->onCurve			= onCurvePtr;
	
	t->maxEdges = 112*4; /* 455 * 9 is about 4 Kbytes */
	/* Allocate the memory */
	t->xEdge 	= (tt_int32*) tsi_AllocMem( mem, t->maxEdges * (sizeof( char ) + sizeof( tt_int32 )*2 ) );
	assert( t->xEdge != NULL );
	t->yEdge	= &t->xEdge[t->maxEdges];
	t->edgeData	= (char *)&t->yEdge[t->maxEdges];
	
	t->baseAddr 	= NULL;
	t->numEdges	 	= 0;

	if ( curveType == 3 ) {
		;
#ifdef T1_OR_T2_IS_ENABLED
		Make3rdDegreeBezierEdgeList( t, greyScaleLevel );
#endif
	} else {
		MakeEdgeList( t, greyScaleLevel );
	}
	return t; /*****/
}





void MakeBits( register tsiScanConv *t, char greyScaleLevel, char xWeightIsOne, 
	       char omitBitMap, char computeBBox, short isDropout )
{
	register tt_int32 i , N;
	tt_int32 xmin, xmax;
	tt_int32 ymin, ymax;
	tt_int32 w, h, firstXEdge, rowBytes;
	register tt_int32 *xEdge = t->xEdge;
	register tt_int32 *yEdge = t->yEdge;
	register unsigned char *baseAddr;
	tt_int32 xmid, ymid;

 	if ( computeBBox ) {
		xmin	= xEdge[0];
		ymin	= yEdge[0];
		xmax 	= xmin;
		ymax 	= ymin;
		
		N = t->numEdges;
		for ( i = 1; i < N; i++ ) {
			tt_int32 x, y;
			
			x = xEdge[i];
			y = yEdge[i];
			if (  x < xmin ) {
				xmin = x;
			} else if ( x > xmax ) {
				xmax = x;
			}
			if ( y < ymin ) {
				ymin = y;
			} else if ( y > ymax ) {
				ymax = y;
			}
		}
	} else {
		xmin = t->xmin;
		xmax = t->xmax;
		ymin = t->ymin;
		ymax = t->ymax;
	}
	xmid = (xmin+xmax) >> 1;
	ymid = (ymin+ymax) >> 1;
/*
	printf("t->numEdges = %d\n", t->numEdges );
	printf("t->maxEdges = %d\n", t->maxEdges );
	assert( xmin >= -32000 && xmin <= 32000 );
	assert( xmax >= -32000 && xmax <= 32000 );
	assert( ymin >= -32000 && ymin <= 32000 );
	assert( ymax >= -32000 && ymax <= 32000 );
*/
	
	/* Min is inclusive the max is not */
	t->fLeft26Dot6 = xmin;
	t->left		= xmin = ( xmin + 0  ) >> 6;
	t->right	= xmax = ( xmax + 64 + 0 ) >> 6;
	t->fTop26Dot6 = ymax + 64;
	t->top		= ymin = ( ymin + 0  ) >> 6;
	t->bottom	= ymax = ( ymax + 64 + 0 ) >> 6;
	
	/* System.out.println("ymin =  " + ymin + " ymax = " + ymax ); */
	/* printf("ymin = %d, ymax = %d\n", ymin, ymax); */

	w = xmax - xmin;
	h = ymax - ymin;
	
	if ( greyScaleLevel == 0 ) {
		rowBytes = (w+7) / 8;
	} else {
		rowBytes = w; 
	}
	t->rowBytes = rowBytes;
	t->baseAddr = NULL;
	if ( omitBitMap ) return; /*****/
	N = rowBytes * h;
	t->baseAddr = baseAddr = (unsigned char*) tsi_AllocMem( t->mem, N * sizeof( unsigned char ) );

	for ( i = 0; i < N; i++ ) {
		baseAddr[i] = 0;
	}
	firstXEdge = 0;
	
	if ( greyScaleLevel > 0 ) {
		tt_int32 fac, h1;
		
		
		h1 = h - 1;
		fac = 120 / (t->gMul* 2 );
		for ( i = 0; i < t->numEdges;  ) {
			tt_int32 x1, x2, y, k;
			if ( t->edgeData[i] & IS_XEDGE ) {
				firstXEdge = i;
				break; /*****/
			}
			
			/* draw from i to j */
			y  = yEdge[i];
			x1 = xEdge[i];
			x2 = xEdge[i+1];
			
			y  >>= 6;
			x1 >>= 6;
			x2 >>= 6;
			x1 -= xmin;
			x2 -= xmin;
			
			/* k = ( ymax - (y - ymin) - 1 ) * rowBytes; */
			/* k = ( h - y - 1) * rowBytes; */
			k = ( h1 - (y - ymin) ) * rowBytes;

			x1 += k;
			x2 += k;
			if ( x1 == x2 ) {
				baseAddr[x1] = (unsigned char)(baseAddr[x1] + ((xEdge[i+1] & 63)
									       - (xEdge[i] & 63)) * fac / 64);
			} else {
				baseAddr[x1] = (unsigned char)(baseAddr[x1] + (64 - (xEdge[i] & 63)) * fac / 64);
				baseAddr[x2] = (unsigned char)(baseAddr[x2] + ((xEdge[i+1] & 63)) * fac / 64);
				for ( x1++; x1 < x2; x1++ ) {
					/* System.out.println("x at " + x + "," + y ); */
					baseAddr[x1] = (unsigned char)(baseAddr[x1] + fac);
				}
			}

			i += 2;
		}
		for ( i = firstXEdge; i < t->numEdges;  ) {
			tt_int32 y1, y2, x, k;
			
			/* draw from i to j */
			x  = xEdge[i];
			y1 = yEdge[i];
			y2 = yEdge[i+1];
			x  >>= 6;
			x -= xmin;
			y1 = (y1 + 0) >> 6;
			y2 = (y2 + 0) >> 6;
			
			/* y = y1; */
			k = ( h1 - (y1 - ymin) )  * rowBytes + x;
			if ( y1 == y2 ) {
				baseAddr[k] = (unsigned char)(baseAddr[k] + ((yEdge[i+1] & 63) - 
									     (yEdge[i] & 63)) * fac / 64);			
			} else {
				baseAddr[k] = (unsigned char)(baseAddr[k] + (64 - (yEdge[i] & 63)) * fac / 64);			
				/* y  = y2; */
				/* k2  = ( h1 - (y2 - ymin) ) * rowBytes + x; */
				x += ( h1 - (y2 - ymin) ) * rowBytes; /* use x as k2 */
				baseAddr[x] = (unsigned char)(baseAddr[x] + ((yEdge[i+1] & 63)) * fac / 64);
				/* k = ( h1 - (y - ymin)  ) * rowBytes + x; */
				
				for ( x += rowBytes; x < k; x += rowBytes ) {
					baseAddr[x] = (unsigned char)(baseAddr[x] + fac); 
					/* walk through memory in increasing order */
				}
			}

			i += 2;
		}	
		if ( xWeightIsOne ) {
			int x, y, xLimit;
			unsigned char sum, b0, b1, delta;
			xLimit   = rowBytes;
			baseAddr = t->baseAddr;
			for ( y = h; y > 0; y-- ) {
				b0 = baseAddr[0];
				for ( x = 1; x < xLimit; x++ ) {
					b1  = baseAddr[x];
					sum = (unsigned char)(b0 + b1);
					if ( sum > 120 && sum <= 120 + 60 + 30 ) {
						delta   = (unsigned char)((sum - 120) >> 2);	/* 0.25 */
						delta   = (unsigned char)(delta + (delta >> 1 ));
						/* 0.375 is more gentle than 0.50 */
						b0			   = (unsigned char)(b0 - delta);
						b1			   = (unsigned char)(b1 - delta);
						baseAddr[x-1]  = b0;
						baseAddr[x  ]  = b1;
					}
					b0 = b1;
				}
				baseAddr += rowBytes;
			}
		}		
		return; /*****/
	}
	
	
	for ( i = 0; i < t->numEdges;  ) {
		tt_int32 x1, x2, y, k, b1, b2, bi;
		if ( t->edgeData[i] & IS_XEDGE ) {
			firstXEdge = i;
			break; /*****/
		}
		
		/* draw from i to j */
		x1 = xEdge[i];
		x2 = xEdge[i+1];
		x1 = (x1 + 32) >> 6;  /* note: center of pixel is biased upward, rather than including */
		x2 = (x2 + 32) >> 6;
		
		if ( x1 < x2 ) {
			y  = yEdge[i];
			y  >>= 6;
			x1 -= xmin;
			x2 -= xmin;
			
			k = ( h - 1 - (y - ymin) ) * rowBytes;
			
			/* old slow approach */
			/*
			for ( x = x1; x < x2; x++ ) {
				baseAddr[k + (x>>3)] |= (byte)( 0x80 >> (x & 0x07) );
			}
			*/
			b1 = x1 >> 3;
			x2--; /* Make x2 inclusive */
			b2 = x2 >> 3;
			if ( b1 == b2 ) {
				baseAddr[k + b1] |= (unsigned char)(0x00ff >> 
						    (x1 & 0x07)) & (unsigned char)(0xff80 >> (x2 & 0x07));
			} else {
				baseAddr[k + b1] |= (unsigned char)(0x00ff >> (x1 & 0x07));
				baseAddr[k + b2] |= (unsigned char)(0xff80 >> (x2 & 0x07));
				
				for ( bi = b1+1; bi < b2; bi++ ) {
					baseAddr[k + bi] = (unsigned char)0xff;
				}
			}
		}
		i += 2;
	}
 	/* x dropout control */
 	if (isDropout)
 	{
		{
			for ( i = 0; i < firstXEdge;  ) {
				tt_int32 x1, x2, y, x, k;
				
				/* draw from i to j */
				x1 = xEdge[i];
				x2 = xEdge[i+1];
				
				if ( (x2 - x1) < 64 ) {
					y  = yEdge[i];
					y  >>= 6;
					x1 = (x1 + 0) >> 6;
					x2 = (x2 + 0) >> 6;
					x1 -= xmin;
					x2 -= xmin;
					
					k = ( h - 1 - (y - ymin) ) * rowBytes;
					if ( (baseAddr[k + (x1>>3)] & (unsigned char)( 0x80 >> (x1 & 0x07))) == 0 
					     && (baseAddr[k + (x2>>3)] & (unsigned char)( 0x80 >> (x2 & 0x07))) == 0 ) {
						/* dropout */
						x = xEdge[i];
						if ( x > xmid ) { 
						  /* create a slight preference to go towards the center */
							x = (x + xEdge[i+1]-2)>>1;
						} else {
							x = (x + xEdge[i+1]+1)>>1;
						}
						x = (x+0) >> 6;
						x -= xmin;
						/* System.out.println("x-drop at " + x + "," + y ); */
						baseAddr[k + (x>>3)] |= (unsigned char)( 0x80 >> (x & 0x07) );
					}
				} 
				i += 2;
			}
		}


		/* y dropout control */
		{
			for ( i = firstXEdge; i < t->numEdges;  ) {
				tt_int32 y1, y2, y, x, k1, k2;
				
				/* draw from i to j */
				y1 = yEdge[i];
				y2 = yEdge[i+1];
				
				
				if ( (y2 - y1) < 64 ) {
					x  = xEdge[i];
					x  >>= 6;
					x -= xmin;
					y1 = (y1 + 0) >> 6;
					y2 = (y2 + 0) >> 6;
					k1 = ( h - 1 - (y1 - ymin) ) * rowBytes;
					k2 = ( h - 1 - (y2 - ymin) ) * rowBytes;
					if ( (baseAddr[k1 + (x>>3)] & (unsigned char)( 0x80 >> (x & 0x07))) == 0 && 
					     (baseAddr[k2 + (x>>3)] & (unsigned char)( 0x80 >> (x & 0x07))) == 0 ) {
						/* dropout */
						y = yEdge[i];
						if ( y > ymid ) { 
						  /* create a slight preference to go towards the center */
							y = (y + yEdge[i+1]-2)>>1;
						} else {
							y = (y + yEdge[i+1]+1)>>1;
						}
						y = (y+0) >> 6;
						/* System.out.println("y-drop at " + x + "," + y ); */
						k1 = ( h - 1 - (y - ymin) ) * rowBytes;
						baseAddr[k1 + (x>>3)] |= (unsigned char)( 0x80 >> (x & 0x07) );
					}
				} 
				i += 2;
			}
		}
    }
}


void tsi_DeleteScanConv( tsiScanConv *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->baseAddr );
		tsi_DeAllocMem( t->mem, (char *)t->xEdge );
		tsi_DeAllocMem( t->mem, (char *)t );
	}
}
