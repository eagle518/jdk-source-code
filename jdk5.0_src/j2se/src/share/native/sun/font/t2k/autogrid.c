/*
 * @(#)autogrid.c	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * File:		AUTOGRID.C
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
#include "autogrid.h"
#include "agridint.h"
#include "util.h"




#ifdef ENABLE_AUTO_HINTING

/* Begin List of TrueType Op-codes */
#define tt_SVTCA_Y		0x00
#define tt_SVTCA_X		0x01

#define tt_CALL			0x2B

#define tt_SRP0_BASE	0x10
#define tt_SRP1_BASE	0x11
#define tt_SRP2_BASE	0x12

#define tt_PUSHB_BASE	0xB0
#define tt_PUSHW_BASE	0xB8
#define tt_NPUSHB		0x40
#define tt_NPUSHW		0x41

#define tt_MIAP_BASE	0x3E
#define tt_MDAP_BASE	0x2E

#define tt_ALIGNRP		0x3C
#define tt_MDRP_BASE	0xC0
#define tt_MIRP_BASE	0xE0

#define tt_IP_BASE		0x39

#define tt_IUPY			0x30
#define tt_IUPX			0x31

#define tt_IF			0x58
#define tt_ELSE			0x1B
#define tt_EIF			0x59
#define tt_JMPR			0x1C

#define tt_RS			0x43
#define tt_WS			0x42

/* End List of TrueType Op-codes */
#endif /* ENABLE_AUTO_HINTING */


#define A_LOT_OF_POINTS_FOR_THE_SAME_COORDINATE 128


/*#undef assert*/
/*#define assert(x) NULL*/


/*
 * Description:		Returns the absolute value for an tt_int16.
 * How used:		Just call with the value.
 * Side Effects: 	none.
 * Return value: 	the absolute of z.
 */
tt_int16 ag_Abs16( register tt_int16 z )
{
	if ( z < 0 ) z = (tt_int16)-z;
	return z; /*****/
}

/*
 * Description:		Approximates the Euclidian distance sqrt(dx*dx+dy*dy).
 * How used:		Call with dx and dy.
 * Side Effects: 	None.
 * Return value: 	The distance.
 */
static tt_int16 ag_FDist( register tt_int16 dx, register tt_int16 dy )
{
	if ( dx < 0 ) dx = (tt_int16)-dx;
	if ( dy < 0 ) dy = (tt_int16)-dy;
	/* This is close enough to sqrt( dx * dx + dy * dy */
	return (tt_int16)(dx > dy ? (dx + (dy>>1)) : (dy + (dx>>1))); /*****/
}

/*
 * Description:		Returns the x distance between points pt1 and pt2.
 * How used:		Call with a pointer to ag_DataType, and the two point numbers.
 * Side Effects: 	None.
 * Return value: 	The x-distance.
 */
static int ag_XDist( register ag_DataType *hData, register int pt1, register int pt2 )
{
	register int dx = hData->oox[pt2] - hData->oox[pt1];
	/* If negative then just change the sign */
	if ( dx < 0 ) dx = -dx;
	return dx; /*****/
}

/*
 * Description:		Returns the y distance between points pt1 and pt2.
 * How used:		Call with a pointer to ag_DataType, and the two point numbers.
 * Side Effects: 	None.
 * Return value: 	The y-distance.
 */
static int ag_YDist( register ag_DataType *hData, register int pt1, register int pt2 )
{
	register int dy = hData->ooy[pt2] - hData->ooy[pt1];
	/* If negative then just change the sign */
	if ( dy < 0 ) dy = -dy;
	return dy; /*****/
}

/*
 * Description:		Computes the cvt-height number for the point 'pt'.
 * How used:		Call with ag_DataType and a point number.
 * Side Effects: 	none.
 * Return value: 	the cvt number, or < 0 if no height is found
 */
tt_int16 ag_Height( register ag_DataType *hData, int pt )
{
	register int	i, y, tmp, min, isFigure;
	tt_int16 			cvtEntry, lowY;
	ag_HeightType	*heights;
	int 			doesNotPointRight, doesNotPointtLeft, doBothHeights;
	tt_int16 prev, next, thresHold;
	
	doesNotPointRight	= hData->cos_f[pt] <= ag_COS_10_DEG && hData->cos_b[pt]  <= ag_COS_10_DEG;
	doesNotPointtLeft	= hData->cos_f[pt] >= -ag_COS_10_DEG && hData->cos_b[pt] >= -ag_COS_10_DEG;
	
	prev		= hData->prevPt[pt];
	next		= hData->nextPt[pt];
	y			= hData->ooy[pt];
	/* Added 6/16/98, To fix height problem in Apple Palatino Bold */
	if ( doesNotPointRight && (hData->flags[pt] & YEX) && y > hData->ooy[prev] && y > hData->ooy[next]  ) {
		doesNotPointRight = false; /* Pretend we have a tangent going to the right. */
	}
	if ( doesNotPointtLeft && (hData->flags[pt] & YEX) && y < hData->ooy[prev] && y < hData->ooy[next]  ) {
		doesNotPointtLeft = false; /* Pretend we have a tangent going to the left. */
	}
	/* For heights we only care about points with tangents going left or right */
	if ( doesNotPointRight && doesNotPointtLeft ) return -1; /*****/
	
	heights		= hData->gData.heights;
	lowY 		= (tt_int16)(hData->unitsPerEm/4);
	min			= hData->unitsPerEm / 66; /* heightFudge */
	cvtEntry 	= -1;
	isFigure	= hData->isFigure;

	
	doBothHeights = true;
	/* If an oncurve point */
	if ( hData->onCurve[ pt ] ) {
		
		thresHold	= (tt_int16)(hData->unitsPerEm/16);
		/* Sometimes flat heights are closer to rounds than the flats.... */
		if ( (hData->onCurve[ next ] ) &&
		      ag_Abs16( (short)(hData->oox[pt] - hData->oox[next]) ) > thresHold &&
		      (hData->cos_f[pt] > ag_COS_5_DEG || hData->cos_f[pt] < -ag_COS_5_DEG) ) {
			doBothHeights = false; /*****/
		} else if ( (hData->onCurve[ prev ] ) &&
		      ag_Abs16( (short)(hData->oox[pt] - hData->oox[prev]) ) > thresHold &&
		      (hData->cos_b[pt] > ag_COS_5_DEG || hData->cos_b[pt] < -ag_COS_5_DEG) ) {
			doBothHeights = false; /*****/
		}
	}
	
	for ( i = 0; i <= ag_PARENTHESES_BOTTOM; i++ ) {
		tmp = heights[i].flat;
		if ( tmp > lowY ) {
			/* top, black is below us */
			if ( doesNotPointRight ) continue; /*****/
		} else {
			/* bottom, black is above us */
			if ( doesNotPointtLeft )  continue; /*****/
		}

		/* New May 6, 1992 */
		if ( i == ag_FIGURE_HEIGHT || i == ag_FIGURE_BASE_HEIGHT ) {
			if ( !isFigure ) continue;/*****/
		} else {
			if ( isFigure ) continue; /*****/
		}
		
		/* F L A T */
		/* tmp = y - heights[i].flat; */
		tmp = tmp - y;
		if ( tmp < 0 ) tmp = -tmp;
		if ( tmp < min ) {
			min = tmp;
			/* cvtEntry = (short)(i * HEIGHT_ENTRY_SIZE); */
			cvtEntry = (short)(i+i);
			if ( min == 0 ) break; /*****/
		}

		if ( doBothHeights ) {
			/* R O U N D */
			tmp = y - heights[i].round;
			if ( tmp < 0 ) tmp = -tmp;
			if ( tmp < min ) {
				min = tmp;
				/* cvtEntry = (short)(i * HEIGHT_ENTRY_SIZE + 1); */
				cvtEntry = (short)(i+i+1);
				if ( min == 0 ) break; /*****/
			}
		}
	}
	return cvtEntry; /*****/
}

/*
 * Description:		Normalizes the Euclidian length of the (*A,*B) vector to SMALLFRAC_ONE.
 *					This routine has to go really fast.
 * How used:		Call with pointers to the data (dx, and dy).
 * Side Effects: 	Writes to *A and *B.
 * Return value: 	None.
 */
static void ag_DoubleNorm( register tt_int32 *A, register tt_int32 *B )
{
	register tt_int32 tmpA, tmpB;
	if ( (tmpA = *A) == 0 ) {
		*B = *B >= 0 ? SMALLFRAC_ONE : -SMALLFRAC_ONE;
	} else if ( (tmpB = *B) == 0 ) {
		*A = tmpA >= 0 ? SMALLFRAC_ONE : -SMALLFRAC_ONE;
	} else {
		register tt_int32 root, old_root;
		register tt_int32 square;
		
		root	 = tmpA >= 0 ? tmpA : -tmpA; /* Do an initial approximation, in root */
		old_root = tmpB >= 0 ? tmpB : -tmpB;
		/* square   = tmpA*tmpA + tmpB*tmpB; */
		root	= root > old_root ? root + (old_root>>1) : old_root + (root>>1);

		/* Rescale to about SMALLFRAC_ONE to increase precision. March 12, 1997 */
		tmpA	= tmpA * SMALLFRAC_ONE / root;
		tmpB	= tmpB * SMALLFRAC_ONE / root;
		root	= SMALLFRAC_ONE;

		square	= tmpA*tmpA + tmpB*tmpB;

		/* Ok, now enter the Newton Raphson loop */
		do {
			root = ((old_root = root) + square/root + 1 ) >> 1;
		} while (old_root != root );
		/* Done, now scale the vector to length SMALLFRAC_ONE */
		assert( root != 0 );
		*A = (tmpA * SMALLFRAC_ONE) / root;
		*B = (tmpB * SMALLFRAC_ONE) / root;
	}
}


/*
 * Description:		Computes true tangents and coordinates for all points.
 *					Also computes and sets the realX, and realY arrays
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	Sets cos_b, cos_f, sin_b, sin_f, realX, and realY arrays in ag_DataType.
 * Return value: 	None.
 */
static void ag_ComputeTangents( register ag_DataType *hData )
{
	register int A, B, C, lastPoint, ctr;
	tt_int16 xC, yC, xB, yB, xA, yA;
	tt_int32 dxF, dyF;
	tt_int32 dxB, dyB;
	register tt_uint8 *onCurve = hData->onCurve; 
	register tt_int16 *oox = hData->oox;
	register tt_int16 *ooy = hData->ooy;
	
	assert( hData != NULL );
	/* Loop for each contour */
	for ( ctr = 0; ctr < hData->numberOfContours; ctr++ ) {
		lastPoint = hData->endPoint[ ctr ];
		if ( lastPoint <= hData->startPoint[ ctr ] ) continue; /*****/
		
		B  = hData->startPoint[ ctr ];
		A  = lastPoint;
		xA = oox[A];
		yA = ooy[A];
	    while ( B <= lastPoint ) {
	    	tt_int16 realX, realY;
	    	
	    	C  = hData->nextPt[B];
			xB = oox[B];
			yB = ooy[B];
			xC = oox[C];
			yC = ooy[C];
			if ( onCurve[ B ] ) {
				dxF = xC - xB;
				dyF = yC - yB;
				dxB = xB - xA;
				dyB = yB - yA;
				realX = xB;
				realY = yB;
			} else {
				tt_int16 xAB, yAB, xBC, yBC;
				
				if ( !onCurve[ C ] ) {
					xC = (tt_int16)(xC + xB + 1); xC >>= 1;
					yC = (tt_int16)(yC + yB + 1); yC >>= 1;
				}
				if ( !onCurve[ A ] ) {
					xA = (tt_int16)(xA + xB + 1); xA >>= 1;
					yA = (tt_int16)(yA + yB + 1); yA >>= 1;
				}
				xAB = (tt_int16)((xA + xB + 1) >> 1);
				yAB = (tt_int16)((yA + yB + 1) >> 1);
				xBC = (tt_int16)((xB + xC + 1) >> 1);
				yBC = (tt_int16)((yB + yC + 1) >> 1);
				dxB = dxF = xBC - xAB;
				dyB = dyF = yBC - yAB;
				realX = (tt_int16)((xBC + xAB + 1) >> 1);
				realY = (tt_int16)((yBC + yAB + 1) >> 1);
			}
			
			ag_DoubleNorm( &dxF, &dyF );
			hData->cos_f[B] = dxF;
			hData->sin_f[B] = dyF;
			if ( !(onCurve[ B ] ) || (dxB == 0 && dxF == 0) || (dyB == 0 && dyF == 0) ) {		
				hData->cos_b[B] = hData->cos_f[B];
				hData->sin_b[B] = hData->sin_f[B];
			} else {
				ag_DoubleNorm( &dxB, &dyB );
				hData->cos_b[B] = dxB;
				hData->sin_b[B] = dyB;
			}				
			hData->realX[B] = realX;
			hData->realY[B] = realY;
			A	= B++;
			xA	= xB;
			yA	= yB;
	    }
	}
}


/*
 * Description:		Returns the previous different y value.
 *					This is the coordinate of the first point which is prior to the current point
 *					AND not on the same coordinate as the current point.
 * How used:		Call with pointer to ag_DataType and a point number.
 * Side Effects: 	None.
 * Return value: 	The y coordinate of the previous different y value.
 */
static tt_int16 ag_PrevDifferentYValue( register ag_DataType *hData, register int pt )
{
	register tt_int16	thisY, *coord = hData->ooy;
	register int	prevPoint, i = A_LOT_OF_POINTS_FOR_THE_SAME_COORDINATE;

	thisY 		= coord[pt];
	prevPoint	= pt;
	do {
		prevPoint = hData->prevPt[prevPoint];
	} while ( coord[prevPoint] == thisY && prevPoint != pt && i-- > 0 );
	return coord[prevPoint]; /*****/
}

/*
 * Description:		Returns the previous different x value.
 *					This is the coordinate of the first point which is prior to the current point
 *					AND not on the same coordinate as the current point.
 * How used:		Call with pointer to ag_DataType and a point number.
 * Side Effects: 	None.
 * Return value: 	The x coordinate of the previous different y value.
 */
static tt_int16 ag_PrevDifferentXValue( register ag_DataType *hData, register int pt )
{
	register tt_int16	thisX, *coord = hData->oox;
	register int	prevPoint, i = A_LOT_OF_POINTS_FOR_THE_SAME_COORDINATE;

	thisX		= coord[pt];
	prevPoint	= pt;
	do {
		prevPoint = hData->prevPt[prevPoint];
	} while ( coord[prevPoint] == thisX && prevPoint != pt && i-- > 0);
	return coord[prevPoint]; /*****/
}


/*
 * Description:		This routine finds all the X and Y extrema as fast as possible.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	Sets the XEX and YEX bit fields in flags array in ag_DataType.
 * Return value: 	None.
 */
static void ag_FindXandYExtrema( register ag_DataType *hData )
{
	register tt_int16	*coord;
	register tt_uint16	*flags = hData->flags;
	register tt_int16	*nextPt = hData->nextPt; 
	register int	B, C, ctr;
	tt_int16 			Az, Bz, Cz;

	for ( ctr = 0; ctr < hData->numberOfContours; ctr++ ) {
		/* Do the X-direction */
		C		= hData->startPoint[ ctr ];
		if ( hData->endPoint[ ctr ] <= C ) continue; /*****/
		coord	= hData->oox;
		Bz		= ag_PrevDifferentXValue( hData, C );
		Cz		= coord[C];
		do {
    		Az	= Bz;
    		Bz	= Cz;
    		B	= C;
		    do {
		    	C  = nextPt[C];
			    Cz = coord[C];
		    } while ( Cz == Bz && C != B );
    		if ( (Bz > Cz && Bz > Az) || (Bz < Cz && Bz < Az) ) {
				flags[B] |= XEX;
    		}
    	} while ( C > B );

		/* Do the Y-direction */
		C		= hData->startPoint[ ctr ];;
		coord	= hData->ooy;
		Bz		= ag_PrevDifferentYValue( hData, C );
		Cz		= coord[C];
		do {
    		Az	= Bz;
    		Bz	= Cz;
    		B	= C;
		    do {
		    	C  = nextPt[C];
			    Cz = coord[C];
		    } while ( Cz == Bz && C != B );
    		if ( (Bz > Cz && Bz > Az) || (Bz < Cz && Bz < Az) ) {
				flags[B] |= YEX;
    		}
    	} while ( C > B );
	}
}

/*
 * Description:		Finds and marks all the inflection points.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	Sets the INFLECTION bit field in flags array in ag_DataType.
 * Return value: 	None.
 */
static void ag_FindInflections( register ag_DataType *hData )
{
	register int A, B, C;
	int ctr, lastPoint, beginPt, oldA, oldB, oldC, point;
	tt_int32 crossProduct, oldCrossProduct;
	tt_int32 dx1,dy1,dx2,dy2;
	int sign1, sign2;
	tt_int32 limit;
	register tt_uint16 *flags = hData->flags;
	register tt_int16 *nextPt = hData->nextPt;
	
	limit = hData->unitsPerEm / 256;
	if ( limit == 0 ) limit = 1;
	
	A = B = C = 0;
	for ( ctr = 0; ctr < hData->numberOfContours; ctr++ ) {
		lastPoint = hData->endPoint[ ctr ];
		if ( lastPoint <= hData->startPoint[ ctr ]+1 ) continue; /*****/
		
		crossProduct = 0;
		B = hData->startPoint[ ctr ];
		A = hData->prevPt[B];
      	while ( B <= lastPoint) {
			C = nextPt[B];
			dx1 = hData->oox[B] - hData->oox[A];
			dy1 = hData->ooy[B] - hData->ooy[A];
			dx2 = hData->oox[C] - hData->oox[B];
			dy2 = hData->ooy[C] - hData->ooy[B];
			
			if ( !( flags[B] & CORNER ) ) {
				crossProduct = dx1 * dy2 - dx2 * dy1;
				crossProduct /= (ag_FDist( (short)(dx1+dx2), (short)(dy1+dy2) ) +1);
				if (crossProduct<-limit || crossProduct>limit) break; /*****/
			}
			A = B++;
       }
		
		if ( crossProduct == 0 ) {
			B = hData->startPoint[ ctr ];
			A = hData->prevPt[B];
	      	while ( B <= lastPoint) {
				C = nextPt[B];
				dx1 = hData->oox[B] - hData->oox[A];
				dy1 = hData->ooy[B] - hData->ooy[A];
				dx2 = hData->oox[C] - hData->oox[B];
				dy2 = hData->ooy[C] - hData->ooy[B];
				
				crossProduct = dx1 * dy2 - dx2 * dy1;
				crossProduct /= (ag_FDist( (short)(dx1+dx2), (short)(dy1+dy2) ) +1);
				
				if ( crossProduct != 0 ) break; /*****/
		        A = B++;
	        }
	    }
      	if ( B > lastPoint ) continue; /*****/
    
    	oldA = A;
    	oldB = B;
    	oldC = C;
    	oldCrossProduct = crossProduct;
		sign2 = (oldCrossProduct >= 0 ? 1 : -1);
      	beginPt = B;
      	do {
			A = hData->prevPt[B];
			C = nextPt[B];
			dx1 = hData->oox[B] - hData->oox[A];
			dy1 = hData->ooy[B] - hData->ooy[A];
			dx2 = hData->oox[C] - hData->oox[B];
			dy2 = hData->ooy[C] - hData->ooy[B];
			
			crossProduct = dx1 * dy2 - dx2 * dy1;
			crossProduct /= (ag_FDist( (short)(dx1+dx2), (short)(dy1+dy2) ) +1);
			sign1 = (crossProduct >= 0 ? 1 : -1);
      	
			if ( crossProduct != 0 && (crossProduct<-limit || crossProduct>limit) ) {
				if ( sign1 != sign2 && (oldCrossProduct<-limit || oldCrossProduct>limit) ) {
					if ( !((flags[oldA] | flags[oldB] | flags[oldC]) & CORNER) ) {
						if ( !((flags[A] | flags[B] | flags[C]) & CORNER) ) {
							int pointCount, limitCount;
							
							point = oldA;
							for ( pointCount = 0; point != C; pointCount++ ) {
								point = nextPt[point];
							}
							limitCount = (pointCount+1)/2;
							point = oldA;
							for ( pointCount = 0; pointCount < limitCount; pointCount++ ) {
								point = nextPt[point];
							}
							if ( !(flags[point] & CORNER) ) {
								flags[point] |= INFLECTION;
							}
						}
					}
				}
		    	oldA = A;
		    	oldB = B;
		    	oldC = C;
				oldCrossProduct = crossProduct;
				sign2 = sign1;
			}
			if ( flags[B] & CORNER ) {
				oldCrossProduct = crossProduct = 0;
			}
      	
	    	B = nextPt[B];
	    } while ( B != beginPt);
	}
}


/*
 * Description:		Marks up all the points by
 * 					setting various bit flags in the flags array
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	Sets the flags array in ag_DataType.
 * Return value: 	None.
 */
static void ag_MarkPoints( register ag_DataType *hData )
{
	register int point, nextPt, prevPt;
	int lastPoint, limit, bigDistance;
	register tt_uint16 *flags = hData->flags;
	register tt_uint8 *onCurve = hData->onCurve;
	int limit1000 = ONE_PERCENT_OF_THE_EM/10+1; /* 1%/10 + 1 fUnit */
	tt_int32 dist;
	
	lastPoint = hData->endPoint[ hData->numberOfContours-1 ];
	
	/* Clear out the flags array */
	for ( point = lastPoint+2; point >= 0; point-- ) {
		flags[point] = 0;
	}
	
	/* Find all x and y extrema */
	ag_FindXandYExtrema( hData );
	
	/* dump extra extremas */
	for ( point = lastPoint; point >= 0; point-- ) {
		if ( !(onCurve[point] ) ) {
			nextPt = hData->nextPt[point];
			prevPt = hData->prevPt[point];
			if ( flags[point] & XEX ) {
				dist = hData->oox[point] - hData->oox[nextPt]; if ( dist < 0 ) dist = -dist;
				if ( (onCurve[nextPt] ) && dist <= limit1000 ) {
					flags[point]  &= ~XEX;
					flags[nextPt] |=  XEX;
				}
				dist = hData->oox[point] - hData->oox[prevPt]; if ( dist < 0 ) dist = -dist;
				if ( (onCurve[prevPt] ) && dist <= limit1000 ) {
					flags[point]  &= ~XEX;
					flags[prevPt] |=  XEX;
				}
			}
			if ( flags[point] & YEX ) {
				dist = hData->ooy[point] - hData->ooy[nextPt]; if ( dist < 0 ) dist = -dist;
				if ( (onCurve[nextPt] ) && dist <= limit1000 ) {
					flags[point]  &= ~YEX;
					flags[nextPt] |=  YEX;
				}
				dist = hData->ooy[point] - hData->ooy[prevPt]; if ( dist < 0 ) dist = -dist;
				if ( (onCurve[prevPt] ) && dist <= limit1000 ) {
					flags[point]  &= ~YEX;
					flags[prevPt] |=  YEX;
				}
			}
		}
	}

	/* Mark corners, + direction */
	for ( point = lastPoint; point >= 0; point-- ) {
		/* more than a 5 degree directional change */
		if ( SMALLFRACVECMUL(hData->cos_f[point], hData->cos_b[point], hData->sin_f[point], hData->sin_b[point])  <= ag_COS_5_DEG ) {
			flags[point] |= CORNER;
		}
/* #define GO_LIGHT_ON_ITALICS */
#define GO_LIGHT_ON_ITALICS
#ifdef GO_LIGHT_ON_ITALICS
		if ( hData->cos_f[point] > ag_COS_5_DEG || hData->cos_f[point] < -ag_COS_5_DEG ) {
			flags[point] |= IN_XF;
		} else if ( hData->sin_f[point] > ag_COS_5_DEG || hData->sin_f[point] < -ag_COS_5_DEG ) {
			flags[point] |= IN_YF;
		}
		if ( hData->cos_b[point] > ag_COS_5_DEG || hData->cos_b[point] < -ag_COS_5_DEG ) {
			flags[point] |= IN_XB;
		} else if ( hData->sin_b[point] > ag_COS_5_DEG || hData->sin_b[point] < -ag_COS_5_DEG ) {
			flags[point] |= IN_YB;
		}
#else
		if ( hData->cos_f[point] > ag_COS_15_DEG || hData->cos_f[point] < -ag_COS_15_DEG ) {
			flags[point] |= IN_XF;
		} else if ( hData->sin_f[point] > ag_COS_15_DEG || hData->sin_f[point] < -ag_COS_15_DEG ) {
			flags[point] |= IN_YF;
		}
		if ( hData->cos_b[point] > ag_COS_15_DEG || hData->cos_b[point] < -ag_COS_15_DEG ) {
			flags[point] |= IN_XB;
		} else if ( hData->sin_b[point] > ag_COS_15_DEG || hData->sin_b[point] < -ag_COS_15_DEG ) {
			flags[point] |= IN_YB;
		}
#endif
	}	

	/* Mark rounds */
	limit = (hData->unitsPerEm >> 7) + 1;
	for ( point = lastPoint; point >= 0; point-- ) {
		if ( flags[point] & CORNER  ) continue; /*****/
		
		nextPt = hData->nextPt[point];
		prevPt = hData->prevPt[point];
		if (onCurve[point] ) {
			if ( flags[point] & XEX ) {
				if ( (!(onCurve[nextPt] ) && ag_XDist(hData, point, nextPt) < limit) &&
				     (!(onCurve[prevPt] ) && ag_XDist(hData, point, prevPt) < limit) ) {
					 flags[point] |= X_ROUND;
				}
			}
			if ( flags[point] & YEX ) {
				if ( (!(onCurve[nextPt] ) && ag_YDist(hData, point, nextPt) < limit) &&
				     (!(onCurve[prevPt] ) && ag_YDist(hData, point, prevPt) < limit) ) {
					 flags[point] |= Y_ROUND;
				}
			}
		} else {
			if ( flags[point] & XEX ) {
				if ( (!(onCurve[nextPt] ) && ag_XDist(hData, point, nextPt) < limit) ||
				     (!(onCurve[prevPt] ) && ag_XDist(hData, point, prevPt) < limit) ) {
					 flags[point] |= X_ROUND;
				}
			}
			if ( flags[point] & YEX ) {
				if ( (!(onCurve[nextPt] ) && ag_YDist(hData, point, nextPt) < limit) ||
				     (!(onCurve[prevPt] ) && ag_YDist(hData, point, prevPt) < limit) ) {
					 flags[point] |= Y_ROUND;
				}
			}
		}
	}
		
	/* Find inflections */
	ag_FindInflections( hData ); 
	
	/* Mark x and y important points */
	for ( point = (lastPoint+2); point >= 0; point-- ) {
		if ( flags[point] & (CORNER | XEX) ) {
			flags[point] |= X_IMPORTANT;
		}
		if ( flags[point] & (CORNER | YEX) ) {
			flags[point] |= Y_IMPORTANT;
		}
	}
	flags[lastPoint+1] |= X_IMPORTANT; 
	flags[lastPoint+2] |= X_IMPORTANT; 
	
	/* Lines */
	bigDistance = TEN_PERCENT_OF_THE_EM;
	for ( point = lastPoint; point >= 0; point-- ) {
		nextPt = hData->nextPt[point];
		if ( onCurve[point] && onCurve[nextPt] ) {
			tt_int16 dx, dy, dist;
			
			dx		= (tt_int16)(hData->oox[point] - hData->oox[nextPt]);
			dy		= (tt_int16)(hData->ooy[point] - hData->ooy[nextPt]);
			dist	= ag_FDist(dx, dy);
			if ( dist >= bigDistance ) {
				if ( hData->cos_f[point] >= -ag_SIN_2_5_DEG && hData->cos_f[point] <= ag_SIN_2_5_DEG ) {
					flags[point]  |= X_IMPORTANT;
					flags[nextPt] |= X_IMPORTANT;
				} else 	if ( hData->sin_f[point] >= -ag_SIN_2_5_DEG && hData->sin_f[point] <= ag_SIN_2_5_DEG ) {
					flags[point]  |= Y_IMPORTANT;
					flags[nextPt] |= Y_IMPORTANT;
				} else {
					flags[point]  |= (X_IMPORTANT | Y_IMPORTANT);
					flags[nextPt] |= (X_IMPORTANT | Y_IMPORTANT);
				}
			}
		}
	}
}

/*
 * Description:		Returns true if the point pair [pt1,pt2] represents
 * 					a black distance enclosed by roughly parallell lines.
 * How used:		Call with pointer to ag_DataType, the two points, and their projection vectors.
 * Side Effects: 	None.
 * Return value: 	true or false.
 */
static int ag_BlackAndParallell( register ag_DataType *hData, register int pt1, register int pt2, SMALLFRAC xProj1, SMALLFRAC yProj1 )
{
	tt_int32 dx = hData->oox[pt2] - hData->oox[pt1];
	tt_int32 dy = hData->ooy[pt2] - hData->ooy[pt1];
	
	/* In this test rotate Rotate [xProj1,yProj1] + 90 degrees */
	/* It should point in the direction of pt2  */
	if ( (yProj1 * dx - xProj1 * dy) > 0 ) {
 		register SMALLFRAC xProj2, yProj2;
 		
		/* dx, dy is pointing in opposite direction of pt1 so we just test for < 0 instead of > 0 */
		xProj2 = hData->cos_f[pt2];
		yProj2 = hData->sin_f[pt2];
		/* In the first test rotate Rotate [xProj2,yProj2] + 90 degrees */
		/* It should point in the direction of pt1 , and the projections should roughly point in exactly opposite directions */
		if ( ( (yProj2 * dx - xProj2 * dy) < 0 ) && (SMALLFRACVECMUL(xProj1, xProj2, yProj1, yProj2) <= -15892L) ) return true; /*****/
		
		xProj2 = hData->cos_b[pt2];
		yProj2 = hData->sin_b[pt2];
		/* In the first test rotate Rotate [xProj2,yProj2] + 90 degrees */
		/* It should point in the direction of pt1 , and the projections should roughly point in exactly opposite directions */
		if ( ( (yProj2 * dx - xProj2 * dy) < 0 ) && (SMALLFRACVECMUL(xProj1, xProj2, yProj1, yProj2) <= -15892L) ) return true; /*****/
	}
	return false; /*****/
}

/*
 * Description:		Finds point pairs enclosing black character features.
 * How used:		Call with pointer to ag_DataType.
 *
 * Algorithm:		The code loop through all important point pairs
 *					the two points are called pt1 and pt2.
 *					For each point pair the quantity:
 *					Dist(p1,p2)*0.50 + Rotate90Degrees(ForwardTangent(pt1)) * (pt2 - pt1)
 *					and
 *					Dist(p1,p2)*0.50 + Rotate90Degrees(BackwardTangent(pt1)) * (pt2 - pt1)
 *					is computed.
 *
 *					For each pt1 we remember the minumum value of the quantity
 *					and the pt2 that produced it.
 *					
 *					pt2 will go into
 *					hData->forwardAngleOrthogonalPair[pt1], and
 *					hData->backwardAngleOrthogonalPair[pt1]
 *					
 * Philosophy behind the algorithm and what we are trying to accomplish:
 *					The idea is to come if with a preliminary list
 *					of potential black features that need to be controlled.
 *					Basically the quantity the algorithm is minimizing ensures that
 *					we find points that are close and and fairly directly on opposites
 *					sides of the black area.				
 *					
 *					
 * Side Effects: 	Sets forwardAngleOrthogonalPair, and backwardAngleOrthogonalPair in ag_DataType.
 * Return value: 	None.
 */
static void ag_FindPointPairs( register ag_DataType *hData )
{
	register	int		pt1, pt2, searchPointCount;
	register	tt_uint16 	flags1, flags2;
	register 	tt_int32 	dist, forwardPenalty, backwardPenalty, fDist, xDist, yDist;
	short				i, j;
	SMALLFRAC			xProj1F, yProj1F;
	SMALLFRAC			xProj1B, yProj1B;
	int					straight, p2InX, p2InY;
	register tt_uint16		*flags = hData->flags;
		
				
	pt2 = hData->endPoint[ hData->numberOfContours-1 ]; /* pt2 == the last point for this loop */
	/* Loop through all the points and cache the important points */
	for ( searchPointCount = pt1 = 0; pt1 <= pt2; pt1++ ) {
		hData->forwardAngleOrthogonalPair[pt1]	= -1;
		hData->backwardAngleOrthogonalPair[pt1]	= -1;

		if ( ( flags[pt1] & (X_IMPORTANT | Y_IMPORTANT | INFLECTION) ) &&
		     ( flags[pt1] & ( IN_XF | IN_YF  | IN_XB | IN_YB ) ) &&  /* If you want diagonals then do not do this check */
		       hData->nextPt[pt1] != pt1) {
		    /* Cache the points in this array to speed up this routine */
			hData->searchPoints[ searchPointCount++ ] 	= (short)pt1;
		}
	}
	
	/* Outer loop for all the cached points from above */
	for ( i = 0; i < searchPointCount; i++ ) {
		pt1		= hData->searchPoints[i];
		flags1	= flags[pt1];
		/* cache the projection vectors into registers (if available) for a speed gain */
	    xProj1F = hData->cos_f[pt1];
		yProj1F = hData->sin_f[pt1];
	    xProj1B = hData->cos_b[pt1];
		yProj1B = hData->sin_b[pt1];
		
		straight = xProj1B == xProj1F && yProj1B == yProj1F;
		forwardPenalty = backwardPenalty = SHORTMAX;

		/* Inner loop for all the cached points from above */
		for ( j = 0; j < searchPointCount; j++ ) {
			pt2		= hData->searchPoints[j];
			/* A point pair can not refer to itself */
			if ( pt2 == pt1 ) continue; /*****/
			flags2	= flags[pt2];
			p2InY	= flags2 & (IN_YF | IN_YB);
			p2InX	= flags2 & (IN_XF | IN_XB);
			
			/* If you want diagonals then do not do this test */
			if ( !((p2InX && (flags1 & (IN_XF | IN_XB))) || (p2InY && (flags1 & (IN_YF | IN_YB)))) ) continue;/*****/
#ifdef OLD
			/* There is probably no harm in removing this, April 24, 1996 ---Sampo */
			if ( (!( flags1 & (X_IMPORTANT | Y_IMPORTANT) ) || !( flags2 & (X_IMPORTANT | Y_IMPORTANT) )) && !(flags1 & flags2 & INFLECTION) ) continue; /*****/
#endif
			xDist = hData->realX[pt2] -  hData->realX[pt1];
			yDist = hData->realY[pt2] -  hData->realY[pt1];
			/* Set fDist to roughly the Euclidian Distance from pt1 to pt2 * 0.50 */
			fDist = xDist; if ( fDist < 0 ) fDist = -fDist;
			dist  = yDist; if (  dist < 0 )  dist = -dist;
			fDist = fDist > dist ? (fDist>>1) + (dist>>2) : (fDist>>2) + (dist>>1); 
			
			if ( fDist < forwardPenalty ) {
				/* Do a forward test for pt1 */
				/* If you want diagonals then do not do this if-statement check */
	    		if ( (p2InX && (flags1 & IN_XF)) || (p2InY && (flags1 & IN_YF)) ) {
	    			/* Multiply the pt1->pt2 vector with the the tangent vector
	    			   at pt1 rotated 90 degrees */
					dist = SMALLFRACVECMUL(-yProj1F, xDist, xProj1F, yDist );
					if ( dist < 0 ) dist = -dist;
					dist = dist + fDist;
					/* If the point qualifies and has a lower penalty than what we have
					   found so far then remember the point and the penalty */
					if ( dist < forwardPenalty && ag_BlackAndParallell( hData, pt1, pt2, xProj1F, yProj1F ) ) {
						forwardPenalty							= dist;
						hData->forwardAngleOrthogonalPair[pt1]	= (short)pt2;
					}
				}
			}
			/* Only do this if there is a directional change in the outline
			   since we take care of the other case below in the  if ( straight ) block */
			if ( !straight && fDist < backwardPenalty ) {
				/* Do a backward test for pt1 */
				/* If you want diagonals then do not do this if-statement check */
				if ( (p2InX && (flags1 & IN_XB)) || (p2InY && (flags1 & IN_YB)) ) {
					dist = SMALLFRACVECMUL(-yProj1B, xDist, xProj1B, yDist );
					if ( dist < 0 ) dist = -dist;
					dist = dist + fDist;
					/* If the point qualifies and has a lower penalty than what we have
					   found so far then remember the point and the penalty */
					if ( dist < backwardPenalty && ag_BlackAndParallell( hData, pt1, pt2, xProj1B, yProj1B ) ) {
						backwardPenalty							= dist;
						hData->backwardAngleOrthogonalPair[pt1]	= (short)pt2;
					}
				}
			}
		}
		/* If the point is at a location with no directional change in the outline
		   then the forward and backward pairs are set to be identical */
		if ( straight ) {
			hData->backwardAngleOrthogonalPair[pt1]	= hData->forwardAngleOrthogonalPair[pt1];
		}
	}
}

/*
 * Description:		Does the topological analysis of the glyph.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	Sets contnents of ag_DataType.
 * Return value: 	None.
 */
static void ag_AnalyzeChar( register ag_DataType *hData )
{
	register int A, B, C;
	register int ctr, lastPoint, firstPoint;
	register tt_int16 *prevPt = hData->prevPt;
	register tt_int16 *nextPt = hData->nextPt;
	register tt_uint16 *flags;
	
	/* Initialize the prev and next point arrays */
	for ( ctr = 0; ctr < hData->numberOfContours; ctr++ ) {
		lastPoint  = hData->endPoint[ ctr ];
		firstPoint = hData->startPoint[ ctr ];

		if ( lastPoint != firstPoint ) {
			/* A normal multiple point contour */
			A = lastPoint; B = firstPoint; C = B + 1;
		    while ( B <= lastPoint ) {
		    	prevPt[ B ] = (short)A;
		    	nextPt[ B ] = (short)C;
		    	A = B;
		    	B = C++;
		    }
		    nextPt[ lastPoint ] = (short)firstPoint;
	    } else {
	    	/* Single point contour */
	    	prevPt[ firstPoint ] = (short)firstPoint;
	    	nextPt[ firstPoint ] = (short)firstPoint;
	    }
	}
	/* Computes the tangents, for the */
	/* cos_b, cos_f, sin_b, and sin_f arrays */
	/* and also computes and sets the realX, and RealY arrays */
	ag_ComputeTangents( hData );
	/* Sets various bit flags in the flags array */
	ag_MarkPoints( hData );
	/* Finds black point pairs and records them in */
	/* forwardAngleOrthogonalPair,  + forwardPenalty, and */
	/* backwardAngleOrthogonalPair, + backwardPenalty */
	ag_FindPointPairs( hData );
	/* ----- */
	/* Sets the HEIGHT bit flag in the flags array */
	flags = hData->flags;
	for ( ctr = 0; ctr < hData->numberOfContours; ctr++ ) {
		lastPoint = hData->endPoint[ ctr ];
		if ( lastPoint <= hData->startPoint[ ctr ] ) continue; /*****/
	    for ( B = hData->startPoint[ ctr ]; B <= lastPoint; B++ ) {
			if ( ( flags[B] & Y_IMPORTANT ) && ag_Height( hData, B ) >= 0  ) {
				flags[B] |= HEIGHT;
			}
		}
	}
}




#ifdef ENABLE_AUTO_HINTING


/*
 * Description:		Set the TrueType rererence point 0 equal to 'point'.
 * How used:		Call with the pointer to ag_DataType, and the point number.
 * Side Effects: 	Sets and increments ttcode in ag_DataType. Sets and increments ttdata in ag_DataType. Sets RP0 in ag_DataType;
 * Return value: 	None.
 */
static void ag_SRP0(ag_DataType *hData, int point)
{
	if ( hData->RP0 != point ) {
		*hData->ttcode++ = tt_SRP0_BASE;
		*hData->ttdata-- = (short)point;
		hData->RP0 = (short)point;
	}
}

/*
 * Description:		Set the TrueType rererence point 1 equal to 'point'.
 * How used:		Call with the pointer to ag_DataType, and the point number.
 * Side Effects: 	Sets and increments ttcode in ag_DataType. Sets and increments ttdata in ag_DataType. Sets RP1 in ag_DataType;
 * Return value: 	None.
 */
static void ag_SRP1(ag_DataType *hData, int point)
{
	if ( hData->RP1 != point ) {
		*hData->ttcode++ = tt_SRP1_BASE;
		*hData->ttdata-- = (short)point;
		hData->RP1 = (short)point;
	}
}

/*
 * Description:		Set the TrueType rererence point 2 equal to 'point'.
 * How used:		Call with the pointer to ag_DataType, and the point number.
 * Side Effects: 	Sets and increments ttcode in ag_DataType. Sets and increments ttdata in ag_DataType. Sets RP2 in ag_DataType;
 * Return value: 	None.
 */
static void ag_SRP2(ag_DataType *hData, int point)
{
	if ( hData->RP2 != point ) {
		*hData->ttcode++ = tt_SRP2_BASE;
		*hData->ttdata-- = (short)point;
		hData->RP2 = (short)point;
	}
}

/*
 * Description:		Interpolate point B, between the points A and C.
 * How used:		Call with the pointer to ag_DataType, and the points A, B, and C.
 * Side Effects: 	ttcode, ttdata, RP1, and RP2 in ag_DataType.
 * Return value: 	None.
 */
static void ag_IPPoint( ag_DataType *hData, int A, int B, int C )
{
	if ( hData->RP1 == C || hData->RP2 == A ) {
		ag_SRP1( hData, C );
		ag_SRP2( hData, A );
	} else {
		ag_SRP1( hData, A );
		ag_SRP2( hData, C );
	}
	*hData->ttcode++ = tt_IP_BASE;
	*hData->ttdata-- = (short)B;
}



void ag_IF( register ag_DataType *hData, ag_ElementType *elem, tt_int16 storeIndex )
{
	elem;
	assert( storeIndex >= 0 && storeIndex <= 255 );
	
	*hData->ttcode++ = tt_PUSHB_BASE; /* push 1 */
	*hData->ttcode++ = (unsigned char)storeIndex;
	*hData->ttcode++ = tt_RS;
	*hData->ttcode++ = tt_IF;
}

void ag_ELSE( register ag_DataType *hData, ag_ElementType *elem )
{
	elem;
	
	*hData->ttcode++ = tt_ELSE;
}

void ag_EIF( register ag_DataType *hData, ag_ElementType *elem )
{
	elem;
	
	
	*hData->ttcode++ = tt_EIF;
}


void ag_JMPR( register ag_DataType *hData, ag_ElementType *elem, tt_int32 positionOfTarget )
{
	tt_int32 position1, n;
	
	elem;
	
	/* add thre bytes for the push */
	position1 = hData->ttcode - hData->ttCodeBase + 3;
	n = positionOfTarget - position1;
	
	*hData->ttcode++ = tt_PUSHW_BASE; /* push 1 word */
	*hData->ttcode++ = (unsigned char)(n >> 8);
	*hData->ttcode++ = (unsigned char)(n & 0xff);
	
	*hData->ttcode++ = tt_JMPR;
}

#endif /* ENABLE_AUTO_HINTING */

void ag_LINK( ag_DataType *hData, register ag_ElementType* elem, tt_int16 *ooz, tt_int16 doX, tt_int16 doY, short minDist, short round, char c1, char c2, int from, int to )
{
	int pos;
	tt_int32 dist;
	short cvtNumber;
	
	dist = ooz[to] - ooz[from];
	if ( dist >= 0 ) {
		/* We have a positive distance */
		pos = true;
	} else {
		/* We have a negative distance */
		dist = -dist;
		pos	= false;
	}
	/* Do a one-directional link */
	if ( (cvtNumber = ag_GetCvtNumber( hData, doX, doY, 0, dist )) >= 0 ) {
		/* We have a cvt number */
		( pos ? ag_MoveDirectRelativePointInPositiveDirection : ag_MoveDirectRelativePointInNegativeDirection )( hData, elem, cvtNumber, from, to, doX );
	} else {
		/* We do not have a cvt number */
		(doX ? ag_MDRPX : ag_MDRPY)( hData, elem, -1, true, minDist, round, c1, c2, from, to );
	}
}

#ifdef OLD
/*
 * Function: 21 / ag_ADJUST
 *
 * Function 21 takes 3 arguments
 * stack: anchor, from, to => -
 *
 * This is to controls distances where point 'to' will move
 *
 *
 * Side effects: None;
 */
FDEF[], 21
#BEGIN
#PUSHOFF
/* stack: anchor, from, to 					*/
DUP[]
/* stack: anchor, from, to, to 	 			*/
ROT[]
/* stack: anchor, to, to, from 	 			*/
DUP[]
/* stack: anchor, to, to, from, from 	 	*/
ROT[]
/* stack: anchor, to, from, from, to        */
MD[O]
/* stack: anchor, to, from, -Dist_ft         */
NEG[]
/* stack: anchor, to, from, Dist_ft         */
#PUSH, 5, 4
/* stack: anchor, to, from, Dist_ft, 5, 4	*/
CINDEX[]
/* stack: anchor, to, from, Dist_ft, 5, to	*/
SWAP[]
/* stack: anchor, to, from, Dist_ft, to, 5	*/
MINDEX[]
/* stack: to, from, Dist_ft, to, anchor		*/
DUP[]
/* stack: to, from, Dist_ft, to, anchor, anchor */
ROT[]
/* stack: to, from, Dist_ft, anchor, anchor, to */
MD[O]
/* stack: to, from, Dist_ft, anchor, -Dist_at */
NEG[]
/* stack: to, from, Dist_ft, anchor, Dist_at */
/* NOTE!!! STORE_multiplier	== 10 */

#PUSH, 10
/* stack: to, from, Dist_ft, anchor, Dist_at, 10 */
RS[]
/* stack: to, from, Dist_ft, anchor, Dist_at, multiplier */
MUL[]
/* stack: to, from, Dist_ft, anchor, Dist_at' */
#PUSH, 1
/* stack: to, from, Dist_ft, anchor, Dist_at', 1/64 */
MUL[]
/* stack: to, from, Dist_ft, anchor, Dist_at'' */

SWAP[]
/* stack: to, from, Dist_ft, Dist_at, anchor */
GC[N]
/* stack: to, from, Dist_ft, Dist_at, coord_anchor */
ADD[]
/* stack: to, from, Dist_ft, value2 */
ROT[]
/* stack: to, Dist_ft, value2, from */
GC[N]
/* stack: to, Dist_ft, value2, coord_from */
ROT[]
/* stack: to, value2, coord_from, Dist_ft */

#PUSH, 10
/* stack: to, value2, coord_from, Dist_ft, 10 */
RS[]
/* stack: to, value2, coord_from, Dist_ft, multiplier */
MUL[]
/* stack: to, value2, coord_from, Dist_ft' */
#PUSH, 1
/* stack: to, value2, coord_from, Dist_ft', 1 */
MUL[]
/* stack: to, value2, coord_from, Dist_ft'' */

ADD[]
/* stack: to, value2, value1 */
/* NOTE!!! STORE_mulRepeatCount	== 11 */
#PUSH, 11
/* stack: to, value2, value1, 11 */
RS[]
/* stack: to, value2, value1, mulRepeatCount */
NOT[]
/* stack: to, value2, value1, mulRepeatCount==0 */

IF[]
/* stack: to, value2, value1,  */

DUP[]
/* stack: to, value2, value1, value1  */
ROT[]
/* stack: to, value1, value1, value2  */
DUP[]
/* stack: to, value1, value1, value2, value2  */
ROT[]
/* stack: to, value1, value2, value2, value1  */
GTEQ[]
/* stack: to, value1, value2, value2>=value1  */

IF[]
/* stack: to, value1, value2  */

#PUSH, 64
/* stack: to, value1, value2, 64  */
SUB[]
/* stack: to, value1, value2-64  */
SWAP[]
/* stack: to, value2-64, value1  */
DUP[]
/* stack: to, value2-64, value1, value1  */
ROT[]
/* stack: to, value1, value1, value2-64 */
MAX[]
/* stack: to, value1, new-value2 */

ELSE[]
/* stack: to, value1, value2  */

#PUSH, 64
/* stack: to, value1, value2, 64  */
ADD[]
/* stack: to, value1, value2+64  */
SWAP[]
/* stack: to, value2+64, value1  */
DUP[]
/* stack: to, value2+64, value1, value1  */
ROT[]
/* stack: to, value1, value1, value2+64 */
MIN[]
/* stack: to, value1, new-value2 */

EIF[]
SWAP[]
/* stack: to, new-value2, value1 */

EIF[]
/* stack: to, value2, value1,  */
/******/
/* stack: to, value2, value1 */
DUP[]
/* stack: to, value2, value1, value1 */
ADD[]
/* stack: to, value2, value1+ value1 */
ADD[]
/* stack: to, value2+value1+value1  */
#PUSH, 192
/* stack: to, value2+value1+value1, 3.0 */
DIV[]
/* stack: to, (value2+value1+value1)/3.0 */

ROUND[Gr]
/* stack: to, coord */
SCFS[]
/* stack: - */

#END
#PUSHON
ENDF[]
#endif

#ifdef ENABLE_AUTO_GRIDDING
void ag_ADJUST( register ag_DataType *hData, register ag_ElementType* elem, tt_int16 doX, tt_int16 UNUSED(doY), tt_int16 anchor, tt_int16 from, tt_int16 to )
{
	tt_int32 dist1, value1, dist2, value2;
	register F26Dot6 *z1;
	register short *ooz1;
	register tt_int32 multiplier;
	register short UPEM = hData->unitsPerEm;

	if ( doX ) {
		multiplier		= hData->xPixelsPerEm*ag_pixelSize;
		ooz1			= hData->oox;
		z1				= elem->x;
	} else {
		multiplier		= hData->yPixelsPerEm*ag_pixelSize;
		ooz1			= hData->ooy;
		z1				= elem->y;
	}
	/* just an approximation, using ooz1 is better
		dist1 = oz1[to] - oz1[from];
		dist2 = oz1[to] - oz1[anchor];
	*/
	/* This is equivalent to the TrueType MD instruction */
	dist1 = ooz1[to] - ooz1[from];
	dist2 = ooz1[to] - ooz1[anchor];
	dist1 *= multiplier;
	dist2 *= multiplier;
	dist1 += UPEM>>1;
	dist2 += UPEM>>1;
	dist1 /= UPEM;
	dist2 /= UPEM;

	dist1 = (dist1 * hData->storage[ STORE_multiplier ] + 32 ) >> 6;
	dist2 = (dist2 * hData->storage[ STORE_multiplier ] + 32 ) >> 6;
	dist1 /= 64;
	dist2 /= 64;

	value1 = z1[from]	+ dist1;
	value2 = z1[anchor] + dist2;
	
	if ( hData->storage[ STORE_mulRepeatCount ] == 0 ) {
		if ( value2 >= value1 ) {
			value2 -= 64;
			if ( value2 < value1 ) value2 = value1;
		} else {
			value2 += 64;
			if ( value2 > value1 ) value2 = value1;
		}
	}
	if ( hData->fontType == ag_KANJI ) {
		z1[to] = (value1 + value2 +1)/2; /* added 2/19/98 */
	} else {
		z1[to] = (value1 + value1 + value2 + 1)/3;
	}
	if ( !hData->strat98 || dist1 >= doX ? 44 : 32 ) {
		z1[to] += ag_pixelSize/2;
		z1[to] &= ~(ag_pixelSize-1);
	}
}
#else
void ag_ADJUST( ag_DataType *hData, register ag_ElementType* elem, tt_int16 doX, tt_int16 doY, tt_int16 anchor, tt_int16 from, tt_int16 to )
{
	elem; doX; doY;
	/* stack: anchor, from, to 					*/

	*hData->ttdata-- = 21;
	*hData->ttdata-- = to;
	*hData->ttdata-- = from;
	*hData->ttdata-- = anchor;
	*hData->ttcode++ = tt_CALL;
}
#endif

#ifdef ENABLE_AUTO_GRIDDING
void ag_ADJUSTSPACING( register ag_DataType *hData, register ag_ElementType* elem, tt_int32 lsbPoint, tt_int32 minPoint, tt_int32 maxPoint, tt_int32 rsbPoint )
{
	tt_int32 lsb1, rsb1, ws1, aw1;
	tt_int32 lsb2, rsb2, ws2;
	register F26Dot6 *x  = elem->x;
	register F26Dot6 *ox = hData->ox;
	F26Dot6  xLSB, xRSB;
	
	
	xLSB  = x[lsbPoint];
	xRSB  = x[rsbPoint];
	xRSB += ag_pixelSize/2;
	xRSB &= ~(ag_pixelSize-1);
	
	
	aw1  = xRSB	- xLSB;
	lsb1 = ox[minPoint]	- ox[lsbPoint];
	rsb1 = ox[rsbPoint]	- ox[maxPoint];
	if ( aw1 >= 3*ag_pixelSize && lsb1 > -8 && rsb1 > -8 ) {
		ws1		= lsb1 + rsb1;

		lsb2	= x[minPoint]	- xLSB;
		rsb2	= xRSB			- x[maxPoint];
		ws2		= lsb2			+ rsb2;

		if ( ws2 < ws1 - 32 + (rsb2 < 32 ? 25 : 0) ) {
			xRSB		+= ag_pixelSize;
		} else if ( ws2 > ws1 + 32 + 25 ) {
			x[lsbPoint]	+= ag_pixelSize;
		} else if ( rsb2 <= rsb1 - (rsb2 < 32 ? 0 : 25) && lsb2 > lsb1 + 25  ) {
			x[lsbPoint]	+= ag_pixelSize;
			xRSB		+= ag_pixelSize;
		}			
	}
	
	x[rsbPoint] = xRSB;
}
#else
void ag_ADJUSTSPACING( register ag_DataType *hData, register ag_ElementType* elem, tt_int32 lsbPoint, tt_int32 minPoint, tt_int32 maxPoint, tt_int32 rsbPoint )
{
	/* Not done yet */
	hData;
	elem;
	lsbPoint;
	minPoint;
	maxPoint;
	rsbPoint;
}
#endif

#ifdef OLD
/*
 * Function: 22
 *
 * Function 22 takes 0 arguments
 * stack: -
 *
 * Corresponds to ag_INIT_STORE
 *
 * Side effects: 4 storage locations are set;
 *
 * ASSUMES STORE_maxMul 		== 8
 * ASSUMES STORE_minMul 		== 9
 * ASSUMES STORE_multiplier 	== 10
 * ASSUMES STORE_mulRepeatCount == 11
 */
FDEF[], 22
#BEGIN
#PUSHOFF
/* stack: - */
#PUSH, 10, 4096, 9, 2048, 8, 8192, 11, 0
WS[]
WS[]
WS[]
WS[]
#END
#PUSHON
ENDF[]
#endif

#ifdef ENABLE_AUTO_GRIDDING
void ag_INIT_STORE( ag_DataType *hData )
{
	hData->storage[ STORE_multiplier ]		= 64*64;
	hData->storage[ STORE_minMul ]			= 32*64;
	hData->storage[ STORE_maxMul ]			= 128*64;
	hData->storage[ STORE_mulRepeatCount ]	= 0;
}
#else
/*
 *
 */
void ag_INIT_STORE( ag_DataType *hData )
{
	*hData->ttdata-- = 22;
	*hData->ttcode++ = tt_CALL;
}
#endif

#ifdef OLD
/*
 * Function: 23 / AG_CHECK_AND_TWEAK
 *
 * Function 14 takes 2 arguments
 * stack: cvtNumber, ptA => -
 *
 * ASSUMES STORE_maxMul 		== 8
 * ASSUMES STORE_minMul 		== 9
 * ASSUMES STORE_multiplier 	== 10
 * ASSUMES STORE_mulRepeatCount == 11
 * ASSUMES STORE_error			== 12
 * ASSUMES STORE_return			== 13
 * Side effects: Storage, and coordinate of point 'ptA';
 *
 */
FDEF[], 23
#BEGIN
#PUSHOFF		/* stack: cvtNumber, ptA 					*/
DUP[]			/* stack: cvtNumber, ptA, ptA 	 			*/
GC[N]			/* stack: cvtNumber, ptA, coordA 	 		*/
ROT[]			/* stack: ptA, coordA, cvtNumber 	 		*/
#PUSH, 26		/* stack: ptA, coordA, cvtNumber, 26		*/
CALL[]			/* stack: ptA, goal							*/
ROUND[Gr]		/* stack: ptA, Rgoal						*/
DUP[]			/* stack: ptA, Rgoal, Rgoal					*/

ROT[]			/* stack: Rgoal, Rgoal, ptA					*/
DUP[]			/* stack: Rgoal, Rgoal, ptA, ptA					*/
GC[N]			/* stack: Rgoal, Rgoal, ptA, coordA				*/
ROT[]			/* stack: Rgoal, ptA, coordA, Rgoal				*/
SUB[]			/* stack: Rgoal, ptA, coordA-Rgoal					*/
#PUSH, 12		/* stack: Rgoal, ptA, coordA-Rgoal, 12				*/
SWAP[]			/* stack: Rgoal, ptA, 12, coordA-Rgoal             */
WS[]			/* stack: Rgoal, ptA, 					            */
#PUSH, 14, 11, 12/* stack: Rgoal, ptA, (8+6), 11, 12				    */
RS[]			/* stack: Rgoal, ptA, (8+6), 11, error				    */
ROT[]			/* stack: Rgoal, ptA, 11, error, (8+6)				    */
ROT[]			/* stack: Rgoal, ptA, error, (8+6), 11			    	*/
RS[]			/* stack: Rgoal, ptA, error, (8+6), mulRepeatCount		*/
GT[]			/* stack: Rgoal, ptA, error, (8+6)>mulRepeatCount		*/
AND[]			/* stack: Rgoal, ptA, error && (8+6)>mulRepeatCount	*/

IF[]			/* stack: Rgoal, ptA								*/

POP[]			/* stack: Rgoal, -									*/
POP[]			/* stack: -											*/
#PUSH, 11		/* stack: 11										*/
RS[]			/* stack: mulRepeatCount							*/

IF[]			/* stack: - */
#PUSH, 0, 12		/* stack: 0, 12										*/
RS[]			/* stack: 0, error									*/
LT[]			/* stack: error>0									*/

IF[]
#PUSH, 8, 10	/* stack: 8, 10										*/
RS[]			/* stack: 8, multiplier								*/
WS[]			/* stack: - */
#PUSH, 10, 32, 10, 9 	/* stack: 10, 32, 10, 9 */
RS[] 					/* stack: 10, 32, 10, minMul */
SWAP[] 					/* stack: 10, 32, minMul, 10 */
RS[] 					/* stack: 10, 32, minMul, multiplier */
ADD[] 					/* stack: 10, 32, minMul+multiplier */
MUL[] 					/* stack: 10, (minMul+multiplier)/2 */
WS[] 					/* stack: 10, (minMul+multiplier)/2 */

ELSE[]			/* stack: - */
#PUSH, 9, 10	/* stack: 9, 10										*/
RS[]			/* stack: 9, multiplier								*/
WS[]			/* stack: - */
#PUSH, 10, 32, 10, 8 	/* stack: 10, 32, 10, 8 */
RS[] 					/* stack: 10, 32, 10, maxMul */
SWAP[] 					/* stack: 10, 32, maxMul, 10 */
RS[] 					/* stack: 10, 32, maxMul, multiplier */
ADD[] 					/* stack: 10, 32, minMul+multiplier */
MUL[] 					/* stack: 10, (minMul+multiplier)/2 */
WS[] 					/* stack: 10, (minMul+multiplier)/2 */

EIF[]			/* stack: - */

EIF[]			/* stack: - */
#PUSH, 13, 1, 11, 1, 11	/* stack: 13, 1, 11, 1, 11					*/
RS[]			/* stack: 13, 1, 11, 1, mulRepeatCount				*/
ADD[]			/* stack: 13, 1, 11, 1+mulRepeatCount				*/
WS[]			/* stack: 13, 1, 									*/
WS[]			/* stack: -	 										*/

ELSE[]			/* stack: Rgoal, ptA								*/

SWAP[]			/* stack: ptA, Rgoal								*/
SCFS[]			/* stack: -											*/
#PUSH, 13, 0, 22	/* stack: 13, 0, 22								*/
CALL[]			/* stack: 13, 0										*/
WS[]			/* stack: -											*/

EIF[]			/* -												*/

#END
#PUSHON
ENDF[]
#endif


#ifdef ENABLE_AUTO_GRIDDING

void AG_CHECK_AND_TWEAK( ag_DataType *hData, ag_ElementType* elem, tt_int16 doX, tt_int16 cvtNumber, tt_int16 ptA )
{
	tt_int32 coord, goal;				

	F26Dot6 *oz1, *z1;
	
	if ( doX ) {
		z1 = elem->x;
		oz1 = hData->ox;
	} else {
		z1 = elem->y;
		oz1 = hData->oy;
	}

	coord = z1[ptA];
	goal  = ag_ModifyHeightGoal( hData, cvtNumber, oz1[ptA] );
	goal += ag_pixelSize/2;
	goal &= ~(ag_pixelSize-1);

	hData->storage[ STORE_error ] = coord - goal;
	if ( hData->storage[ STORE_error ] != 0 && hData->storage[ STORE_mulRepeatCount ] < (8+6) ) {
		if ( hData->storage[ STORE_mulRepeatCount ] != 0 ) {
			if ( hData->storage[ STORE_error ] > 0 ) {
				hData->storage[ STORE_maxMul ]		= hData->storage[ STORE_multiplier ];
				hData->storage[ STORE_multiplier ]	= (hData->storage[ STORE_multiplier ] + hData->storage[ STORE_minMul ]) /  2;
			} else {
				hData->storage[ STORE_minMul ]		= hData->storage[ STORE_multiplier ];
				hData->storage[ STORE_multiplier ]	= (hData->storage[ STORE_multiplier ] + hData->storage[ STORE_maxMul ]) /  2;
			}
		}
		hData->storage[ STORE_mulRepeatCount ]++;
		hData->storage[ STORE_return ]	= 1; /***** We need to jump back! ***** *****/
	} else {
		z1[ptA] = goal;
		ag_INIT_STORE( hData );
		hData->storage[ STORE_return ]	= 0; /*****/
	}
}

#else
void AG_CHECK_AND_TWEAK( ag_DataType *hData, ag_ElementType* elem, tt_int16 doX, tt_int16 cvtNumber, tt_int16 ptA )
{
	hData; elem; doX;
	/* stack: cvtNumber, ptA 					*/
	*hData->ttdata-- = 23;
	*hData->ttdata-- = ptA;
	*hData->ttdata-- = cvtNumber;
	*hData->ttcode++ = tt_CALL;
}
#endif



#ifdef OLD
/*
 * Function: 24 / ag_ASSURE_AT_LEAST_EQUAL
 *
 * Function 14 takes 2 arguments
 * stack:  prev, point => -
 *
 * Side effects: may change the coordinate of point 'point';
 */
FDEF[], 24
#BEGIN
#PUSHOFF		/* stack: prev, point					*/
DUP[]			/* stack: prev, point, point			*/
ROT[]			/* stack: point, point, prev			*/
GC[N]			/* stack: point, point, coord_pr			*/
DUP[]			/* stack: point, point, coord_pr, coord_pr		*/
ROT[]			/* stack: point, coord_pr, coord_pr, point		*/
GC[N]			/* stack: point, coord_pr, coord_pr, coord_p		*/
GT[]			/* stack: point, coord_pr, coord_p < coord_pr	*/

IF[]			/* stack: point, coord_pr, -	*/

SCFS[]			/* stack: - */

ELSE[]			/* stack: point, coord_pr, -	*/

POP[]			/* stack: point,  	*/
POP[]			/* stack: */

EIF[]			/* stack: */

#END
#PUSHON
ENDF[]
#endif

#ifdef ENABLE_AUTO_GRIDDING
void ag_ASSURE_AT_LEAST_EQUAL( ag_DataType *UNUSED(hData), register ag_ElementType* elem, tt_int16 doX, tt_int16 prev, tt_int16 point )
{
	F26Dot6 *z1;
	
	z1 = doX ? elem->x : elem->y;
	if ( z1[point] < z1[prev] ) {
		z1[point] = z1[prev];
	}
}
#else
void ag_ASSURE_AT_LEAST_EQUAL( ag_DataType *hData, register ag_ElementType* UNUSED(elem), tt_int16 UNUSED(doX), tt_int16 prev, tt_int16 point )
{
	/* stack: prev, point					*/
	*hData->ttdata-- = 24;
	*hData->ttdata-- = point;
	*hData->ttdata-- = prev;
	*hData->ttcode++ = tt_CALL;
}
#endif

#ifdef OLD
/*
 * Function: 25 / ag_ASSURE_AT_MOST_EQUAL
 *
 * Function 14 takes 2 arguments
 * stack:  prev, point => -
 *
 * Side effects: may change the coordinate of point 'point';
 */
FDEF[], 25
#BEGIN
#PUSHOFF		/* stack: prev, point					*/
DUP[]			/* stack: prev, point, point			*/
ROT[]			/* stack: point, point, prev			*/
GC[N]			/* stack: point, point, coord_pr			*/
DUP[]			/* stack: point, point, coord_pr, coord_pr		*/
ROT[]			/* stack: point, coord_pr, coord_pr, point		*/
GC[N]			/* stack: point, coord_pr, coord_pr, coord_p		*/
LT[]			/* stack: point, coord_pr, coord_p > coord_pr	*/

IF[]			/* stack: point, coord_pr, -	*/

SCFS[]			/* stack: - */

ELSE[]			/* stack: point, coord_pr, -	*/

POP[]			/* stack: point,  	*/
POP[]			/* stack: */

EIF[]			/* stack: */

#END
#PUSHON
ENDF[]
#endif

#ifdef OLD
/*
 * Function: 27 / ag_ASSURE_AT_MOST_EQUAL2
 *
 * Function 27 takes 3 arguments
 * stack:  prev, point1, point2 => -
 *
 * Side effects: may change the coordinate of point 'point';
 */
FDEF[], 27
#BEGIN
#PUSHOFF		/* stack: prev, point1, point2			*/
ROT[]			/* stack: point2, point1, prev			*/
SWAP[]			/* stack: point2, prev, point1			*/
DUP[]			/* stack: point2, prev, point1, point1	*/
ROT[]			/* stack: point2, point1, point1, prev	*/
MD[N]			/* stack: point2, point1, diff 			*/
DUP[]			/* stack: point2, point1, diff, diff 	*/
#PUSH, 0		/* stack: point2, point1, diff, diff, 0 */
GT[]			/* stack: point2, point1, diff, diff>0	*/

IF[]			/* stack: point2, point1, diff, -		*/

DUP[]			/* stack: point2, point1, diff, diff	*/
ROT[]			/* stack: point2, diff, diff, point1	*/

DUP[]			/* stack: point2, diff, diff, point1, point1 		*/
GC[N]			/* stack: point2, diff, diff, point1, coord_pt1		*/
ROT[]			/* stack: point2, diff, point1, coord_pt1, diff		*/
SUB[]			/* stack: point2, diff, point1, coord_pt1-diff		*/
SCFS[]			/* stack: point2, diff,  */

SWAP[]			/* stack: diff, point2  			*/

DUP[]			/* stack: diff, point2, point2  	*/
GC[N]			/* stack: diff, point2, coord_pt2	*/
ROT[]			/* stack: point2, coord_pt2, diff	*/
SUB[]			/* stack: point2, coord_pt2- diff	*/
SCFS[]			/* stack: -							*/

ELSE[]			/* stack: point2, point1, diff, -	*/

POP[]			/* stack: point2, point1,			*/
POP[]			/* stack: point2,					*/
POP[]			/* stack:							*/

EIF[]			/* stack: */


#END
#PUSHON
ENDF[]
#endif

#ifdef ENABLE_AUTO_GRIDDING
void ag_ASSURE_AT_MOST_EQUAL( ag_DataType *UNUSED(hData), register ag_ElementType* elem, tt_int16 doX, tt_int16 prev, tt_int16 point )
{
	F26Dot6 *z1;
	
	z1 = doX ? elem->x : elem->y;
	if ( z1[point] > z1[prev] ) {
		z1[point] = z1[prev];
	}
}

void ag_ASSURE_AT_MOST_EQUAL2( ag_DataType *UNUSED(hData), register ag_ElementType* elem, tt_int16 doX, tt_int16 prev, tt_int16 point1, tt_int16 point2 )
{
	F26Dot6 *z1;
	
	z1 = doX ? elem->x : elem->y;
	if ( z1[point1] > z1[prev] ) {
		tt_int16 diff = (short)(z1[point1] - z1[prev]);
		
		z1[point1] -= diff;
		z1[point2] -= diff;
	}
}

#else
void ag_ASSURE_AT_MOST_EQUAL( ag_DataType *hData, register ag_ElementType* UNUSED(elem), tt_int16 UNUSED(doX), tt_int16 prev, tt_int16 point )
{
	/* stack: prev, point					*/
	*hData->ttdata-- = 25;
	*hData->ttdata-- = point;
	*hData->ttdata-- = prev;
	*hData->ttcode++ = tt_CALL;
}

void ag_ASSURE_AT_MOST_EQUAL2( ag_DataType *UNUSED(hData), register ag_ElementType* elem, tt_int16 doX, tt_int16 prev, tt_int16 point1, tt_int16 point2 )
{
	/* stack: prev, point1, point2					*/
	*hData->ttdata-- = 27;
	*hData->ttdata-- = point2;
	*hData->ttdata-- = point1;
	*hData->ttdata-- = prev;
	*hData->ttcode++ = tt_CALL;
}

#endif



#define PULL_HEIGHT (64+32)

#ifdef OLD
/*
 *
 * Function: 26/ag_ModifyHeightGoal
 *
 * Height Sweep function
 *
 *
 * Function 26 takes 2 args
 * stack:  value, keyCvt#
 *     =>
 * stack:  goal
 *
 * pull = 96
 *
 * if ( value > cvt(keyCvt#) ) { 
 * 		goal = max( cvt(keyCvt#), value-pull )
 * } else {
 * 		goal = min( cvt(keyCvt#), value+pull )
 * }
 *
 * Side effects: None
 */
FDEF[], 26
#BEGIN
#PUSHOFF			/* value, keyCvt#, 						*/
RCVT[]				/* value, cvt(keyCvt#)					*/
SWAP[]				/* cvt(keyCvt#), value					*/
DUP[]				/* cvt(keyCvt#), value, value			*/
#PUSH, 96, 4		/* cvt(keyCvt#), value, value, pull, 4	*/
CINDEX[]			/* cvt(keyCvt#), value, value, pull, cvt(keyCvt#) */
ROLL[]				/* cvt(keyCvt#), value, pull, cvt(keyCvt#), value	*/
LT[]				/* cvt(keyCvt#), value, pull, value > cvt(keyCvt#)	*/

IF[]				/* cvt(keyCvt#), value, pull			*/
	SUB[]			/* cvt(keyCvt#), value - pull			*/
	MAX[]			/* max(cvt(keyCvt#), value-pull)		*/
ELSE[]				/* cvt(keyCvt#), value, pull			*/
	ADD[]			/* cvt(keyCvt#), value + pull			*/
	MIN[]			/* min(cvt(keyCvt#), value+pull)		*/
EIF[]				/* unrounded-goal						*/
ROUND[Bl]			/* goal									*/
#END
#PUSHON
ENDF[]
#endif


#ifdef ENABLE_AUTO_GRIDDING
/* PULL_HEIGHT is set to a somewhat large value since the round heights undergo some 
 * intentional distorsion
 */
/*
 * Description:		Modifies current heigth Value towards the height goal,
 *					by at most the PULL_HEIGHT amount
 * 					if ( currentValue > goal ) { 
 * 						currentValue = max( goal, currentValue-pull )
 * 					} else {
 * 						currentValue = min( goal, currentValue+pull )
 * 					}
 * How used:		Call with the goal and the current value
 * Side Effects: 	None.
 * Return value: 	The modified height goal.
 */
F26Dot6 ag_ModifyHeightGoal( ag_DataType *hData, tt_int16 cvtNumber, F26Dot6 currentValue )
{
	F26Dot6 newGoal, goal;
	
	if ( cvtNumber == CVT_EXIT_MARKER ) return currentValue; /*****/	
	goal = hData->cvt[cvtNumber];
	if ( currentValue > goal ) { 
		/* newGoal = max( goal, currentValue-pull ) */
		newGoal = currentValue-PULL_HEIGHT;
		if ( goal > newGoal ) newGoal = goal;
	} else {
		/* newGoal = min( goal, currentValue+pull ) */
		newGoal = currentValue+PULL_HEIGHT;
		if ( goal < newGoal ) newGoal = goal;
	}
	return newGoal; /*****/
}
#else
F26Dot6 ag_ModifyHeightGoal( ag_DataType *hData, tt_int16 cvtNumber, F26Dot6 currentValue )
{
	*hData->ttdata-- = 26;
	/* value, keyCvt#, 						*/
	*hData->ttdata-- = cvtNumber;
	*hData->ttdata-- = (short)currentValue;
	*hData->ttcode++ = tt_CALL;
	return 0;
}
#endif


#ifdef ENABLE_AUTO_HINTING

/*
 * Description:		Move ptB relative to ptA.
 * 					Maps to the TrueType instruction MIRP if cvtNumber >= 0 or MDRP otherwise.
 * How used:		Call with a pointer to ag_DataType, cvtNumber, and all the 5 TrueType MDRP/MIRP boolean flags, and the two points
 *					The 2 color bits are combined into one input parameter "c1" here.
 * Side Effects: 	ttcode, ttdata, RP0, RP1, and RP2 in ag_DataType.
 * Return value: 	None.
 */
static void ag_MRP( ag_DataType *hData, short cvtNumber, short move, short minDist, short round, char c1, int ptA, int ptB )
{
	tt_uint8 value;
	
	ag_SRP0( hData, ptA );
	
	value = (tt_uint8)(cvtNumber >= 0 ? tt_MIRP_BASE : tt_MDRP_BASE);
	if ( c1 == 'B' ) {
		value += 1;
	} else if ( c1 == 'G' ) {
		value += 0;
	} else if ( c1 == 'W' ) {
		value += 2;
	} else {
		assert( false );
	}
	*hData->ttcode++ = (tt_uint8)(value + (move ? 0x10 : 0) + (minDist ? 0x08 : 0 ) + (round ? 0x04 : 0));
	if ( cvtNumber >= 0 ) {
		*hData->ttdata-- = cvtNumber;
	}
	*hData->ttdata-- = (short)ptB;
	hData->RP1 = hData->RP0;
	hData->RP2 = (short)ptB;
	if ( move ) {
		 hData->RP0 = (short)ptB;
	}
}

#endif /* ENABLE_AUTO_HINTING */

#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		Generates the TrueType instruction to push "numberofArgs" bytes onto the stack.
 * How used:		Call with a pointer to the data to be pushed onto the stack,
 *					number of Arguments and a pointer to memory area for the result.
 * Side Effects: 	Sets the memory pointed at by uCharP.
 * Return value: 	Number of bytes put into the memory area pointed at by uCharP.
 */
static int ag_BytePush( short *argStore, int numberofArgs, register unsigned char *uCharP )
{
	register int i;
	register int count = 0;

	assert( numberofArgs <= 255 );
	if ( numberofArgs <= 8 ) {
		uCharP[count++] = (tt_uint8)(tt_PUSHB_BASE + numberofArgs - 1);/* PUSHB */
	} else {
		uCharP[count++] = tt_NPUSHB;
		uCharP[count++] = (tt_uint8)numberofArgs;
	}
	for ( i = 0; i < numberofArgs; i++ ) {
		assert( argStore[i] >= 0 && argStore[i] <= 255 );
		uCharP[count++] = (tt_uint8)argStore[i];
	}
	return count; /*****/
}

/*
 * Description:		Generates the TrueType instruction to push "numberofArgs" shorts onto the stack.
 * How used:		Call with a pointer to the data to be pushed onto the stack,
 *					number of Arguments and a pointer to memory area for the result.
 * Side Effects: 	Sets the memory pointed at by uCharP.
 * Return value: 	Number of bytes put into the memory area pointed at by uCharP.
 */
static int ag_WordPush( short *argStore, int numberofArgs, register unsigned char *uCharP )
{
	register int i;
	register int count = 0;

	assert( numberofArgs <= 255 );
	if ( numberofArgs <= 8 ) {
		uCharP[count++] = (tt_uint8)(tt_PUSHW_BASE + numberofArgs - 1);/* PUSHW */
	} else {
		uCharP[count++] = tt_NPUSHW;
		uCharP[count++] = (tt_uint8)numberofArgs;
	}
	for ( i = 0; i < numberofArgs; i++ ) {
		uCharP[count++] = (char)((argStore[i] >> 8) & 0x00ff);
		uCharP[count++] = (char)(argStore[i] & 0x00ff);
	}
	return count; /*****/
}

/*
 * Description:		Returns the number of bytes that all do fit within [0,255] inclusively.
 * How used:		Call with an array of shorts,
 *					and the number of elements in the array.
 * Side Effects: 	None.
 * Return value: 	The number of bytes that all do fit within [0,255] inclusive.
 */
static int ag_ByteRunLength( register short *args, register int n )
{
	register int len;
	
	for ( len = 0; len < n && *args >= 0 && *args <= 255; args++ ) {
		len++;
	}
	return len; /*****/
}
#endif /* ENABLE_AUTO_HINTING */


#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		Emits an optimized burst of TrueType push statements.
 * How used:		Call with an array of shorts (this data will go onto the TrueType stack),
 *					and the number of elements in the array,
 *					and a pointer to where to put the TrueType code.
 * Side Effects: 	Fills in uCharP[].
 * Return value: 	Then number of bytes of TrueType code generated.
 */
static int ag_OptimizingPushArguments( short *argStore, int numberofArgs, register unsigned char *uCharP )
{
	register int i, count, argument;
	int argCount, n, byteRun, runLength;
	int doBytePush, limit;
	
	
	count = 0;
	for ( n = 0; numberofArgs > 0;  ) {
		argCount = numberofArgs > 255 ? 255 : numberofArgs;
		doBytePush = true;
		
		for ( runLength = 0, i = n; i < (n + argCount); i++ ) {
			argument = argStore[i];
			byteRun = ag_ByteRunLength( &argStore[i], n + argCount - i );
			/* if we have a run of bytes of length 3 (2 if first or last) or more it is more
			   optimal in terms of space to push them as bytes */
			limit = 3;
			if ( (runLength == 0) || ((byteRun + i) >= (n + argCount)) ) {
				limit = 2;
			}
			if ( byteRun >= limit ) {
				if ( runLength > 0 ) {
					argCount = runLength;
					doBytePush = false;
					break; /*****/
				} else {
					argCount = byteRun;
					doBytePush = true;
					break; /*****/
				}
			}
			if ( argument > 255 || argument < 0 ) doBytePush = false;
			runLength++;
		}
		numberofArgs -= argCount;
		
		count += (doBytePush ? ag_BytePush : ag_WordPush)( &argStore[n], argCount, &uCharP[count] );
		n += argCount;
	}
	return count; /*****/
}

#endif /* ENABLE_AUTO_HINTING */


#ifdef ENABLE_AUTO_GRIDDING

#define PULL_WEIGHT 42
/*
 * Description:		Modifies currentValue towards goal,
 *					by at most the PULL_WEIGHT amount
 * 					if ( currentValue > goal ) { 
 * 						currentValue = max( goal, currentValue-pull )
 * 					} else {
 * 						currentValue = min( goal, currentValue+pull )
 * 					}
 * How used:		Call with the goal and the current value
 * Side Effects: 	None.
 * Return value: 	The modified weight goal.
 */
F26Dot6 ag_ModifyWeightGoal( F26Dot6 goal, F26Dot6 currentValue )
{
	F26Dot6 newGoal;
	
	if ( currentValue > goal ) { 
		/* newGoal = max( goal, currentValue-pull ) */
		newGoal = currentValue-PULL_WEIGHT;
		if ( goal > newGoal ) newGoal = goal;
	} else {
		/* newGoal = min( goal, currentValue+pull ) */
		newGoal = currentValue+PULL_WEIGHT;
		if ( goal < newGoal ) newGoal = goal;
	}
	return newGoal; /*****/
}

#define CVT_PULL 32
/* The CVT_PULL should be somewhere between 0.5 and 1.0 pixels */
/* It makes sense to set it lower than PULL_WEIGHT ! */
/* For now set at 0.50 pixels */

/*
 * Description:		Modifies current cvt Value towards the cvt goal,
 *					by at most the CVT_PULL amount
 * How used:		Call with the goal and the current value
 * Side Effects: 	None.
 * Return value: 	The modified goal.
 */
static F26Dot6 ag_ModifyCvtGoal( F26Dot6 goal, F26Dot6 currentValue )
{
	F26Dot6 diff, newGoal = currentValue;
	
	diff = goal - currentValue;
	if ( diff < 0 ) diff = -diff;
	if ( diff <= CVT_PULL ) {
		newGoal = goal;
	}
	return newGoal; /*****/
}


#endif /* ENABLE_AUTO_GRIDDING */


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does nothing.
 * How used:		Call with pointer to ag_DataType
 * Side Effects: 	None.
 * Return value: 	None.
 */
void ag_SVTCA_X( ag_DataType * UNUSED(hData) )
{
}
#else
/*
 * Description:		Emits the TrueType SVTCA[X] instruction.
 * How used:		Call with pointer to ag_DataType
 * Side Effects: 	ttcode, inX, and inY in ag_DataType.
 * Return value: 	None.
 */
void ag_SVTCA_X( ag_DataType *hData )
{
	if ( !hData->inX ) {
		hData->inX = true;
		hData->inY = false;
		*hData->ttcode++ = tt_SVTCA_X;
	}
}
#endif

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does nothing.
 * How used:		Call with pointer to ag_DataType
 * Side Effects: 	None.
 * Return value: 	None.
 */
void ag_SVTCA_Y( ag_DataType * UNUSED(hData) )
{
}
#else
/*
 * Description:		Emits the TrueType SVTCA[Y] instruction.
 * How used:		Call with pointer to ag_DataType
 * Side Effects: 	ttcode, inX, and inY in ag_DataType.
 * Return value: 	None.
 */
void ag_SVTCA_Y( ag_DataType *hData )
{
	if ( !hData->inY ) {
		hData->inX = false;
		hData->inY = true;
		*hData->ttcode++ = tt_SVTCA_Y;
	}
}
#endif

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MDRP (Move Direct Relative Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the cvt #, and the 5 TrueType MDRP/MIRP boolean flags,
 *					and the two point numbers in ptA and ptB. Note that ptB is the point that moves.
 *					Use in the X-direction.
 * Side Effects: 	Modifies the x array contents for point 'ptB' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MDRPX( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short UNUSED(move), short minDist, short UNUSED(round), char UNUSED(c1), char UNUSED(c2), int ptA, int ptB )
{
	register F26Dot6 dist;
	register short sign;
	register F26Dot6 *x;

	/* Use them so Metrowerks C does not complain, MS C does not allow */
	/* commenting out parameter names in the function list */
	
	/* dist = hData->ox[ptB] - hData->ox[ptA]; */
	dist = hData->oox[ptB] - hData->oox[ptA];
	dist *= hData->xPixelsPerEm*ag_pixelSize;
	dist += hData->unitsPerEm>>1;
	dist /= hData->unitsPerEm;


	sign = 1;
	if ( dist < 0 ) {
		dist = -dist;
		sign = -1;
	}
	/* strat98 */
	if ( hData->strat98 ) {
		if ( dist < ag_pixelSize/2 && minDist ) {
			dist += dist;
			if ( dist > ag_pixelSize/2 ) dist = ag_pixelSize/2;
		}
	} else {
		if ( cvtNumber >= 0 ) {
			dist = ag_ModifyWeightGoal( hData->cvt[cvtNumber], dist );
		}
		dist += ag_pixelSize/2;
		dist &= ~(ag_pixelSize-1);
		if ( dist == 0 && minDist ) {
			dist = ag_pixelSize;
		}
	}
	x = elem->x;
	x[ptB] = x[ptA] + (dist * sign );
}
#else
/*
 * Description:		Calls the ag_MRP function().
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the cvt #, and the 5 TrueType MDRP/MIRP boolean flags,
 *					and the two point numbers in ptA and ptB. Note that ptB is the point that moves.
 *					Use in the X-direction.
 * Side Effects: 	same as ag_MRP.
 * Return value: 	None.
 */
void ag_MDRPX( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short move, short minDist, short round, char c1, char c2, int ptA, int ptB )
{
	assert( hData->inX );
	elem; c2;
	ag_MRP( hData, cvtNumber, move, minDist, round, c1, ptA, ptB );
}
#endif

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MDRP (Move Direct Relative Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the cvt #, and the 5 TrueType MDRP/MIRP boolean flags,
 *					and the two point numbers in ptA and ptB. Note that ptB is the point that moves.
 *					Use in the Y-direction.
 * Side Effects: 	Modifies the y array contents for point 'ptB' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MDRPY( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short move, short minDist, short round, char c1, char c2, int ptA, int ptB )
{
	register F26Dot6 dist, dist2;
	register short sign;
	register F26Dot6 *y;

	move = round = 0;
	c1 = c2 = 0;
	
	/* dist = hData->oy[ptB] - hData->oy[ptA]; */
	dist = hData->ooy[ptB] - hData->ooy[ptA];
	dist *= hData->yPixelsPerEm*ag_pixelSize;
	dist += hData->unitsPerEm>>1;
	dist /= hData->unitsPerEm;
	
	sign = 1;
	if ( dist < 0 ) {
		dist = -dist;
		sign = -1;
	}
	y = elem->y;
	/* strat98 */
	dist2 = dist;
	if ( cvtNumber >= 0 ) {
		dist2 = ag_ModifyWeightGoal( hData->cvt[cvtNumber], dist );
	}
	if ( hData->strat98 ) {
		if ( dist2 >= ag_pixelSize*36/64 && hData->fontType != ag_KANJI ) {
			dist = dist2;
			dist += ag_pixelSize/2;
			dist &= ~(ag_pixelSize-1);
		}
/* #define SUBTRACT_WEIGHT_TEST */
#ifdef SUBTRACT_WEIGHT_TEST
if ( dist > 64 && hData->fontType == ag_KANJI ) {
	dist2 = dist;
	dist -= 22;
	if ( dist < 64 ) dist = 64;
	dist = (dist + dist2) >> 1;
}
#endif
	} else {
		dist = dist2;
		dist += ag_pixelSize/2;
		dist &= ~(ag_pixelSize-1);
		if ( dist == 0 && minDist ) {
			dist = ag_pixelSize;
		}
	}
	y[ptB] = y[ptA] + (dist * sign );
}
#else
/*
 * Description:		Calls the ag_MRP function().
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the cvt #, and the 5 TrueType MDRP/MIRP boolean flags,
 *					and the two point numbers in ptA and ptB. Note that ptB is the point that moves.
 *					Use in the Y-direction.
 * Side Effects: 	same as ag_MRP.
 * Return value: 	None.
 */
void ag_MDRPY( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short move, short minDist, short round, char c1, char c2, int ptA, int ptB )
{
	assert( hData->inY );
	elem; c2;
	ag_MRP( hData, cvtNumber, move, minDist, round, c1, ptA, ptB );
}
#endif


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MDAP (Move Direct Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, and the point number.
 *					Use in the X-direction.
 * Side Effects: 	Modifies the x array contents for point 'point' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MDAPX( ag_DataType * UNUSED(hData), register ag_ElementType* elem, short round, int point )
{
	if ( round ) {
		register F26Dot6 *x = elem->x;
		x[point] += ag_pixelSize/2;
		x[point] &= ~(ag_pixelSize-1);
	}
}
#else
/*
 * Description:		Emits the TrueType instruction MDAP (Move Direct Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, and the point number.
 *					Use in the X-direction.
 * Side Effects: 	ttcode, ttdata, RP0, in RP1 in ag_DataType.
 * Return value: 	None.
 */
void ag_MDAPX( ag_DataType *hData, register ag_ElementType* elem, short round, int point )
{
	assert( hData->inX );
	elem;
	*hData->ttcode++ = (tt_uint8)(tt_MDAP_BASE + ( round ? 1 : 0 ));
	*hData->ttdata-- = (short)point;
	hData->RP0 = hData->RP1 = (short)point;
}
#endif /* ENABLE_AUTO_GRIDDING */


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MDAP (Move Direct Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, and the point number.
 *					Use in the Y-direction.
 * Side Effects: 	Modifies the y array contents for point 'point' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MDAPY( ag_DataType *UNUSED(hData), register ag_ElementType* elem, short round, int point )
{
	if ( round ) {
		register F26Dot6 *y = elem->y;
		y[point] += ag_pixelSize/2;
		y[point] &= ~(ag_pixelSize-1);
	}
}
#else
/*
 * Description:		Emits the TrueType instruction MDAP (Move Direct Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, and the point number.
 *					Use in the Y-direction.
 * Side Effects: 	ttcode, ttdata, RP0, in RP1 in ag_DataType.
 * Return value: 	None.
 */
void ag_MDAPY( ag_DataType *hData, register ag_ElementType* elem, short round, int point )
{
	assert( hData->inY );
	elem;
	*hData->ttcode++ = (tt_uint8)(tt_MDAP_BASE + ( round ? 1 : 0 ));
	*hData->ttdata-- = (short)point;
	hData->RP0 = hData->RP1 = (short)point;
}
#endif /* ENABLE_AUTO_GRIDDING */


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MIAP (Move Indirect Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, the point number, and the cvt number.
 *					Use in the X-direction.
 * Side Effects: 	Modifies the x array contents for point 'point' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MIAPX( ag_DataType *hData, register ag_ElementType* elem, short round, int point, short cvtNumber)
{
	if ( round ) {
		register F26Dot6 *x = elem->x;
		x[point] = ag_ModifyHeightGoal( hData, cvtNumber, x[point] );
		x[point] += ag_pixelSize/2;
		x[point] &= ~(ag_pixelSize-1);
	}
}
#else
/*
 * Description:		Emits the TrueType instruction MIAP (Move Indirect Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, the point number, and the cvt number.
 *					Use in the X-direction.
 * Side Effects: 	ttcode, ttdata, RP0, in RP1 in ag_DataType.
 * Return value: 	None.
 */
void ag_MIAPX( ag_DataType *hData, register ag_ElementType* elem, short round, int point, short cvtNumber)
{
	assert( hData->inX );
	elem;
	*hData->ttcode++ = (tt_uint8)(tt_MIAP_BASE + ( round ? 1 : 0 ));
	*hData->ttdata-- = cvtNumber;
	*hData->ttdata-- = (short)point;
	hData->RP0 = hData->RP1 = (short)point;
}
#endif /* ENABLE_AUTO_GRIDDING */

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction MIAP (Move Indirect Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, the point number, and the cvt number.
 *					Use in the Y-direction.
 * Side Effects: 	Modifies the y array contents for point 'point' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MIAPY( ag_DataType *hData, register ag_ElementType* elem, short round, int point, short cvtNumber )
{
	if ( round ) {
		register F26Dot6 *y = elem->y;
		y[point] = ag_ModifyHeightGoal( hData, cvtNumber, y[point] );
		y[point] += ag_pixelSize/2;
		y[point] &= ~(ag_pixelSize-1);
	}
}
#else
/*
 * Description:		Emits the TrueType instruction MIAP (Move Indirect Absolute Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, the round boolean, the point number, and the cvt number.
 *					Use in the Y-direction.
 * Side Effects: 	ttcode, ttdata, RP0, in RP1 in ag_DataType.
 * Return value: 	None.
 */
void ag_MIAPY( ag_DataType *hData, register ag_ElementType* elem, short round, int point, short cvtNumber)
{
	assert( hData->inY );
	elem;
	*hData->ttcode++ = (tt_uint8)(tt_MIAP_BASE + ( round ? 1 : 0 ));
	*hData->ttdata-- = cvtNumber;
	*hData->ttdata-- = (short)point;
	hData->RP0 = hData->RP1 = (short)point;
}
#endif /* ENABLE_AUTO_GRIDDING */

typedef void (*ag_IPPointFuncPtr) ( ag_DataType *hData, register ag_ElementType* elem, int A, int B, int C );

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction IP (Interpolate Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, and the 3 points (A,B,C).
 *					Use in the X-direction. Point 'B' is the point that moves.
 * Side Effects: 	Modifies the x array contents for point 'B' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_IPPointX( ag_DataType *UNUSED(hData), register ag_ElementType* elem, int A, int B, int C )
{
	tt_int32 oDist = elem->oox[C] - elem->oox[A];
	elem->x[B] = elem->x[A] + ((elem->oox[B] - elem->oox[A]) * (elem->x[C] - elem->x[A]) + (oDist>>1)) / oDist;

}
#else
/*
 * Description:		Emits the TrueType instruction IP (Interpolate Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, and the 3 points (A,B,C).
 *					Use in the X-direction. Point 'B' is the point that moves.
 * Side Effects: 	ttcode, ttdata, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_IPPointX( register ag_DataType *hData, register ag_ElementType* elem, int A, int B, int C )
{
	register int point;
	assert( hData->inX );
	elem;
	
	point = hData->nextPt[B];
	if ( hData->flags[point] & X_TOUCHED && hData->oox[point] == hData->oox[B] ) {
		return; /****** Done, IUP will take care of this */
	}
	point = hData->prevPt[B];
	if ( hData->flags[point] & X_TOUCHED && hData->oox[point] == hData->oox[B] ) {
		return; /****** Done, IUP will take care of this */
	}
	ag_IPPoint( hData, A, B, C );
}
#endif /* ENABLE_AUTO_GRIDDING */

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction IP (Interpolate Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, and the 3 points (A,B,C).
 *					Use in the Y-direction. Point 'B' is the point that moves.
 * Side Effects: 	Modifies the y array contents for point 'B' in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_IPPointY( ag_DataType *UNUSED(hData), register ag_ElementType* elem, int A, int B, int C )
{
	tt_int32 oDist = elem->ooy[C] - elem->ooy[A];
	elem->y[B] = elem->y[A] + ((elem->ooy[B] - elem->ooy[A]) * (elem->y[C] - elem->y[A]) + (oDist>>1))  / oDist;
}
#else
/*
 * Description:		Emits the TrueType instruction IP (Interpolate Point).
 * How used:		Call with pointers to ag_DataType, ag_ElementType, and the 3 points (A,B,C).
 *					Use in the Y-direction. Point 'B' is the point that moves.
 * Side Effects: 	ttcode, ttdata, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_IPPointY( register ag_DataType *hData, register ag_ElementType* elem, int A, int B, int C )
{
	register int point;
	assert( hData->inY );
	elem;
	point = hData->nextPt[B];
	if ( hData->flags[point] & Y_TOUCHED && hData->ooy[point] == hData->ooy[B] ) {
		return; /****** Done, IUP will take care of this */
	}
	point = hData->prevPt[B];
	if ( hData->flags[point] & Y_TOUCHED && hData->ooy[point] == hData->ooy[B] ) {
		return; /****** Done, IUP will take care of this */
	}
	ag_IPPoint( hData, A, B, C );
}
#endif /* ENABLE_AUTO_GRIDDING */


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction IUP[X] (Interpolate Untouched Points).
 *					Used for smoothing the outlines.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType.
 *					Use in the X-direction.
 * Side Effects: 	Modifies the x array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_XSmooth( register ag_DataType *hData, register ag_ElementType *elem )
{
	register int A, B;
	int ctr, lastPoint, startPt, endPt, beginPt;
	register tt_uint16 mask;
	register tt_int16 *nextPt = hData->nextPt;
	register tt_uint16 *flags = hData->flags;
	tt_int32 oldStartX, newstartX, multiplier, divider;

	mask = X_TOUCHED;
	for ( ctr = 0; ctr < elem->contourCount; ctr++ ) {
		lastPoint = elem->ep[ ctr ];
		if ( lastPoint <= elem->sp[ ctr ] ) continue; /*****/
		B = elem->sp[ ctr ];
      	while ( !(flags[B] & mask) && B <= lastPoint)
        	B++;
      	if ( B > lastPoint ) continue; /*****/
    
      	beginPt = B;
      	for (;;) {
	    	for ( B = nextPt[startPt = B]; !(flags[B] & mask); ) {
	    		B = nextPt[B];
	    	}
	    	endPt = B;
	    	
	    	oldStartX = hData->ox[startPt];
	    	newstartX = elem->x[startPt];
	    	divider = oldStartX - hData->ox[endPt];
			if ( divider == 0 ) {
				/* multiplier = 0; divider = 1; */
				/* Just shift */
				divider = newstartX - oldStartX;
		    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
		    		elem->x[A] += divider;
		    	}
			} else {
				multiplier = newstartX - elem->x[endPt];
				if ( multiplier > 8192 || multiplier < -8192 ) {
					multiplier = util_FixDiv( multiplier, divider );
			    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
			    		elem->x[A] = newstartX + util_FixMul( hData->ox[A] - oldStartX, multiplier );
			    	}
				} else {
			    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
			    		elem->x[A] = newstartX + (((hData->ox[A] - oldStartX) * multiplier) / divider);
			    	}
		    	}
			}
	    	if ( B == beginPt) break;/*****/
	    }
	}
}
#else
/*
 * Description:		Emits the TrueType instruction IUP[X] (Interpolate Untouched Points).
 *					Used for smoothing the outlines.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType.
 *					Use in the X-direction.
 * Side Effects: 	ttcode in ag_DataType.
 * Return value: 	None.
 */
void ag_XSmooth( register ag_DataType *hData, ag_ElementType *elem )
{
	elem;
	*hData->ttcode++ = tt_IUPX;
}
#endif /* ENABLE_AUTO_GRIDDING */

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of the TrueType instruction IUP[Y] (Interpolate Untouched Points).
 *					Used for smoothing the outlines.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType.
 *					Use in the Y-direction.
 * Side Effects: 	Modifies the y array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_YSmooth( register ag_DataType *hData, register ag_ElementType *elem )
{
	register int A, B;
	int ctr, lastPoint, startPt, endPt, beginPt;
	register tt_uint16 mask;
	register tt_int16 *nextPt = hData->nextPt;
	register tt_uint16 *flags = hData->flags;
	tt_int32 oldStartY, newstartY, multiplier, divider;

	mask = Y_TOUCHED;
	for ( ctr = 0; ctr < elem->contourCount; ctr++ ) {
		lastPoint = elem->ep[ ctr ];
		if ( lastPoint <= elem->sp[ ctr ] ) continue; /*****/
		B = elem->sp[ ctr ];
      	while ( !(flags[B] & mask) && B <= lastPoint)
        	B++;
      	if ( B > lastPoint ) continue; /*****/
    
      	beginPt = B;
      	for (;;) {
	    	for ( B = nextPt[startPt = B]; !(flags[B] & mask); ) {
	    		B = nextPt[B];
	    	}
	    	endPt = B;
	    	
	    	oldStartY = hData->oy[startPt];
	    	newstartY = elem->y[startPt];
	    	divider = oldStartY - hData->oy[endPt];
			if ( divider == 0 ) {
				/* multiplier = 0; divider = 1; */
				/* Just shift */
				divider = newstartY - oldStartY;
		    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
		    		elem->y[A] += divider;
		    	}
			} else {
				multiplier = newstartY - elem->y[endPt];
				if ( multiplier > 8192 || multiplier < -8192 ) {
					multiplier = util_FixDiv( multiplier, divider );
			    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
			    		elem->y[A] = newstartY + util_FixMul( hData->oy[A] - oldStartY, multiplier );
			    	}
				} else {
			    	for ( A = nextPt[startPt]; A != endPt; A = nextPt[A] ) {
			    		elem->y[A] = newstartY + (((hData->oy[A] - oldStartY) * multiplier) / divider);
			    	}
		    	}
			}
	    	if ( B == beginPt) break;/*****/
	    }
	}
}
#else
/*
 * Description:		Emits the TrueType instruction IUP[Y] (Interpolate Untouched Points).
 *					Used for smoothing the outlines.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType.
 *					Use in the Y-direction.
 * Side Effects: 	ttcode in ag_DataType.
 * Return value: 	None.
 */
void ag_YSmooth( register ag_DataType *hData, ag_ElementType *elem )
{
	elem;
	*hData->ttcode++ = tt_IUPY;
}
#endif /* ENABLE_AUTO_GRIDDING */

/*
 * Function 9 and 10 info
 *
 * Here they are called:
 * ag_MoveDirectRelativePointInPositiveDirection()
 * and
 * ag_MoveDirectRelativePointInNegativeDirection()
 *
 * They both represent single directional links. This means they only move one point.
 *
 *
 * Algorithm:		Determine the distance we are going to create between from and to.
 *					See ag_ModifyWeightGoal() for more info on how we determine the distance.
 *					Sets the coordinate of the "to" point equal to the coordinate of
 *					the "from" point plus the distance we are creating.
 *
 * Philosophy behind the algorithm and what we are trying to accomplish:
 *					We are controling distances, by regularizing them somewhat.
 *					See ag_ModifyWeightGoal() for more info on how we determine the distance.
 *					This gives an even look to the font without distorting it too much.
 *					A big part of hinting/gridding is "regularization".
 *
 *
 * Use ag_MoveDirectRelativePointInPositiveDirection() if the coordinate of the "to"
 * point is greater" than the coordinate of the "from" point. Otherwise use,
 * ag_MoveDirectRelativePointInNegativeDirection(). The two functions are identical
 * with this only difference (positive or negative distance).
 */


/*
 * Function: 9
 *
 * stack: keyCvt#, ptA, ptB => -
 *
 * This is to controls distances where only ptB will move
 * POSITIVE MOVE (ONESIDED)
 */
#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of StingRay function call 9.
 *					Used for a single directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the y direction.
 * Side Effects: 	Modifies the x or y array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MoveDirectRelativePointInPositiveDirection( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	( doX ? ag_MDRPX : ag_MDRPY )( hData, elem, cvtNumber, false, true, true, 'G', 'r', from, to );
}
#else
/*
 * Description:		Emits the TrueType instructions for invoking StingRay function call 9.
 *					Used for a single directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the y direction.
 * Side Effects: 	ttcode, ttdata, RP0, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_MoveDirectRelativePointInPositiveDirection( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	assert( doX ? hData->inX : hData->inY );
	elem; doX;
	*hData->ttdata-- = 9;
	*hData->ttdata-- = (short)to;
	*hData->ttdata-- = (short)from;
	*hData->ttdata-- = cvtNumber;
	*hData->ttcode++ = tt_CALL;
	
	/* Side Effects of the call */
	hData->RP0 = (short)from;
	hData->RP1 = hData->RP0; /* MSIRP */
	hData->RP2 = (short)to;
}
#endif /* ENABLE_AUTO_GRIDDING */

/*
 * Function: 10
 *
 * stack: keyCvt#, ptA, ptB => -
 *
 * This is to controls distances where only ptB will move
 *
 * NEGATIVE MOVE (ONESIDED)
 */
#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of StingRay function call 10.
 *					Used for a single directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the Y direction.
 * Side Effects: 	Modifies the x or y array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_MoveDirectRelativePointInNegativeDirection( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	( doX ? ag_MDRPX : ag_MDRPY )( hData, elem, cvtNumber, false, true, true, 'G', 'r', from, to );
}
#else
/*
 * Description:		Emits the TrueType instructions for invoking StingRay function call 10.
 *					Used for a single directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the y direction.
 * Side Effects: 	ttcode, ttdata, RP0, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_MoveDirectRelativePointInNegativeDirection( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	assert( doX ? hData->inX : hData->inY );
	elem; doX;
	*hData->ttdata-- = 10;
	*hData->ttdata-- = (short)to;
	*hData->ttdata-- = (short)from;
	*hData->ttdata-- = cvtNumber;
	*hData->ttcode++ = tt_CALL;

	/* Side Effects of the call */
	hData->RP0 = (short)from;
	hData->RP1 = hData->RP0; /* MSIRP */
	hData->RP2 = (short)to;
}
#endif /* ENABLE_AUTO_GRIDDING */

/*
 * Function 14 and 13 info
 *
 * Here they are called:
 * ag_BiDirectionalLink()
 * and
 * ag_BiDirectionalLinkWithCvt()
 *
 * They both represent bi directional links. This means they move two points.
 *
 *
 * Algorithm:		Determine the distance we are going to create between from and to.
 *					See ag_ModifyWeightGoal() for more info on how we determine the distance.
 *					How does it move the two points so that the distance between them
 *					becomes equal to the distance we are creating ???
 *					First we determine the goal (==the distance we are creating)
 *				    which is equal to the return value of ag_ModifyWeightGoal()
 *					rounded to the closest pixel, while ensuring it does not
 *					drop below the minimum distance.
 *						
 *					Next we set delta variable
 *					delta = (short)((goal-distAB)/2);
 *					This is the difference between what we want and what we have divided by two.
 *					
 *					Next we subtract the delta from the "from" coordinate and then we
 *					we round the from coordinate to the closest pixel boundary.
 *					Finally we just set the "to" coordinate" equal to the
 *					new coordinate value for the "from" point plus the goal!
 *
 * Philosophy behind the algorithm and what we are trying to accomplish:
 *					We are controling distances, by regularizing them somewhat.
 *					See ag_ModifyWeightGoal() for more info on how we determine the distance.
 *					This gives an even look to the font without distorting it too much.
 *					A big part of hinting/gridding is this "regularization".
 *					Moving both points allows us to distribute the distortion
 *					onto both points. This is more gentle than a single directional
 *					link, which puts the distorion on one point only.
 *
 *
 * Use ag_BiDirectionalLinkWithCvt() if you have a cvt number, otherwise us
 * ag_BiDirectionalLink(). The two functions are identical
 * with this only difference (cvt or no cvt).
 */



/*
 * Function: 14
 *
 *
 * stack: minDist, ptA, ptB => -
 *
 * This is to controls distances where both ptA and ptB will move
 *
 * This function does not pull towards a cvt value.
 */
#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of StingRay function call 14 AND 13.
 *					Set cvtNumber < 0 for function 14, and >= 0 for function 13.
 *					Used for a bi-directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, minimum distance, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the Y direction.
 * Side Effects: 	Modifies the x or y array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_BiDirectionalLink( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short minDist, int from, int to, short doX )
{
	register F26Dot6 distAB, goal, delta, *coord;
	
	if ( doX ) {
		distAB 	= (short)(hData->ox[to] - hData->ox[from]);
		coord	= elem->x;
	} else {
		distAB = (short)(hData->oy[to] - hData->oy[from]);
		coord	= elem->y;
	}
	assert( distAB >= 0 );
	goal  = distAB;
	if ( cvtNumber >= 0 ) {
		goal = ag_ModifyWeightGoal( hData->cvt[cvtNumber], distAB );
	}
	goal += ag_pixelSize/2;
	goal &= ~(ag_pixelSize-1);
	if ( goal < minDist ) goal = minDist;
	/*delta = (short)((goal-distAB)>>1); -13>>1 -> -7, not OK */
	delta = (short)((goal-distAB)/2); /* -13/2 == -6, OK */

	coord[from] -= delta;
	coord[from]  += ag_pixelSize/2; /* Grid It */
	coord[from]  &= ~(ag_pixelSize-1);
	coord[to]    = coord[from] + goal;
}
#else
/*
 * Description:		Emits the TrueType instructions for invoking StingRay function call 14.
 *					Used for a bi-directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, -1 for cvt #, minimum distance, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the Y direction.
 * Side Effects: 	ttcode, ttdata, RP0, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_BiDirectionalLink( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, short minDist, int from, int to, short doX )
{
	assert( doX ? hData->inX : hData->inY );
	assert( cvtNumber < 0 );
	elem; cvtNumber; doX;
	*hData->ttdata-- = 14;
	*hData->ttdata-- = (short)to;
	*hData->ttdata-- = (short)from;
	*hData->ttdata-- = (short)(minDist ? 64 : 0); /* minDist */
	*hData->ttcode++ = tt_CALL;

	/* Side Effects of the call */
	hData->RP0 = (short)from;
	hData->RP1 = hData->RP0; /* MSIRP */
	hData->RP2 = (short)to;
}
#endif /* ENABLE_AUTO_GRIDDING */

/*
 * Function: 13
 *
 *
 * stack: keyCvt#, ptA, ptB => -
 *
 * This is to controls distances where both ptA and ptB will move
 *
 *
 */
#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Does the auto-grid equivalent of StingRay function call 13.
 *					Used for a bi-directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the Y direction.
 * Side Effects: 	Modifies the x or y array contents in the ag_DataType data structure.
 * Return value: 	None.
 */
void ag_BiDirectionalLinkWithCvt( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	ag_BiDirectionalLink( hData, elem, cvtNumber, ag_pixelSize, from, to, doX );	
}
#else
/*
 * Description:		Emits the TrueType instructions for invoking StingRay function call 13.
 *					Used for a bi-directional link.
 * How used:		Call with pointers to ag_DataType, and ag_ElementType, the cvt number, the two points and the doX boolean.
 *					If doX is true we work in the X direction otherwise we work in the Y direction.
 * Side Effects: 	ttcode, ttdata, RP0, RP1, in RP2 in ag_DataType.
 * Return value: 	None.
 */
void ag_BiDirectionalLinkWithCvt( ag_DataType *hData, register ag_ElementType* elem, short cvtNumber, int from, int to, short doX )
{
	assert( doX ? hData->inX : hData->inY );
	elem; doX;
	*hData->ttdata-- = 13;
	*hData->ttdata-- = (short)to;
	*hData->ttdata-- = (short)from;
	*hData->ttdata-- = cvtNumber;
	*hData->ttcode++ = tt_CALL;

	/* Side Effects of the call */
	hData->RP0 = (short)from;
	hData->RP1 = hData->RP0; /* MSIRP */
	hData->RP2 = (short)to;
}
#endif /* ENABLE_AUTO_GRIDDING */


/*
 * Description:		Given a distance and the direction the function returns the best matching
 *					cvt number, or -1 if no close enough match was found.
 * How used:		Call with pointer to ag_DataType, the doX, doY, doD booleans, and the distance in FUnits.
 * Side Effects: 	None.
 * Return value: 	the cvt number or -1.
 */
short ag_GetCvtNumber( ag_DataType *hData, short doX, short doY, short doD, tt_int32 dist )
{
	register F26Dot6 delta, minDelta;
	register int i, best_i = -1;
	tt_int16 cvtNumber, cvtNumberBase;
	tt_int16 maxW, minW;
	
	doD 	  = false;
	minDelta  = 0x7fff;
	cvtNumber = -1;
	if ( doX ) {
		cvtNumberBase = GLOBALNORMSTROKEINDEX;
		for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
			if ( hData->gData.xWeight[i] ) {
				maxW = (short)(hData->gData.xWeight[i] + (hData->gData.xWeight[i]>>1)); 
				minW = (short)(hData->gData.xWeight[i] - (hData->gData.xWeight[i]>>1)); 
				delta = (short)(hData->gData.xWeight[i] - dist);
				if ( delta < 0 ) delta = (short)-delta;
				if ( delta < minDelta && dist >= minW && dist <= maxW ) {
					minDelta = delta;
					best_i = i;
				}
			}
		}
		if ( best_i >= 0 ) {
			cvtNumber = (short)(cvtNumberBase + best_i);
		}
	} else if ( doY ) {
	 	cvtNumberBase = GLOBALNORMSTROKEINDEX + ag_MAXWEIGHTS;
		for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
			if ( hData->gData.yWeight[i] ) {
				maxW = (short)(hData->gData.yWeight[i] + (hData->gData.yWeight[i]>>1)); 
				minW = (short)(hData->gData.yWeight[i] - (hData->gData.yWeight[i]>>1)); 
				delta = (short)(hData->gData.yWeight[i] - dist);
				if ( delta < 0 ) delta = (short)-delta;
				if ( delta < minDelta && dist >= minW && dist <= maxW ) {
					minDelta = delta;
					best_i = i;
				}
			}
		}
		if ( best_i >= 0 ) {
			cvtNumber = (short)(cvtNumberBase + best_i);
		}
	}
	return cvtNumber; /*****/
}


/*
 * Description:		The function returns the maximum cvt weight for the x direction.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	None.
 * Return value: 	The maximum weight in FUnits.
 */
short ag_GetXMaxCvtVal( ag_DataType *hData )
{
	register int i;
	register short maxVal = 0;
	
	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		if ( hData->gData.xWeight[i] > maxVal ) maxVal = hData->gData.xWeight[i];
	}
	return maxVal; /*****/
}

/*
 * Description:		The function returns the maximum cvt weight for the y direction.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	None.
 * Return value: 	The maximum weight in FUnits.
 */
short ag_GetYMaxCvtVal( ag_DataType *hData )
{
	register int i;
	register short maxVal = 0;
	
	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		if ( hData->gData.yWeight[i] > maxVal ) maxVal = hData->gData.yWeight[i];
	}
	return maxVal; /*****/
}


/*
 * Description:		Finds links (== pseudo-stems).
 *
 * Algorithm:		Loops through all contours and all points in the contours.		
 *					A height link is basically recorded based on the HEIGHT bit
 *					in the hData->flags array.
 *					A stem link is recorded if the current point has
 *					a point pair, and if that other point has a point pair
 *					equal to the current point.
 *					
 * Philosophy behind the algorithm and what we are trying to accomplish:
 *					Basically we are just recording the heights, and also
 *					finding stems representing black features.
 *					Why do we require the double point pairs going
 *					both ways in this algorithm? It seemed that this property held true
 *					for most black features that we are looking for.
 *					Deeper reasons have been lost in the mist of time.
 *					
 *					
 * How used:		Call with pointers to ag_DataType, tp_LinkType, and an integer.
 * Side Effects: 	Modifies the links array and sets *linkCount to the number of links found.
 * Return value: 	None.
 */
static void ag_FindLinks( ag_DataType *hData, tp_LinkType *links, int *linkCount )
{
	register int tLinkCount, i;
	int ctr, B, lastPoint, Bf, Bb;
	int doF, doB;
	int change;
	
	/* Initialize the link count to zero */
	tLinkCount = 0;
	/* Loop through all contours */
	for ( ctr = 0; ctr < hData->numberOfContours && tLinkCount < hData->maxLinks; ctr++ ) {
		lastPoint = hData->endPoint[ ctr ];
		if ( lastPoint <= hData->startPoint[ ctr ] ) continue; /*****/
		
		/* Loop through all points in this contour */
	    for ( B = hData->startPoint[ ctr ]; B <= lastPoint; B++ ) {
			doF = doB = false; /* Initialize doF and doB to false */
			Bf = hData->forwardAngleOrthogonalPair[B];
			if ( Bf >= 0 ) {
				doF   = true; /* Set doF true if there is a forward pair */
			} 
			Bb = hData->backwardAngleOrthogonalPair[B];
			if ( Bb >= 0 ) {
				doB   = true; /* Set doB true if there is a backward pair */
			}
			/* If nothing to do in either direction then bail out */
	    	if ( !doB && !doF ) continue; /*****/
	    	
	    	/* Record heights as links */
	    	if ( hData->flags[B] & HEIGHT ) {
	    		links[tLinkCount].type = INC_HEIGHT;
	    		links[tLinkCount].priority = 1;
	    		links[tLinkCount].from = (short)B; /* Set both the from and to points to B */
	    		links[tLinkCount].to   = (short)B; /* Since there is only one point of relevance for heights. */
	    		tLinkCount++;
	    	}
	    	/* Enter if-block if doF is true */
	    	if ( doF ) {
	    		/* if either the forward point pair for Bf or the backward point pair for Bf
	    		   is equal to B then we have a link from Bf to B !!! */
	    		if ( hData->forwardAngleOrthogonalPair[Bf] == B || hData->backwardAngleOrthogonalPair[Bf] == B ) {
		    		links[tLinkCount].type = INC_LINK;
		    		links[tLinkCount].priority  = 5;
		    		links[tLinkCount].forwardTo = true;
		    		links[tLinkCount].from = (short)Bf;
		    		links[tLinkCount].to   = (short)B;
		    		tLinkCount++;
	    		}
	    	}
	    	/* Enter if-block if (doB AND ( Bb != Bf OR we have more than a 30 degree
	    	   directional change at point B ))
	    	 */
	    	if ( doB && (Bb != Bf || ( SMALLFRACVECMUL(hData->cos_f[B], hData->cos_b[B], hData->sin_f[B], hData->sin_b[B]) <= ag_COS_30_DEG )) ) {
	    		/* if either the forward point pair for Bb or the backward point pair for Bb
	    		   is equal to B then we have a link from Bb to B !!! */
	    		if ( hData->forwardAngleOrthogonalPair[Bb] == B || hData->backwardAngleOrthogonalPair[Bb] == B ) {
		    		links[tLinkCount].type = INC_LINK;
		    		links[tLinkCount].priority = 5;
		    		links[tLinkCount].forwardTo = false;
		    		links[tLinkCount].from = (short)Bb;
		    		links[tLinkCount].to   = (short)B;
		    		tLinkCount++;
		    		if ( tLinkCount >= hData->maxLinks ) break; /******/
	    		}
	    	}
	    }
	}
	/* Determine Directions */
	for ( i = 0; i < tLinkCount; i++ ) {
		SMALLFRAC dx, dy;
		int to = links[i].to;

		/* Get dx,dy for the to point */
		if ( links[i].forwardTo ) {
			dx = hData->cos_f[to];
			dy = hData->sin_f[to];
		} else {
			dx = hData->cos_b[to];
			dy = hData->sin_b[to];
		}
		/* We consider us as being aligned with an axis if we
		   are within 15 degrees of it */
		if ( dx > ag_COS_15_DEG || dx < -ag_COS_15_DEG ) {
			links[i].direction = INC_YDIR;
		} else if ( dy > ag_COS_15_DEG || dy < -ag_COS_15_DEG ) {
			links[i].direction = INC_XDIR;
		} else {
			links[i].direction = 0xff;
		}
	}
	
	/* Sort (slow bubble sort) */
	/* It's slow but it is not a bottle neck */
	B = tLinkCount-1;
	/* Keep looping as long as we are not sorted */
	for ( change = true; change; ) {
		change = false;
		for ( i = 0; i < B; i++ ) {
			/* If wrong order then swap */
			if ( links[i].priority > links[i+1].priority ) {
				tp_LinkType tmp;
				change = true;
				/* swap */
				memcpy( (char *)&tmp,			(char *)&links[i],		sizeof( tp_LinkType ) );
				memcpy( (char *)&links[i],		(char *)&links[i+1],	sizeof( tp_LinkType ) );
				memcpy( (char *)&links[i+1],	(char *)&tmp,			sizeof( tp_LinkType ) );
			}
		}
	}
	/* Set *linkCount */
	*linkCount = (short)tLinkCount;
}

/*
 * Description:		Gets all the stems from the links array, which has to be set by a prior call to ag_FindLinks().
 *
 * Algorithm:		This routine is very simple and just copies x and y distances
 *					from the hData->links data structure
 *					representing black features into separate x and y distance arrays.
 *
 * Philosophy behind the algorithm and what we are trying to accomplish:
 *					We are just mapping x and y "stems" from the internal
 *					hData->links data structure into a simpler array data
 *					structure.
 *
 * How used:		Call with pointers to ag_DataType, tp_LinkType, the number of links, 
 *					a pointer to a pointer to a short for x-Distances, and a pointer to a tt_int32 for the number of x-stems.
 *					a pointer to a pointer to a short for y-Distances, and a pointer to a tt_int32 for the number of y-stems.
 *					The caller HAS TO deallocate *xDistPtr AND *yDistPtr if they are not equal to NULL.
 * Side Effects: 	Modifies  *xDistPtr[], *xCountPtr, *yDistPtr[], *yCountPtr
 * Return value: 	0 if no error, -1 if an error was encountered.
 */
static int ag_GetStems( register ag_DataType *hData, ag_ElementType *e, short **xDistPtr, tt_int32 *xCountPtr, short **yDistPtr, tt_int32 *yCountPtr )
{
	register int i;
	/* Heuristic: stem-thickness is < hData->unitsPerEm/4 */
	/* changed to unitsPerEm/3 June 11, 1997 due to TS Logan and TS Manchester */
	short *xDist, *yDist, tmp, limit = (short)(hData->unitsPerEm/3+1); 
	tt_int32 xDistCount = 0, yDistCount = 0; /* Initialize */
	int returnCode = 0; /* Initialize */
		
	/* Loop through all links, and count x and y links
	   so that we know how much memory to allocate below */
	for ( i = 0; i < hData->linkCount; i++ ) {
		if ( hData->links[i].type != INC_LINK ) continue; /*****/
		if ( hData->links[i].direction == INC_XDIR ) {
			tmp = (short)(e->oox[ hData->links[i].to ] - e->oox[ hData->links[i].from ]);
			if ( tmp < 0 ) tmp = (short)-tmp; /* Take the absolute */
			if ( tmp <= limit ) { /* See if less than the limit */
				xDistCount++; /* count x links */
			}
		} else if ( hData->links[i].direction == INC_YDIR ) {
			tmp = (short)(e->ooy[ hData->links[i].to ] - e->ooy[ hData->links[i].from ]);
			if ( tmp < 0 ) tmp = (short)-tmp;  /* Take the absolute */
			if ( tmp <= limit ) { /* See if less than the limit */
				yDistCount++; /* count y links */
			}
		}
	}
	/* Allocate the memory for the XDist and yDist arrays */
	xDist = (short *)tsi_AllocMem( hData->mem, (xDistCount + 1) * sizeof( short ) ); /* We do + 1 just to ensure that we do not get a non null pointer. */
	yDist = (short *)tsi_AllocMem( hData->mem, (yDistCount + 1) * sizeof( short ) ); /* This is important in case we have a zero count in one direction but not the other. */
	xDistCount = yDistCount = 0;
	if ( xDist != NULL && yDist != NULL ) {
		/* OK, we got the memory */
		/* Loop through all the links again
		   but this time record x and y distances
		   in the XDist and yDist arrays */
		for ( i = 0; i < hData->linkCount; i++ ) {
			if ( hData->links[i].type != INC_LINK ) continue; /*****/
			if ( hData->links[i].direction == INC_XDIR ) {
				tmp = (short)(e->oox[ hData->links[i].to ] - e->oox[ hData->links[i].from ]);
				if ( tmp < 0 ) tmp = (short)-tmp;  /* Take the absolute */
				if ( tmp <= limit ) { /* See if less than the limit */
					xDist[ xDistCount++ ] = tmp; /* Record an x distance */
				}
			} else if ( hData->links[i].direction == INC_YDIR ) {
				tmp = (short)(e->ooy[ hData->links[i].to ] - e->ooy[ hData->links[i].from ]);
				if ( tmp < 0 ) tmp = (short)-tmp;  /* Take the absolute */
				if ( tmp <= limit ) { /* See if less than the limit */
					yDist[ yDistCount++ ] = tmp; /* Record a y distance */
				}
			}
		}
	} else {
		/* Error */
		returnCode = -1;
		tsi_DeAllocMem( hData->mem, (char *)xDist ); xDist = NULL;
		tsi_DeAllocMem( hData->mem, (char *)yDist ); yDist = NULL;
	}
	*xDistPtr = xDist; *xCountPtr = xDistCount;
	*yDistPtr = yDist; *yCountPtr = yDistCount;
	return returnCode; /*****/
}


#ifdef ENABLE_AUTO_HINTING
/* These two constants are used by ag_CheckArrayBounds */
#define CODE_MARGIN  1024
#define DATA_MARGIN  512

/*
 * Description:		Ensures that we do not run out of space in the
 *					ttCodeBase and ttDataBase arrays in ag_DataType.
 *					It reallocates them if we are within CODE_MARGIN 
 *					or DATA_MARGIN of running over the end of the arrays.
 * How used:		Call with pointer to ag_DataType.
 * Side Effects: 	ttCodeBase, ttcode, ttDataBase, ttdata in ag_DataType.
 * Return value: 	-1 if an error was encountered, and 0 otherwise.
 */
int ag_CheckArrayBounds( ag_DataType *hData )
{
	tt_int32 n, thisMargin;
	int errorCode = 0;
	
	/* Check the hData->ttCodeBase array */
	n = hData->ttcode - hData->ttCodeBase;
	thisMargin = hData->ttCodeBaseMaxLength - n;
	assert( thisMargin >= 0 );
	if ( thisMargin < CODE_MARGIN ) {
		char *newP;
		/* printf("Increasing ttCodeBase\n"); */
		hData->ttCodeBaseMaxLength = hData->ttCodeBaseMaxLength + 2 * CODE_MARGIN;
		newP = (char *)tsi_AllocMem( hData->mem, hData->ttCodeBaseMaxLength );
		if ( newP == NULL ) return -1; /***** Error *****/
		memcpy( newP, hData->ttCodeBase, n );
		tsi_DeAllocMem( hData->mem, (char *)hData->ttCodeBase );
		hData->ttCodeBase = (tt_uint8 *)newP;
		hData->ttcode = hData->ttCodeBase + n;
	}
	
	/* Check the hData->ttDataBase array */
	n = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata;
	thisMargin = hData->ttDataBaseMaxElements - n;
	assert( thisMargin >= 0 );
	if ( thisMargin < DATA_MARGIN ) {
		tt_int32 oldLen = hData->ttDataBaseMaxElements;
		short *newP;
		/* printf("Increasing ttDataBase\n"); */
		hData->ttDataBaseMaxElements = hData->ttDataBaseMaxElements + 2 * DATA_MARGIN;
		newP = (short *)tsi_AllocMem( hData->mem, (hData->ttDataBaseMaxElements) * sizeof( short ) );
		if ( newP == NULL ) return -1; /***** Error *****/
		memcpy( newP, hData->ttCodeBase, oldLen );
		tsi_DeAllocMem( hData->mem, (char *)hData->ttDataBase );
		hData->ttDataBase = newP;
		hData->ttdata = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - n;
	}
	return errorCode; /*****/
}
#endif /* ENABLE_AUTO_HINTING */

#ifdef ENABLE_AUTO_HINTING
void ag_MovePushDataIntoInstructionStream( ag_DataType *hData, tt_int32 ttcodePosition0, tt_int32 ttdataPosition0 )
{
	tt_int32 ttcodePosition1;
	tt_int32 ttdataPosition1;
	tt_int32 maxMemSize, numberOfInstructions, numberofArgs, length, position;
	char *newP;
	
	ttcodePosition1 = hData->ttcode - hData->ttCodeBase;
	ttdataPosition1 = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata;
	if ( (ttdataPosition1+8) > hData->maxInfo.maxStackElements ) {
		hData->maxInfo.maxStackElements = (unsigned short)(ttdataPosition1+8); /* Leave room for an extra 8 elements for temporaries on the stack */
	}
	numberOfInstructions = ttcodePosition1 - ttcodePosition0;
	numberofArgs         = ttdataPosition1 - ttdataPosition0;
	
	maxMemSize = ttcodePosition1 + ttdataPosition1 * 3 + 2 * CODE_MARGIN; /* Worst case scenario, + margin */
	
	newP = (char *)tsi_AllocMem( hData->mem, maxMemSize );
	assert( newP != 0 );
	
	/* First copy instruction up to the marker */
	position = 0;
	memcpy(  &newP[position], (char *)hData->ttCodeBase, ttcodePosition0 );
	position += ttcodePosition0;
	
	/* Push the data onto the stack */
	/* length = ag_OptimizingPushArguments( hData->ttdata + ttdataPosition0 + 1, numberofArgs, (unsigned char *)&newP[position] ); */
	length = ag_OptimizingPushArguments( hData->ttdata + 1, numberofArgs, (unsigned char *)&newP[position] );
	position += length;
	
	/* Copy instruction after the marker */
	length = ttcodePosition1 - ttcodePosition0;
	memcpy(  &newP[position], (char *)&hData->ttCodeBase[ttcodePosition0], length );
	position += length;
	assert( position <= maxMemSize );
	
	/* Reset ttCodeBase, and the ttcode & ttdata pointers */
	tsi_DeAllocMem( hData->mem, (char *)hData->ttCodeBase );
	hData->ttCodeBase = (tt_uint8 *)newP;
	hData->ttcode = hData->ttCodeBase + position;
	
	hData->ttdata += numberofArgs;
}
#endif /* ENABLE_AUTO_HINTING */


#define CMD_AUTOGRID 0
#define CMD_AUTOHINT 1
#define CMD_FINDSTEMS 2
/*
 * Description:		This internal function handles AUTOGRIDDING, AUTOHINTING and FINDING STEMS.
 * How used:		Call with pointers to ag_DataType and ag_ElementType.
 					a boolean which should be set to true ofr Figures,
 					the type of curve, the command parameter (CMD_AUTOGRID,CMD_AUTOHINT,CMD_FINDSTEMS),
 					the xDist Array, and pointer to a tt_int32 for the number of elements in the xDist array,
 					the yDist Array, and pointer to a tt_int32 for the number of elements in the yDist array,
 * Side Effects: 	ag_ElementType and ag_DataType, and xDist[], yDist[], *xDistCount, and *yDistCount.
 * Return value: 	0 if no errors encountered, and < 0 otherwise
 */
static int ag_ProcessOutline(ag_DataType *hData, ag_ElementType* elem, short isFigure, short curveType, short cmd, OUT short *xDist[], OUT tt_int32 *xDistCount, OUT short *yDist[], OUT tt_int32 *yDistCount )
{
	int returnCode = 0;

	hData->isFigure = isFigure;
	assert( curveType == 2 ); /* This is all we handle at the moment */
	curveType = 2;
	assert( (elem->ep[elem->contourCount-1] + 1 + 2) <= hData->maxPointCount );

    hData->numberOfContours	= (short)elem->contourCount;
    hData->startPoint		= elem->sp;
    hData->endPoint			= elem->ep;
	hData->onCurve			= elem->onCurve;
	hData->oox				= elem->oox; 
	hData->ooy				= elem->ooy;
	
	/* Do all the topological analysis */
	hData->linkCount 						= 0;
	ag_AnalyzeChar( hData );
	ag_FindLinks( hData, hData->links, &hData->linkCount );
	
	/******************** THE CALL ********************/
	if ( cmd == CMD_AUTOGRID ) {
#ifdef ENABLE_AUTO_HINTING
		assert( false );
#endif
		returnCode = ag_DoGlyphProgram97( elem, hData );
		assert( !returnCode );
	} else if ( cmd == CMD_FINDSTEMS ) {
		returnCode = ag_GetStems( hData, elem, xDist, xDistCount, yDist, yDistCount );
	} else if ( cmd == CMD_AUTOHINT ) {
#ifdef ENABLE_AUTO_HINTING
		int numberofArgs, numberOfInstructions;
#ifdef ENABLE_AUTO_GRIDDING
		assert( false );
#endif
		hData->ttCodeBase = (tt_uint8 *)tsi_AllocMem( hData->mem, hData->ttCodeBaseMaxLength = 5000 );
		hData->ttcode = hData->ttCodeBase;
		hData->ttDataBase = (short *)tsi_AllocMem( hData->mem, (hData->ttDataBaseMaxElements = 2500) * sizeof( short ) );
		hData->ttdata = &hData->ttDataBase[hData->ttDataBaseMaxElements-1];
		
		hData->hintFragment = 0;
		if ( hData->ttCodeBase != NULL && hData->ttDataBase != NULL ) {
			returnCode = ag_DoGlyphProgram97( elem, hData );
			numberofArgs			= &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata; /* ttdata goes backwards */
			numberOfInstructions	= hData->ttcode - hData->ttCodeBase;
			
			hData->hintFragmentMaxLength	= numberOfInstructions + numberofArgs * 3; /* Worst case scenario */
			hData->hintFragment 			= (tt_uint8 *)tsi_AllocMem( hData->mem, hData->hintFragmentMaxLength ); /* The caller deallocates this */
			hData->hintFragmentLength 		= 0;
			if ( hData->hintFragment != NULL ) {
				if ( (numberofArgs+8) > hData->maxInfo.maxStackElements ) {
					hData->maxInfo.maxStackElements = (unsigned short)(numberofArgs+8); /* Leave room for an extra 8 elements for temporaries on the stack */
				}
				hData->hintFragmentLength = ag_OptimizingPushArguments( hData->ttdata + 1, numberofArgs, hData->hintFragment );
				memcpy( (char *)&hData->hintFragment[hData->hintFragmentLength], (char *)hData->ttCodeBase, numberOfInstructions);
				hData->hintFragmentLength += numberOfInstructions;
				assert( hData->hintFragmentLength <= hData->hintFragmentMaxLength );
			} else {
				returnCode = -1;
			}
		} else {
			returnCode = -1;
		}
		tsi_DeAllocMem( hData->mem, (char *)hData->ttCodeBase );
		tsi_DeAllocMem( hData->mem, (char *)hData->ttDataBase );
		if ( returnCode == -1 ) {
			tsi_DeAllocMem( hData->mem, (char *)hData->hintFragment );
			hData->hintFragment = NULL;
		}
		/* The caller HAS TO deallocate hData->hintFragment in the normal error free case */
#endif
		;
	} else {
		assert( false );
		returnCode = -1;
	}
	return returnCode; /*****/
}


#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		Scales the glyph to the right size.
 * How used:		Call with pointers to ag_DataType and ag_ElementType.
 * Side Effects: 	The x and y arrays within ag_ElementType.
 * Return value: 	None.
 */
static void ag_ScaleGlyph( ag_DataType *hData, register ag_ElementType *e )
{
	register int i, limit = e->pointCount + 2;
	register tt_int32 xMultiplier = hData->xPixelsPerEm*ag_pixelSize;
	register tt_int32 yMultiplier = hData->yPixelsPerEm*ag_pixelSize;
	register short UPEM = hData->unitsPerEm;
	
	assert( limit <= hData->maxPointCount );
	for ( i = 0; i < limit; i++ ) {
		register F26Dot6 tmpX, tmpY;
		
		/* Interleaved on purposes, so that we go faster on modern processors */
		tmpX = e->oox[i];
		tmpY = e->ooy[i];
		tmpX *= xMultiplier;
		tmpY *= yMultiplier;
		tmpX += UPEM>>1;
		tmpY += UPEM>>1;
		tmpX /= UPEM;
		tmpY /= UPEM;
		
		hData->ox[i] = tmpX;
		e->x[i]  = tmpX;
		
		hData->oy[i] = tmpY;
		e->y[i]  = tmpY;
	}
	if ( e->pointCount > 0 ) {
		assert( hData->oy[0] == e->y[0] );
	}
	i = e->pointCount;
	e->advanceWidth26Dot6 = e->x[i+1] - e->x[i];
	/* printf("ag_ScaleGlyph ooy[0] = %d, oy[0] = %ld, y[0] = %ld\n", e->ooy[0], hData->oy[0], e->y[0] ); */
}
#endif /* ENABLE_AUTO_GRIDDING */


/*
 * Description:		Sets up our cvt information.
 * How used:		Call with pointer to ag_DataType and a 'scale' boolean.
 * Side Effects: 	ocvt within ag_DataType, and also cvt within ag_DataType if 'scale' is true..
 * Return value: 	0 if no errors were encountered, and < 0 otherwise.
 *					*xWeightIsOne = xWeight == onePix
 */
static int ag_SetUpCvt( ag_DataType *hData, int scale, char *xWeightIsOne )
{
	register short i, j, sign;
	register F26Dot6 tmp, xDelta;
	register tt_int32 xMultiplier = hData->xPixelsPerEm*ag_pixelSize;
	register tt_int32 yMultiplier = hData->yPixelsPerEm*ag_pixelSize;
	short UPEM 				  = hData->unitsPerEm;
	int errorCode 			  = 0;
	register F26Dot6 *cvt	  = hData->cvt;
		
	hData->cvtHasBeenSetUp	  = true;
	xDelta = 0;
	
	assert( ag_ASCENDER_HEIGHT < ag_X_HEIGHT );
	assert( ag_CAP_HEIGHT < ag_X_HEIGHT );
	assert( ag_FIGURE_HEIGHT < ag_X_HEIGHT );

	/* Initialize */
	for ( i = 0; i < MAX_CVT_ENTRIES; i++) {
		cvt[i] = 0;
		hData->ocvt[i] = 0;
	}
	
	
	/* compute xDelta */
	tmp = hData->gData.heights[ag_X_HEIGHT].flat;
	hData->ocvt[ag_X_HEIGHT*2] = (short)tmp;
	tmp *= yMultiplier;
	tmp += UPEM >>1;
	tmp /= UPEM;
	xDelta = tmp;
	if ( hData->fontType == ag_ROMAN ) { 
		/* If Roman then raise the x-height slightly to increase legibility */
		tmp += ag_pixelSize/8+1;
	}
	tmp += ag_pixelSize/2; /* round to closest pixel */
	tmp &= ~(ag_pixelSize-1);
	cvt[ag_X_HEIGHT*2] = tmp;
	xDelta = tmp - xDelta; /* x-height distorsion */
	
	/* Loop backwards */
	/* for ( i = MAX_DOUBLE_HEIGHTS-1; i >= 0; i--) */
	for ( i = ag_MAX_HEIGHTS_IN-1; i >= 0; i--) {
		/* Flat height */
		tmp = hData->gData.heights[i].flat;
#ifdef ENABLE_AUTO_GRIDDING
		/* Helvetica 14 ppem T2K, July 2, 1997 ---Sampo */
		if ( i == ag_FIGURE_HEIGHT && hData->fontType == ag_ROMAN ) {
			register F26Dot6 tmp2;
			tmp2 = hData->gData.heights[ag_CAP_HEIGHT].flat - hData->gData.heights[ag_FIGURE_HEIGHT].flat;
			if ( tmp2 < 0 ) tmp2 = -tmp2;
			tmp2 *= yMultiplier;
			tmp2 += UPEM >> 1;
			tmp2 /= UPEM;
			
			if ( tmp2 <= ag_pixelSize/2 ) {
				tmp = hData->gData.heights[ag_CAP_HEIGHT].flat;
				/* switch to cap-height */
			}
		}
#endif
		hData->ocvt[i*2] = (short)tmp;
		if ( scale ) {
			tmp *= yMultiplier;
			tmp += UPEM >> 1;
			tmp /= UPEM;
			if ( i == ag_X_HEIGHT && hData->fontType == ag_ROMAN ) {
				/* If Roman then raise the x-height slightly to increase legibility */
				tmp += ag_pixelSize/8+1;
			} else if ( i < ag_X_HEIGHT || i == ag_PARENTHESES_TOP ) {
				tmp += xDelta;
			}
			tmp += ag_pixelSize/2; /* round to closest pixel */
			tmp &= ~(ag_pixelSize-1);
			cvt[i*2] = tmp;
		}
		/* Overlap amount */
		tmp = hData->gData.heights[i].overLap;
		hData->ocvt[i*2+1] = (short)tmp; /* only store the overlap here */
		if ( scale ) {
			sign = 1;
			if ( tmp < 0 ) {
				tmp = -tmp;
				sign = -1;
			}
			tmp *= yMultiplier;
			tmp += UPEM >> 1;
			tmp /= UPEM;
			tmp += ag_pixelSize/4; /* round -0.25 pixels => 0.75 goes to one pixels */
			tmp &= ~(ag_pixelSize-1);
			tmp = tmp * sign;
			cvt[i*2+1] = tmp + cvt[i*2];
		}
	}
	
	assert( GLOBALNORMSTROKEINDEX == 72 );
	assert( ag_MAXWEIGHTS == 12 );
	for ( j = 0; j < ag_MAXWEIGHTS; j++ ) {
		i = (short)(j + GLOBALNORMSTROKEINDEX);
		tmp = hData->ocvt[i] = hData->gData.xWeight[j];
		if ( scale && tmp != 0 ) {
			tmp *= xMultiplier;
			tmp += UPEM>>1;
			tmp /= UPEM;
			if ( tmp < 33 ) tmp = 33; /* minimum value > 0.5 pixels, so that strokes do not collapse */
			cvt[i] = tmp;
		}
	}

#ifdef ENABLE_AUTO_GRIDDING
	/* Pull towards the x-grand-father */
	tmp = cvt[GLOBALNORMSTROKEINDEX];
	for ( j = 1; j < ag_MAXWEIGHTS; j++ ) {
		i = (short)(j + GLOBALNORMSTROKEINDEX);
		if ( cvt[i] != 0 ) {
			cvt[i] = ag_ModifyCvtGoal( tmp, cvt[i] );
		}
	}
#endif
	
	for ( j = 0; j < ag_MAXWEIGHTS; j++ ) {
		i = (short)(j + GLOBALNORMSTROKEINDEX + ag_MAXWEIGHTS);
		tmp = hData->ocvt[i] = hData->gData.yWeight[j];
		if ( scale && tmp != 0 ) {
			tmp *= yMultiplier;
			tmp += UPEM>>1;
			tmp /= UPEM;
			if ( tmp < 33 ) tmp = 33; /* minimum value > 0.5 pixels, so that strokes do not collapse */
			cvt[i] = tmp;
		}
	}

#ifdef ENABLE_AUTO_GRIDDING
	/* Pull towards the y-grand-father */
	tmp = cvt[GLOBALNORMSTROKEINDEX + ag_MAXWEIGHTS];
	for ( j = 1; j < ag_MAXWEIGHTS; j++ ) {
		i = (short)(j + GLOBALNORMSTROKEINDEX + ag_MAXWEIGHTS);
		if ( cvt[i] != 0 ) {
			cvt[i] = ag_ModifyCvtGoal( tmp, cvt[i] );
		}
	}
#endif

#ifdef ENABLE_AUTO_GRIDDING
	*xWeightIsOne = (char)(cvt[GLOBALNORMSTROKEINDEX] < 64+32);
#else
	*xWeightIsOne = false;
#endif
	if ( hData->strat98 ) {
		*xWeightIsOne = false;
	}
	assert( i <= MAX_CVT_ENTRIES );
	return errorCode; /*****/
}


#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		Read the content of a file into memory.
 * How used:		Call with the name and tt_uint8 **p and tt_int32 *length.
 *					The caller has to de-allocate *p, since this routine
 *					allocates it.
 * Side Effects: 	Sets *p and *length.
 * Return value: 	0 if no errors were encountered, and < 0 otherwise.
 */
static int ag_ReadFile( tsiMemObject *mem, char *name, tt_uint8 **p, tt_int32 *length )
{
	int err;
	FILE *fp;
	int errorCode = 0;
	
	assert( sizeof( tt_uint8 ) == 1 );
	fp = fopen( name, "rb");
	if ( fp == NULL ) return -1; /*****/
	
	err = fseek( fp, 0L, SEEK_END ); /* Go to the end */
	if ( err != 0 ) errorCode = -1;
	*length = ftell( fp );
	err = fseek( fp, 0L, SEEK_SET ); /* Go to the beginning */
	if ( err != 0 ) errorCode = -1;
	*p = (unsigned char *)tsi_AllocMem( mem, *length );
	if ( errorCode == 0 && *p != NULL ) {
		tt_int32 count;
		/* Ok so far */
		count = fread( *p , sizeof( char ), *length, fp );
		if ( count != *length ) {
			errorCode = -1;
		}
	} else {
		errorCode = -1;
	}
	if ( errorCode < 0 ) {
		/* Error */
		tsi_DeAllocMem( mem, (char *)*p );
		*p = NULL;
	}
	fclose( fp );
	return errorCode; /*****/
}
#endif /* ENABLE_AUTO_HINTING */			   		


/*
 * Description:		Checks if hintHandle is a pointer to a ag_DataTypy structure.
 * How used:		Call with the hintHandle.
 * Side Effects: 	None.
 * Return value: 	True if we have a real hint-handle and false otherwise.
 */
static int ag_IsHinthandle( IN ag_HintHandleType hintHandle )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	
	return hData != NULL && hData->magic0xA5A0F5A5 == 0xA5A0F5A5 && hData->magic0x0FA55AF0 == 0x0FA55AF0; /*****/
}

/********** THE EXTERNAL INTERFACE CALLS ARE ALL BELOW THIS LINE **********/


/*
 * Description:		Call this first
 * 					This returns the hintHandle which is passed to all the other external calls.
 * 					In a multi-threaded environment each thread needs to have it's own hintHandle.
 * How used:		Call with the maximum point count for the font, the units per Em,
 *					and a pointer (hintHandle) to ag_HintHandleType.
 * Side Effects: 	Sets *hintHandle.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_HintInit( IN tsiMemObject *mem, IN int maxPointCount, IN short unitsPerEm, OUT  ag_HintHandleType *hintHandle )
{
	register ag_DataType *hData;
	int returnCode = 0;
	
	hData 								= (ag_DataType *)tsi_AllocMem( mem, sizeof( ag_DataType ) );
	if ( hData == NULL ) return -1; /* Bail out */
	hData->mem							= mem;
	hData->magic0xA5A0F5A5				= 0xA5A0F5A5; /* Initialize our magic number */
	hData->magic0x0FA55AF0				= 0x0FA55AF0; /* This is good for basic sanity checking */
	
/*#define JAM_WEIGHTS */
#ifdef JAM_WEIGHTS
	/* HARD-WIRED FOR Arial */

	int i;
	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		hData->gData.xWeight[i] = 0;
		hData->gData.yWeight[i] = 0;
	}
	hData->gData.xWeight[0] = (short)(185 * (tt_int32)unitsPerEm / 2048L);
	hData->gData.yWeight[0] = (short)(145 * (tt_int32)unitsPerEm / 2048L);
	hData->gData.heights[ag_ASCENDER_HEIGHT].flat		= 1466;
	hData->gData.heights[ag_ASCENDER_HEIGHT].overLap	= 25;
	hData->gData.heights[ag_CAP_HEIGHT].flat			= 1466;
	hData->gData.heights[ag_CAP_HEIGHT].overLap			= 25;
	hData->gData.heights[ag_FIGURE_HEIGHT].flat			= 1466;
	hData->gData.heights[ag_FIGURE_HEIGHT].overLap		= 25;
	hData->gData.heights[ag_X_HEIGHT].flat				= 1062;
	hData->gData.heights[ag_X_HEIGHT].overLap			= 25;
	hData->gData.heights[ag_UC_BASE_HEIGHT].flat		= 0;
	hData->gData.heights[ag_UC_BASE_HEIGHT].overLap		= -25;
	hData->gData.heights[ag_LC_BASE_HEIGHT].flat		= 0;
	hData->gData.heights[ag_LC_BASE_HEIGHT].overLap		= -25;
	hData->gData.heights[ag_FIGURE_BASE_HEIGHT].flat	= 0;
	hData->gData.heights[ag_FIGURE_BASE_HEIGHT].overLap = -25;
	hData->gData.heights[ag_DESCENDER_HEIGHT].flat		= -407;
	hData->gData.heights[ag_DESCENDER_HEIGHT].overLap	= -25;
	hData->gData.heights[ag_PARENTHESES_TOP].flat		= 1466;
	hData->gData.heights[ag_PARENTHESES_TOP].overLap	= 25;
	hData->gData.heights[ag_PARENTHESES_BOTTOM].flat	= -407;
	hData->gData.heights[ag_PARENTHESES_BOTTOM].overLap	= -25;
	for ( i = ag_ASCENDER_HEIGHT; i <= ag_PARENTHESES_BOTTOM; i++ ) {
		hData->gData.heights[i].round = (short)(hData->gData.heights[i].flat + hData->gData.heights[i].overLap);
	}
#endif
	/* Initialize maxInfo */
	hData->maxInfo.maxZones 				= 2;
	hData->maxInfo.maxTwilightPoints		= 16;
	hData->maxInfo.maxStorage				= MAXSTORAGE;
	hData->maxInfo.maxFunctionDefs			= 32;
	hData->maxInfo.maxInstructionDefs		= 0;
	hData->maxInfo.maxStackElements			= 256; /* Max for all glyphs AND pre AND font pgm */
	hData->maxInfo.maxSizeOfInstructions	= 0;   /* Max for all glyphs  */

	
	hData->fontType						= ag_ROMAN;	/* Initialize */
	hData->cvtHasBeenSetUp				= false;
	hData->hintInfoHasBeenSetUp			= false;

	maxPointCount += 2;					/* + 2 for the side bearing points */
	hData->maxPointCount 				= maxPointCount;
	hData->unitsPerEm 					= unitsPerEm;
	/* memcpy( (char *)&hData->gData, (char *)gDataIn, sizeof(ag_GlobalDataType)  ); */

	hData->f 							= (tt_uint8 *)tsi_AllocMem( mem, maxPointCount * sizeof( tt_uint8 ) );
	hData->ox							= (F26Dot6 *)tsi_AllocMem( mem, 2 * maxPointCount * sizeof( F26Dot6 ) );
	hData->oy							= (F26Dot6 *)&hData->ox[maxPointCount];

	hData->nextPt						= (tt_int16 *)tsi_AllocMem( mem, 3 * maxPointCount * sizeof( tt_int16 ) );
	hData->prevPt						= &hData->nextPt[maxPointCount];
	hData->searchPoints					= &hData->nextPt[2 * maxPointCount];
	
	hData->flags						= (tt_uint16 *)tsi_AllocMem( mem, maxPointCount * sizeof( tt_uint16 ) );

	hData->realX						= (tt_int16 *)tsi_AllocMem( mem, 2 * maxPointCount * sizeof( tt_int16 ) );
	hData->realY						= &hData->realX[maxPointCount];
	
	hData->forwardAngleOrthogonalPair	= (tt_int16 *)tsi_AllocMem( mem, 3 * maxPointCount * sizeof( tt_int16 ) );
	hData->backwardAngleOrthogonalPair	= &hData->forwardAngleOrthogonalPair[maxPointCount];
	hData->pointToLineMap				= &hData->forwardAngleOrthogonalPair[2*maxPointCount];

	hData->cos_f						= (SMALLFRAC *)tsi_AllocMem( mem, 4 * maxPointCount * sizeof( SMALLFRAC ) );
	hData->sin_f						= &hData->cos_f[ maxPointCount ];
	hData->cos_b						= &hData->cos_f[ 2*maxPointCount ];
	hData->sin_b						= &hData->cos_f[ 3*maxPointCount ];
	
	hData->maxLinks						= maxPointCount + maxPointCount; /* Use 2.0, the CheckerBox in Arial has a ratio of 1.88 */
	hData->links						= (tp_LinkType *)tsi_AllocMem( mem, hData->maxLinks * sizeof( tp_LinkType ) );

	if ( hData == NULL || hData->f == NULL || hData->ox == NULL || hData->oy == NULL ||
		 hData->nextPt == NULL || hData->flags == NULL || hData->realX == NULL || hData->forwardAngleOrthogonalPair == NULL ||
		 hData->cos_f == NULL || hData->links == NULL ) {
		if ( hData != NULL ) {
			ag_HintEnd( hData );
			hData = NULL; 
		}
		returnCode = -1;
	}
	/* Prior to JDk 1.4.2 this flag was undefined so it was "usually"
	 * false. In JDK 1.4.2 it was set to "false" as changing it to the
	 * less common value needed more time to test. In 1.5 it is now "true".
	 * This autogridding strategy appears to result in less washed-out
	 * anti-aliased text, ie somewhat more readable text. There are
	 * still some stems which aren't black, but its an improvement.
	 */
	hData->strat98 = true;
	
	*hintHandle = (ag_HintHandleType)hData;
	return returnCode; /******/
}

/*
 * Description:		Call this to get the data for the ag_HintMaxInfoType structure
 * 					This should be called after ALL the glyphs have been hinted.
 * How used:		Call with the hintHandle and pointer to a ag_HintMaxInfoType
 *					data structure.
 * Side Effects: 	Sets the contents of the ag_HintMaxInfoType data structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_GetHintMaxInfo( IN ag_HintHandleType hintHandle, OUT ag_HintMaxInfoType *p )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	if ( p == 0L || !ag_IsHinthandle(hintHandle) )  return -1; /*****/

	/* *p = &hData->maxInfo; */
	memcpy(p, &hData->maxInfo, sizeof(ag_HintMaxInfoType));
	return 0; /******/
}


/*
 * Description:		Call this to cause the auto-grider to de-allocate it's memory
 * 					HintInit() and HintEnd() form a matching pair
 * How used:		Call with the hintHandle.
 * Side Effects: 	De-allocates internal memory within the ag_DataType data-structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_HintEnd( IN ag_HintHandleType hintHandle )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	tsiMemObject *mem;
	
	if ( hData != NULL ) {
		if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
		mem = hData->mem;
		
		tsi_DeAllocMem( mem, ( char *)hData->f );
		tsi_DeAllocMem( mem, ( char *)hData->ox );
		
		tsi_DeAllocMem( mem, (char *)hData->nextPt );
		tsi_DeAllocMem( mem, (char *)hData->flags );
		tsi_DeAllocMem( mem, (char *)hData->realX );
		tsi_DeAllocMem( mem, (char *)hData->forwardAngleOrthogonalPair );
		tsi_DeAllocMem( mem, (char *)hData->cos_f );
		tsi_DeAllocMem( mem, (char *)hData->links );
		
		tsi_DeAllocMem( mem, (char *)hData );
	}
	return 0; /*****/
}

/*
 * Description:		Call this to set global hint information.
 * 					Always, set at least one x and one y weight.
 * 					Set any unused heights and weights to zero.
 * 					Call this once, before calling ag_AutoGridOutline(), or ag_AutoHintOutline()
 * How used:		Call with the hintHandle, a pointer to ag_GlobalDataType, and the font type.
 * Side Effects: 	Stores the passed in global hint information and the fontType within the private ag_DataType data structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_SetHintInfo( IN ag_HintHandleType hintHandle, IN ag_GlobalDataType *gDataIn, ag_FontCategory fontType )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	int returnCode = 0;
	int i;
	
	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
	
	hData->fontType			= fontType;
	if ( gDataIn ) {	/* allow for null pointer (ie, just pass in fontType) */
		memcpy( (char *)&hData->gData, (char *)gDataIn, sizeof(ag_GlobalDataType)  );
		for ( i = ag_ASCENDER_HEIGHT; i <= ag_PARENTHESES_BOTTOM; i++ ) {
		   assert( hData->gData.heights[i].round == hData->gData.heights[i].flat + hData->gData.heights[i].overLap );
		}
	}
	hData->hintInfoHasBeenSetUp = true;
	return returnCode; /******/
}

/*
 * Description:		Always call this when the size changes, but not otherwise
 * 					A call to AutoGridOutline() is invalid without any previous call to SetScale()
 * 					However if the scale stays constant then multiple calls to AutoGridOutline(), or AutoHintOutline()
 * 					is OK as long as SetScale() was called once with the correct scale first.
 * 					For speed reasons do not call SetScale() more often than you have to.
 * How used:		Call with the hintHandle, and the x and y pixels per Em.
 * Side Effects: 	Remembers the x and y pixels per Em and sets up the internal cvt information.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 *					*xWeightIsOne = xWeight == onePix
 */
int ag_SetScale( IN ag_HintHandleType hintHandle, tt_int32 xPixelsPerEm, tt_int32 yPixelsPerEm, char *xWeightIsOne )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/

	hData->xPixelsPerEm		= xPixelsPerEm;
	hData->yPixelsPerEm		= yPixelsPerEm;
	
	return ag_SetUpCvt( hData, true, xWeightIsOne ); /*****/
}

#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		The caller should copy the 3 tables directly into the 3 corresponding TrueType tables
 * 					This function is only called when hints are desired. Not when we are just auto-gridding.
 * 					The caller HAS to deallocate all three memory pointers.
 * How used:		Call with the hintHandle, and three pairs of ((char **) and (tt_int32*)) for the font program,
 *					pre program and control value table.
 * Side Effects: 	None.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_GetGlobalHints( IN ag_HintHandleType hintHandle,
					 OUT unsigned char **fpgm, OUT tt_int32 *fpgmLength,
					 OUT unsigned char **ppgm, OUT tt_int32 *ppgmLength,
					 OUT short **cvt,          OUT tt_int32 *cvtCount )
{
	register ag_DataType *hData = (ag_DataType *)hintHandle;
	register tt_int32 i;
	register tt_int16 *cvtArea;
	int err1, err2, err3;
	char xWeightIsOne;
	
	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
	err1 = ag_ReadFile( hData->mem, "fpgm.bin", fpgm, fpgmLength ); /* We, will just produce the right data with Visual TypeMan */
	err2 = ag_ReadFile( hData->mem, "ppgm.bin", ppgm, ppgmLength ); /* We, will just produce the right data with Visual TypeMan */
	
	*cvtCount = MAX_CVT_ENTRIES;
	*cvt = cvtArea = (tt_int16 *)tsi_AllocMem( hData->mem, MAX_CVT_ENTRIES * sizeof( tt_int16 ) );
	if ( cvtArea != NULL ) {
		err3 = ag_SetUpCvt( hData, false, &xWeightIsOne );
		for ( i = 0; i < MAX_CVT_ENTRIES; i++ ) {
			cvtArea[i] = hData->ocvt[i];
		}
	} else {
		err3 = -1;
	}
	return err1 == 0 && err2 == 0 && err3 == 0 ? 0 : -1; /*****/
}
#endif /* ENABLE_AUTO_HINTING */



/*
 * Description:		This function can be used, for analyzing strokes for the purpose of
 * 					finding global stem-weights. It sets the content of the xDist[], and yDist[] arrays
 * 					It also sets *xDistCount, and *yDistCount to the number of links found in the x and y directions.
 * 					The caller HAS to deallocate *xDist and *yDist
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 )
 *					and two pairs of (short **, and tt_int32 *) for the x and y stem-weight data.
 * Side Effects: 	xDist[], and yDist[], and *xDistCount, *yDistCount.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_AutoFindStems( IN ag_HintHandleType hintHandle, IN ag_ElementType *elem, IN short isFigure, IN short curveType, OUT short *xDist[], OUT tt_int32 *xDistCount, OUT short *yDist[], OUT tt_int32 *yDistCount )
{
	int errorCode;
	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
	errorCode = ag_ProcessOutline((ag_DataType *)hintHandle, elem, isFigure, curveType, CMD_FINDSTEMS, xDist, xDistCount, yDist, yDistCount );
	return errorCode; /*****/
}

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		This function auto grids the glyph stored within ag_ElementType.
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 )
 * Side Effects: 	ag_DataType and ag_ElementType.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_AutoGridOutline( IN ag_HintHandleType hintHandle, ag_ElementType* elem, short isFigure, short curveType, short grayScale )
{
	int errorCode;
	ag_DataType *hData = (ag_DataType *)hintHandle;
	/*_CrtCheckMemory(); */
	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
	hData->strat98 = grayScale ? true : false;
	ag_ScaleGlyph( hData, elem );
	assert( hData->fontType == ag_ROMAN || hData->fontType == ag_KANJI );
	assert( hData->hintInfoHasBeenSetUp );
	errorCode = ag_ProcessOutline(hData, elem, isFigure, curveType, CMD_AUTOGRID, NULL, NULL, NULL, NULL );
	elem->advanceWidthInt = (elem->x[elem->pointCount+1] - elem->x[elem->pointCount] + 32) >> 6;

	/*_CrtCheckMemory(); */
	return errorCode; /*****/
}
#endif /* ENABLE_AUTO_GRIDDING */

#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		This function auto hints the glyph stored within ag_ElementType.
 * 					The caller HAS to deallocate *hintFragment
 *					This can be called instead of AutoGridOutline() when the client needs
 *					the TrueType hints instead of the auto-gridded outline.
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 ),
 *					and char **hintFragment, and tt_int32 *hintLength for the TrueType glyph hint data.
 * Side Effects: 	ag_DataType, hintFragment, hintLength.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
int ag_AutoHintOutline( IN ag_HintHandleType hintHandle, IN ag_ElementType *elem, IN short isFigure, IN short curveType, OUT unsigned char *hintFragment[], OUT tt_int32 *hintLength )
{
	int errorCode;
	ag_DataType *hData = (ag_DataType *)hintHandle;

	if ( !ag_IsHinthandle(hintHandle) )  return -1; /*****/
	assert( hData->fontType == ag_ROMAN || hData->fontType == ag_KANJI );
	assert( hData->hintInfoHasBeenSetUp );
	errorCode = ag_ProcessOutline(hData, elem, isFigure, curveType, CMD_AUTOHINT, NULL, NULL, NULL, NULL );
	*hintFragment	= hData->hintFragment;
	*hintLength		= hData->hintFragmentLength;
	if ( hData->hintFragmentLength > hData->maxInfo.maxSizeOfInstructions ) {
		hData->maxInfo.maxSizeOfInstructions = (unsigned short)hData->hintFragmentLength;
	}
	return errorCode; /*****/
}c
#endif /* ENABLE_AUTO_HINTING */
