/*
 * @(#)sc.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
	GX CHANGES
	02/16/94	rwb	changed type of sc_FindExtrema4 to void
	02/07/94	rwb	Break up FindExtrema2 into FindExtrema4, and CalculateBoundingBox
	02/07/94	rwb	Took out references to oldFindExtrema
		
	Copyright:	© 1990,1994 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		<16>	 4/25/91	RB		Post 7.0 - Speed up draw_parabola loop
		<15>	 2/11/91	RB		MR,CL,CC,#82410: Add comment about slop in veccount and initials
									for previous change.(date changed to be compatible
									with Reality Sources)
		<14>	 1/24/91	RB		(MR) #External Reported Bug. DropoutControl excluding stubs did
									not work properly on the corner of a glyph bounding box. Changed
									nUpperCrossings and nLowerCrossings.(date changed to be compatible
									with Reality Sources)
		<13>	12/20/90	RB			Add ZERO macro to include 0 degrees in definition of interior
									angle.  Set oncol in MarkRows when vector does not start on row
									but does start on column. [mr]
		<12>	12/18/90	RB		Revert to old definition of interior to not include 0 and 180
									degrees.[mr]
		<11>	12/10/90	RB		Change findextrema to return error if min,max are outside of
									-32768,32767 bounds. Redefine INTERIOR macro to include 0
									degrees and 180 degrees as true. Optimized fillonerow. [cel]
		<10>	 12/5/90	RB		Fix fill routine to agree with spec and do non-zero winding
									number fill rather than positive winding number fill.[mr]
		 <9>	11/21/90	RB		A few optimizations in findextrema, sortRows, sortCols,
									orSomeBits, invpixsegX, invpixsegY. [This is a reversion to <7>
									so mr's initials are added by proxy]
		 <8>	11/13/90	MR		(dnf) Revert back one revision to fix a memmory-trashing bug (we
									hope).
		 <7>	11/13/90	RB		Fix banding so that we can band down to one row, using only
									enough gxBitmap memory and auxillary memory for one row.[mr]
		 <6>	 11/5/90	MR		type cast NEGONE constant, remove QD includes [rb]
		 <5>	10/31/90	RB		No Change
		 <4>	10/31/90	RB		[MR]Fix weitek bug of long<<32 undefined
		 <3>	10/30/90	RB		[MR] Projector screwup - starting over with new version. Code
									Review Changes speeded up scanconverter by up to 33%. Made
									macros from functions INTERIOR, FILLONEROW, ISORT Unrecursed
									compare and put inline. Renamed sort3rows->sortrows,
									sort3cols->sortcols Undid fix <14>, because fs_contourscan now
									inhibits dropoutcontrol when banding

	To Do:
*/

/*
	File:		sc.c

	Contains:	xxx put contents here (or delete the whole gxLine) xxx

	Written by:	xxx put name of writer here (or delete the whole gxLine) xxx

	Copyright:	© 1987-1990 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):
		<15>	 9/26/90	CL		Messed up on adding in the version comment. Needed some comment
									brackets!!!
		<14>	 9/26/90	CL		A bug in sc_orsomebits caused memory trashing.  This routine did
									not take into account that banding could occur and would write
									outside of the gxBitmap band.  This fix checks to see if we are
									banding and skips this routine.
		<13>	 8/22/90	RB		fixed nUppercrossings and nLowercrossings to not access out of
									range scan lines.  Moved functions around in module to be more
									readable.
		<12>	 8/18/90	RB		new sort routine, took out multlong(),
		<11>	 7/18/90	RB		fixed bug in invpixyseg - stray pixels undone
		<10>	 7/18/90	MR		sc_ScanChar now returns error code as int
		 <9>	 7/13/90	MR		Ansi-C stuff, added some static prototypes
		 <8>	 6/22/90	RB		added stubcontrol option to orsomebits
		 <7>	  6/1/90	RB		fix banding/dropout bug
		 <6>	 5/10/90	RB		fix overlapping points that lie on scanlines
		<4+>	  5/8/90	RB		fix scanning of glyphs that collapse to gxLine and overlapping
									points that are on scanlines.
		 <4>	  5/3/90	RB		  Almost completely new scanconverter.  Winding number fill,
									dropout control.
									  Took out Fixupper requirement on dropout control.
		 <3>	 2/27/90	CL		Dropoutcontrol scanconverter and SCANCTRL[] instruction
	   <3.0>	 8/28/89	sjk		Cleanup and one transformation bugfix
	   <2.2>	 8/14/89	sjk		1 point contours now OK
	   <2.1>	  8/8/89	sjk		Improved encryption handling
	   <2.0>	  8/2/89	sjk		Just fixed EASE comment
	   <1.7>	  8/1/89	sjk		Added composites and encryption. Plus some enhancements…
	   <1.6>	 6/13/89	SJK		Comment
	   <1.5>	  6/2/89	CEL		16.16 scaling of metrics, minimum recommended ppem, point size 0
									bug, correct transformed integralized ppem behavior, pretty much
									so
	   <1.4>	 5/26/89	CEL		EASE messed up on “c” comments
	  <•1.3>	 5/26/89	CEL		Integrated the new Font Scaler 1.0 into Spline Fonts

	To Do:
*/
/*
 * File: sc.c
 *
 * This module scanconverts a gxShape defined by quadratic B-splines
 *
 * The BASS project scan converter sub ERS describes the workings of this module.
 *
 *
 *  © Apple Computer Inc. 1987, 1988, 1989, 1990.
 *
 * History:
 * Work on this module began in the fall of 1987.
 * Written June 14, 1988 by Sampo Kaasila.
 *
 * Released for alpha on January 31, 1989.
 * 
 * Added experimental non breaking scan-conversion feature, Jan 9, 1990. ---Sampo
 * 
 */
 
#include "Hint.h"
#ifdef ENABLE_TT_HINTING

#include    "FSCdefs.h"
#include    "sc.h"
#include    "FSError.h"
/* #include	"scaler_types.h"
#include 	"FSglue.h"*/

/* Private prototypes */
				
local void sc_mark( tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 bx, tt_int32 by,
				tt_int16 **xBase, tt_int16 **yBase, sc_BitMapData *bbox );
				
local void sc_markRows( tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 xb, tt_int32 yb,
	 tt_int16 **yBase, sc_BitMapData *bbox, tt_int16** lowRowP, tt_int16** highRowP );
	
local void sc_lineGen(sc_BitMapData *bbox, tt_int32 *px, tt_int32 *py, tt_int32 *pend,
						tt_int16 **xBase, tt_int16 **yBase );
local void sc_lineGenRows(sc_BitMapData *bbox, tt_int32 *px, tt_int32 *py, tt_int32 *pend,
					 tt_int16 **yBase, tt_int16** lowRowP, tt_int16** highRowP );
local void sortRows( sc_BitMapData *bbox, tt_int16** lowRowP, tt_int16** highRowP );
 static void sortCols( sc_BitMapData *bbox );

#if 0
local tt_uint32* compare( tt_int16 coord, tt_int16* lastBitP, tt_uint32* tempP, tt_uint32* row, tt_int16 windNbr);
#endif

local tt_int32 sc_DrawParabola(tt_int32 Ax, tt_int32 Ay, tt_int32 Bx, tt_int32 By, tt_int32 Cx, tt_int32 Cy, tt_int32 **hX, tt_int32 **hY, tt_int32 *count, tt_int32 inGY);

local void sc_wnNrowFill( tt_int32 rowM, tt_int16 nRows, sc_BitMapData *bbox );

static void sc_orSomeBits( sc_BitMapData *bbox, tt_int32 scanKind);

static tt_int16** sc_lineInit( tt_int16* arrayBase, tt_int16** rowBase, tt_int16 nScanlines, tt_int16 maxCrossings,
							tt_int16 minScanline );
static tt_int32 nUpperXings(tt_int16** lineBase, tt_int16** valBase, tt_int16 gxLine, tt_int16 val,
						tt_int16 lineChanges, tt_int16 valChanges, tt_int16 valMin, tt_int16 valMax, tt_int16 lineMax); /*<14>*/
static tt_int32 nLowerXings(tt_int16** lineBase, tt_int16** valBase, tt_int16 gxLine, tt_int16 val,
						tt_int16 lineChanges, tt_int16 valChanges, tt_int16 valMin, tt_int16 valMax, tt_int16 lineMin); /*<14>*/
static void invpixSegY(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP );

static void invpixSegX(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP );

static void invpixOn(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP );

/*
 * Returns the gxBitmap
 * This is the top level call to the scan converter.
 *
 * Assumes that (*handle)->bbox.xmin,...xmax,...ymin,...ymax
 * are already set by sc_FindExtrema()
 *
 * PARAMETERS:
 *
 *
 * glyphPtr is a pointer to sc_CharDataType
 * scPtr is a pointer to sc_GlobalData.
 * lowBand   is lowest scan gxLine to be included in the band.
 * highBand  is one greater than the highest scan gxLine to be included in the band. <7>
 * scanKind contains flags that specify whether to do dropout control and what kind
 * 	0 -> no dropout control
 *	bits 0-15 not equal 0 -> do dropout control
 *	if bit 16 is also on, do not do dropout control on 'stubs' 
*/
int sc_ScanChar2(const fnt_ElementType *glyphPtr, sc_GlobalData *scPtr, sc_BitMapData *bbox,
			tt_int16 lowBand, tt_int16 highBand, tt_int32 scanKind /* , scalerContext* theContext*/) 
{
	register F26Dot6 *x = glyphPtr->x;
	register F26Dot6 *y = glyphPtr->y;
    register ArrayIndex i, endPt, nextPt;
	register tt_int8 *onCurve = (tt_int8*)glyphPtr->onCurve;
	ArrayIndex startPt, j;
	LoopCount ctr;
	sc_GlobalData *p;
    F26Dot6 *xp, *yp, *x0p, *y0p;
    register F26Dot6 xx, yy, xx0, yy0;
	int quit;
	tt_int16 **lowRowP, **highRowP;
	tt_int32 vecCount;
			
	if( scanKind )
	{
		bbox->xBase = sc_lineInit( bbox->xLines, bbox->xBase, (tt_int16)(bbox->xMax - bbox->xMin),
					   bbox->nXchanges, bbox->xMin );	
		bbox->yBase = sc_lineInit( bbox->yLines, bbox->yBase, (tt_int16)(bbox->yMax - bbox->yMin),
					   bbox->nYchanges, bbox->yMin);
	}
	else
	{
		bbox->yBase = sc_lineInit( bbox->yLines, bbox->yBase, (tt_int16)(highBand - lowBand),
					   bbox->nYchanges, lowBand );
	}
	lowRowP = bbox->yBase + lowBand;			
	highRowP = bbox->yBase + highBand - 1;
	
	if( glyphPtr->contourCount == 0 ) 
		return scalerError_NoError; /* NO_ERR; */
	p = scPtr;
	for ( ctr = 0; ctr < glyphPtr->contourCount; ctr++ ) {
		x0p = xp = p->xPoints;
        y0p = yp = p->yPoints;
	    startPt = i = glyphPtr->sp[ctr];		
        endPt = glyphPtr->ep[ctr];
		
		if ( startPt == endPt ) continue; 
		quit = 0;
		vecCount = 1;
		if ( onCurve[i] & ONCURVE) {
 		    *xp++ = xx = x[i];
		    *yp++ = yy = y[i++];
		} else {
		    if ( onCurve[endPt] & ONCURVE ) {
			    startPt = endPt--;
				*xp++ = xx = x[startPt];
				*yp++ = yy = y[startPt];
			} else {
				*xp++ = xx = (x[i] + x[endPt] + 1) >> 1;
				*yp++ = yy = (y[i] + y[endPt] + 1) >> 1;
    			goto Offcurve;
			}
		}
	    while ( true ) {
		    while ( onCurve[i] & ONCURVE )
			{
				if( ++vecCount > MAXVECTORS ) 
				{ /*Ran out of local memory. Consume data and continue. */
					if ( scanKind )
							sc_lineGen(bbox, x0p, y0p, yp-1, bbox->xBase, bbox->yBase );
					else sc_lineGenRows(bbox, x0p, y0p, yp-1, bbox->yBase, lowRowP, highRowP );
					x0p = p->xPoints + 2;		/* save data in points 0 and 1 for final */
					y0p = p->yPoints + 2;
					*x0p++ = *(xp-2);			/* save last vector to be future previous vector */
					*x0p++ = *(xp-1);
					*y0p++ = *(yp-2);
					*y0p++ = *(yp-1);
					xp = x0p;					/* start next processing with last vector */
					x0p = p->xPoints + 2;
					yp = y0p;
					y0p = p->yPoints + 2;
					vecCount = 5;				/* probably could be 4 <15> */
				}
			    *xp++ = xx = x[i];
		        *yp++ = yy = y[i];
				if ( quit ) {
					goto sc_exit;
				} else {
   	                i = i == endPt ? quit = 1, startPt : i + 1;
				}
			}

			do {
Offcurve:		xx0 = xx;
				yy0 = yy;
			    /* nextPt = (j = i) + 1; */
				j = i;
                nextPt = i == endPt ? quit = 1, startPt : i + 1;
				if ( onCurve[nextPt] & ONCURVE ) {
				    xx = x[nextPt];
					yy = y[nextPt];
					i = nextPt;
				} else {
					xx = (x[i] + x[nextPt] + 1) >> 1;
					yy = (y[i] + y[nextPt] + 1) >> 1;
				}
                if ( sc_DrawParabola( xx0, yy0, x[j], y[j], xx, yy,
				                      &xp, &yp, &vecCount, -1) )
				{ /* not enough room to create parabola vectors  */
					if ( scanKind )
							sc_lineGen(bbox, x0p, y0p, yp-1, bbox->xBase, bbox->yBase );
					else sc_lineGenRows(bbox, x0p, y0p, yp-1, bbox->yBase, lowRowP, highRowP );
					
					x0p = p->xPoints + 2;
					y0p = p->yPoints + 2;
					*x0p++ = *(xp-2);
					*x0p++ = *(xp-1);
					*y0p++ = *(yp-2);
					*y0p++ = *(yp-1);
					xp = x0p;
					x0p = p->xPoints + 2;
					yp = y0p;
					y0p = p->yPoints + 2;
					vecCount = 5;
					/* recaptured some memory, try again, if still wont work, MAXVEC is too small */
                	if ( sc_DrawParabola( xx0, yy0, x[j], y[j], xx, yy,
				                      &xp, &yp, &vecCount, -1) ) 
				            return  scalerError_ScanError; /* SCAN_ERR; */
				}
				if ( quit ) {
					goto sc_exit;
				} else {
   	                i = i == endPt ? quit = 1, startPt : i + 1;
				}
            } while ( ! (onCurve[i] & ONCURVE) );

    	}
sc_exit:

		if ( scanKind )
		{
			sc_lineGen(bbox, x0p, y0p, yp-1, bbox->xBase, bbox->yBase );
			sc_mark( *(p->xPoints), *(p->yPoints), *(p->xPoints+1), *(p->yPoints+1),
				*(xp-2), *(yp-2), bbox->xBase, bbox->yBase, bbox);
		}
		else
		{
			sc_lineGenRows(bbox, x0p, y0p, yp-1, bbox->yBase, lowRowP, highRowP );
			sc_markRows( *(p->xPoints), *(p->yPoints), *(p->xPoints+1), *(p->yPoints+1),
				*(xp-2), *(yp-2), bbox->yBase, bbox, lowRowP, highRowP );
		}
	}
	
	sortRows( bbox, lowRowP, highRowP ); 
	if ( scanKind ) sortCols( bbox );
	
/* Take care of problem of very small thin glyphs - always fill at least one pixel
   Should this only be turned on if dropout control ?? 
 */
	if( highRowP < lowRowP )
	{
		register tt_int16 *p = *lowRowP;
		register tt_int16 *s = p + bbox->nYchanges+1;
		++*p; *(p+*p) = bbox->xMin;
		++*s; *(s-*s) = bbox->xMax == bbox->xMin ? bbox->xMin+1 : bbox->xMax;
		highBand = lowBand+1;
	}
	else if( bbox->xMin == bbox->xMax )
	{
		register tt_int16 *p;
		register tt_int16 inc = bbox->nYchanges;
		for( p = *lowRowP; p <= *highRowP; p += inc+1 )
		{
			*p=1;
			*(p+inc) = bbox->xMin+1; 
			*(++p) = bbox->xMin;
			*(p+inc) = 1;
		}
	}
	sc_wnNrowFill( lowBand, (tt_int16)(highBand - lowBand), bbox );
	
 	if ( scanKind ) 
 		sc_orSomeBits( bbox, scanKind );
 	return scalerError_NoError; /* NO_ERR; */
}
/* rwb 11/29/90 - modify the old positive winding number fill to be 
 * a non-zero winding number fill to be compatible with skia, postscript,
 * and our documentation.
 */

#define NEGONE 		((tt_uint32)0xFFFFFFFF)

/* x is pixel position, where 0 is leftmost pixel in scanline. 
 * if x is not in the long pointed at by row, set row to the value of temp, clear
 * temp, and clear all longs up to the one containing x.  Then set the bits
 * from x mod 32 through 31 in temp.
 */
#define CLEARUpToAndSetLoOrder( x, lastBit, row, temp )		\
{															\
	if( x >= lastBit )										\
	{														\
		*row++ = temp;										\
		temp = 0;											\
		lastBit += 32;										\
	}														\
	while( x >= lastBit )									\
	{														\
		*row++ = 0;											\
		lastBit += 32;										\
	}														\
	temp |= (NEGONE >> (32 + x - lastBit)); 				\
}

/* x is pixel position, where 0 is leftmost pixel in scanline. 
 * if x is not in the long pointed at by row, set row to the value of temp, set
 * all bits in temp, and set all bits in all longs up to the one containing x.
 * Then clear the bits from x mod 32 through 31 in temp.
 */
#define SETUpToAndClearLoOrder( x, lastBit, row, temp )     \
{															\
	if( x > lastBit )		/*<4>*/							\
	{														\
		*row++ = temp;										\
		temp = NEGONE;										\
		lastBit += 32;										\
	}														\
	while( x > lastBit )	/*<4>*/							\
	{														\
		*row++ = NEGONE;									\
		lastBit += 32;										\
	}														\
	temp &= (NEGONE << (lastBit - x)); 						\
}

#define FILLONEROW( row, longsWide, gxLine, lineWide, xMin )  													\
/* do a winding number fill of one row of a gxBitmap from two sorted arrays \
of onTransitions and offTransitions. 								\
*/																	\
{																	\
register tt_int16 moreOns, moreOffs;									\
register tt_int16 *onTp, *offTp;										\
register tt_uint32 temp;												\
tt_uint32 *rowEnd = row + longsWide;									\
tt_int32  windNbr, lastBit, on, off;									\
																	\
lastBit = 32 + xMin;												\
windNbr  = 0;														\
temp = 0;															\
moreOns = *gxLine;													\
onTp = gxLine+1;														\
offTp = gxLine + lineWide - 1;										\
moreOffs = *offTp;													\
offTp -= moreOffs;													\
																	\
while( moreOns || moreOffs )										\
{																	\
	if( moreOns )													\
	{																\
		on = *onTp;													\
		if( moreOffs )												\
		{															\
			off = *offTp;											\
			if( on < off )											\
			{														\
				--moreOns;											\
				++onTp;												\
				++windNbr;											\
				if( windNbr == 1)									\
					CLEARUpToAndSetLoOrder( on, lastBit, row, temp )\
				else if( windNbr == 0 )								\
					SETUpToAndClearLoOrder( on, lastBit, row, temp )\
			}														\
			else if( on > off )										\
			{														\
				--moreOffs;											\
				++offTp;											\
				--windNbr;											\
				if( windNbr == 0)									\
					SETUpToAndClearLoOrder( off, lastBit, row, temp)\
				else if( windNbr == -1 )							\
					CLEARUpToAndSetLoOrder( off, lastBit, row, temp)\
			}														\
			else													\
			{														\
				--moreOns;											\
				++onTp;												\
				--moreOffs;											\
				++offTp;											\
			}														\
		}															\
		else    						/* no more offs left */		\
		{															\
			--moreOns;												\
			++onTp;													\
			++windNbr;												\
			if( windNbr == 1)										\
				CLEARUpToAndSetLoOrder( on, lastBit, row, temp )	\
			else if( windNbr == 0 )									\
				SETUpToAndClearLoOrder( on, lastBit, row, temp )	\
		}															\
	}																\
	else								/* no more ons left */		\
	{																\
		off = *offTp;												\
		--moreOffs;													\
		++offTp;													\
		--windNbr;													\
		if( windNbr == 0)											\
			SETUpToAndClearLoOrder( off, lastBit, row, temp)		\
		else if( windNbr == -1 )									\
			CLEARUpToAndSetLoOrder( off, lastBit, row, temp)		\
	}																\
}																	\
*row = temp;														\
while( ++row < rowEnd ) *row = 0; 									\
}																	\
/* Winding number fill of nRows of a glyph beginning at rowM, using two sorted 
arrays of onTransitions and offTransitions. 
*/

local void sc_wnNrowFill( tt_int32 rowM, tt_int16 nRows, sc_BitMapData *bbox )
{
	tt_uint32  longsWide = bbox->wide>>5;
	tt_uint32  lineWide = bbox->nYchanges + 2;
	tt_uint32 *rowB = bbox->bitMap;
	tt_int16  *lineB = *(bbox->yBase + rowM + nRows - 1);
	tt_int32   xMin = bbox->xMin;
	while( nRows-- > 0 )
	{
		tt_uint32 *row = rowB;
		tt_int16 *gxLine = lineB;
		FILLONEROW( row, longsWide, gxLine, lineWide, xMin )
		rowB += longsWide;
		lineB -= lineWide;
	}
}
 #undef NEGONE

/* Sort the values stored in locations pBeg to pBeg+nVal in ascending order
*/
#define ISORT( pBeg, pVal )								\
{														\
	register tt_int16 *pj = pBeg;							\
	register tt_int16 nVal = *pVal - 2;					\
	for( ; nVal >= 0; --nVal )							\
	{													\
		register tt_int16 v;								\
		register tt_int16 *pk, *pi;						\
														\
		pk = pj;										\
		pi = ++pj;										\
		v = *pj;										\
		while( *pk > v && pk >= pBeg ) *pi-- = *pk--;	\
		*pi = v;										\
	}													\
}														\

/* rwb 4/5/90 Sort OnTransition and OffTransitions in Xlines arrays */
static void sortCols( sc_BitMapData *bbox )
{
register tt_int16 nrows = bbox->xMax - bbox->xMin - 1;
register tt_int16 *p = bbox->xLines;
register tt_uint32 n = bbox->nXchanges + 1; 			/*<9>*/

	for(; nrows >= 0; --nrows )
	{
		ISORT( p+1, p );
		p += n;										/*<9>*/
		ISORT( p-*p, p );
		++p;
	}
}

/* rwb 4/5/90 Sort OnTransition and OffTransitions in Ylines arrays */
local void sortRows( sc_BitMapData *bbox, tt_int16** lowRowP, tt_int16** highRowP )
{
register tt_uint32 n = bbox->nYchanges + 1; 			/*<9>*/
tt_int16 *p, *pend;

if( highRowP < lowRowP ) return;
p = *lowRowP;
pend = *highRowP;
do
{
	ISORT( p+1, p );
	p += n;											/*<9>*/
	ISORT( p-*p, p );
	++p; 
}
while(p <= pend);
}

/* Generate scan gxLine intersections from series of vectors.  Always need previous
vector to properly generate data for current vector.  pend typically points
one beyond last valid vector endpoint.  4/6/90
*/
local void sc_lineGen(sc_BitMapData *bbox, tt_int32 *px, tt_int32 *py, tt_int32 *pend,
		tt_int16 **xBase, tt_int16 **yBase )
{
tt_int32 x0, x1, y0, y1, xb, yb;
x0 = *px++;
y0 = *py++;
x1 = *px++;
y1 = *py++;
while (py <= pend)
{
	xb = x0;
	yb = y0;
	x0 = x1;
	y0 = y1;
	x1 = *px++;
	y1 = *py++;
	sc_mark( x0, y0, x1, y1, xb, yb, xBase, yBase, bbox);
}
}

/* Generate row scan gxLine intersections from series of vectors.  Always need previous
vector to properly generate data for current vector.  pend typically points
one beyond last valid vector endpoint.  4/6/90
*/
local void sc_lineGenRows(sc_BitMapData *bbox, tt_int32 *px, tt_int32 *py, tt_int32 *pend,
	 tt_int16 **yBase, tt_int16** lowRowP, tt_int16** highRowP )
{
tt_int32 x0, x1, y0, y1, xb, yb;

if( highRowP < lowRowP ) return;

x0 = *px++;
y0 = *py++;
x1 = *px++;
y1 = *py++;
while (py <= pend)
{
	xb = x0;
	yb = y0;
	x0 = x1;
	y0 = y1;
	x1 = *px++;
	y1 = *py++;
	sc_markRows( x0, y0, x1, y1, xb, yb, yBase, bbox, lowRowP, highRowP);
}
}


/* 4/4/90 Version that distinguishes between On transitions and Off transitions.
*/
/* 3/23/90 
* 	A procedure to find and mark all of the scan lines (both row and column ) that are
* crossed by a vector.  Many different cases must be considered according to the direction
* of the vector, whether it is vertical or slanted, etc.  In each case, the vector is first
* examined to see if it starts on a scan-gxLine.  If so, special markings are made and the
* starting conditions are adjusted.  If the vector ends on a scan gxLine, the ending 
* conditions must be adjusted.  Then the body of the case is done.
* 	Special adjustments must be made when a vector starts or ends on a scan gxLine. Whenever
* one vector starts on a scan gxLine, the previous vector must have ended on a scan gxLine.
* Generally, this should result in the gxLine being marked as crossed only once (conceptually
* by the vector that starts on the gxLine.  But, if the two lines form a vertex that
* includes the vertex in a colored region, the gxLine should be marked twice.  If the 
* vertex is also on a perpendicular scan gxLine, the marked scan gxLine should be marked once
* on each side of the perpendicular gxLine.  If the vertex defines a point that is jutting
* into a colored region, then the gxLine should not be marked at all. In order to make
* these vertex crossing decisions, the previous vector must be examined.
*	
*	Because many vectors are short with respect to the grid for small resolutions, the
* procedure first looks for simple cases in which no lines are crossed.
*
* xb, x0, and x1 are x coordinates of previous point, current point and next point
* similaryly yb, y0 and y1
* 	ybase points to an array of pointers, each of which points to an array containing
* information about the glyph contour crossings of a horizontal scan-gxLine.  The first
* entry in these arrays is the number of ON-transition crossings, followed by the y
* coordinates of each of those crossings.  The last entry in each array is the number of
* OFF-transtion crossings, preceded by the Y coordinates for each of these crossings.
* 	xBase contains the same information for the column scan lines.
*/
#define DROUND(a) ((a + HALFM) & INTPART)
#define RSH(a) (tt_int16)(a>>PIXSHIFT)
#define LSH(a) (a<<PIXSHIFT)
#define LINE(a)   ( !((a & FRACPART) ^ HALF))
#define SET(p,val) {register tt_int16 *row = *p; ++*row; *(row+*row)=val;}
#define OFFX(val) {register tt_int16 *s = *px+wideX; ++*s; *(s-*s) = val;}
#define OFFY(val) {register tt_int16 *s = *py+wideY; ++*s; *(s-*s) = val;}
#define BETWEEN(a,b,c) (a < b && b < c )
#define INTERIOR  ((x0-xb)*dy < (y0-yb)*dx)
#define ZERO( prev, mid, next, more, less ) (prev == mid && next == mid && more > less )

local void sc_mark( tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 xb, tt_int32 yb,
tt_int16 **xBase, tt_int16 **yBase, sc_BitMapData *bbox )
{
tt_int16 jx, jy, endx, endy, onrow, oncol, wideX, wideY;
register tt_int16 **px, **py;
tt_int16 **pend;
tt_int32 rx0, ry0, rx1, ry1, dy, dx, rhi, rlo;
register tt_int32  r, incX, incY;

rx0 = DROUND( x0 );
jx = RSH( rx0 );
ry0 = DROUND( y0 );
jy = RSH( ry0 );
rx1 = DROUND( x1 );
endx = RSH( rx1 );
ry1 = DROUND( y1 );
endy = RSH( ry1 );
py = yBase + jy;
px = xBase + jx;
dy = y1 - y0;
dx = x1 - x0;
onrow = false;
oncol = false;
wideX = bbox->nXchanges+1;
wideY = bbox->nYchanges+1;

if( dy >= 0 && dx > 0 )					/* FIRST QUADRANT CASE include ---> */
{
	if LINE(y0)								/* vector starts on row scan gxLine */
	{				
		onrow = true;
		if LINE(x0)							/* can also start on column scan gxLine */
		{
			oncol = true;
			if( INTERIOR || ZERO( yb, y0, y1, xb, x0 ) )
			{
				SET( py, jx )
				OFFX( jy+1 )
				if( xb > x0 ) SET( px, jy )
				if( yb > y0 ) OFFY( jx+1 )
			}
			else
			{
				if BETWEEN(xb, x0, x1) OFFX( jy+1 )
				if BETWEEN(yb, y0, y1) SET( py, jx )
			}
		}
		else								/* starts on row scan but not col */
		{
			if( INTERIOR || ZERO( yb, y0, y1, xb, x0 ))
			{
				SET( py, jx )
				if( yb > y0 ) OFFY( jx )
			}
			else if BETWEEN( yb, y0, y1 ) SET( py, jx )
		}
				
	}
	else if LINE(x0)						/* starts on col scan gxLine but not row */
	{
		oncol = true;
		if( INTERIOR || ZERO( yb, y0, y1, xb, x0 ))
		{
			OFFX( jy )
			if( xb > x0 ) SET( px, jy )		/* cross extremum vertex twice */
		}
		else if BETWEEN( xb, x0, x1 ) OFFX( jy )
	}
	
	if( endy == jy )						/* horizontal gxLine */
	{
		if( endx == jx ) return;
		if( onrow ) ++jy;
		if( oncol ) ++px;
		pend = xBase + endx;
		while( px < pend )
		{
			OFFX( jy )
			++px;
		}
		return;
	}
	
	if( endx == jx )
	{
		pend = yBase + endy;
		if( onrow ) ++py;
		while( py < pend )					/* note oncol can't be true */
		{
			SET( py, jx )
			++py;
		}
		return;
	}
	
	if(! onrow ) rhi = (ry0 - y0 + HALF) * dx;
	else
	{
		rhi = LSH( dx );
		++jy;
		++py;
	}
	if(! oncol ) rlo =  (rx0 - x0 + HALF) * dy;
	else
	{
		rlo = LSH( dy );
		++jx;
		++px;
	}
	r = rhi - rlo;
	incX = LSH( dx );
	incY = LSH( dy );
	do 
	{
		if( r > 0) 
		{
			if( jx == endx ) break;
			OFFX( jy );
			++px;
			++jx;
			r -= incY;
		}
		else
		{
			if( jy == endy ) break;
			SET( py, jx );
			++py;
			++jy;
			r += incX;
		}
	} while (true );
}	
else if( dy > 0 && dx <= 0 )					/* SECOND QUADRANT CASE include vertical up*/
	{
		if LINE(y0)								/* vector starts on row scan gxLine */
		{				
			onrow = true;
			if LINE(x0)							/* can also start on column scan gxLine */
			{
				oncol = true;
				if (INTERIOR || ZERO( xb, x0, x1, yb, y0 ))
				{
					SET( py, jx )
					SET( px, jy )
					if( xb < x0 ) OFFX( jy+1 )
					if( yb > y0 ) OFFY( jx+1 )
				}
				else
				{
					if BETWEEN(yb, y0, y1) SET( py, jx );
					if BETWEEN(x1, x0, xb) SET( px, jy );
				}
			}
			else								/* starts on row scan but not col */
			{
				if( INTERIOR || ZERO( xb, x0, x1, yb, y0 ))
				{
					SET( py, jx )
					if( yb > y0 ) OFFY( jx )
				}
				else if BETWEEN(yb, y0, y1) SET( py, jx)
			}
		}
		else if LINE(x0)						/* starts on col scan gxLine but not row */
		{
			oncol = true;
			if( INTERIOR || ZERO( xb, x0, x1, yb, y0 ))
			{
				SET( px, jy )
				if( xb < x0 ) OFFX( jy )		/* cross extremum vertex twice  */ 
			}
			else if BETWEEN( x1, x0, xb ) SET( px, jy )
		}
		
		if( endy == jy )
		{
			if( endx == jx ) return;
			if LINE(x1) ++endx;
			pend = xBase + endx;
			--px;
			while( px >= pend )					
			{
				SET( px, jy )
				--px;
			}
			return;
		}
		
		if( endx == jx )
		{
			pend = yBase + endy;
			if( onrow ) ++py;
			while( py < pend )					
			{
				SET( py, jx )
				++py;
			}
			return;
		}
		
		if(! onrow ) rhi = (ry0 - y0 + HALF) * dx;
		else
		{
			rhi = LSH( dx );
			++jy;
			++py;
		}
		if(! oncol ) rlo = (rx0 - x0 - HALF) * dy;
			else rlo = -LSH( dy );
			
		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(x1) ++endx;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r <= 0) 
			{
				--jx;
				--px;
				if( jx < endx ) break;
				SET( px, jy );
				r += incY;
			}
			else
			{
				if( jy == endy ) break;
				SET( py, jx );
				++py;
				++jy;
				r += incX;
			}
		} while (true );
	}	
	else if( dy <= 0 && dx < 0 )				/* THIRD QUADRANT CASE includes <--- */
	{
		if LINE(y0)								/* vector starts on row scan gxLine */
		{				
			onrow = true;
			if LINE(x0)							/* can also start on column scan gxLine */
			{
				oncol = true;
				if( INTERIOR || ZERO( yb, y0, y1, x0, xb ))
				{
					OFFY( jx+1 )
					SET( px, jy )
					if( xb < x0 ) OFFX( jy+1 )
					if( yb < y0 ) SET( py, jx )
				}
				else
				{
					if BETWEEN(y1, y0, yb) OFFY( jx+1 );
					if BETWEEN(x1, x0, xb) SET( px, jy );
				}
			}
			else								/* starts on row scan but not col */
			{
				if( INTERIOR  || ZERO( yb, y0, y1, x0, xb ))
				{
					OFFY( jx )
					if( yb < y0 ) SET( py, jx )
				}
				else if BETWEEN(y1, y0, yb) OFFY( jx)
			}
		}
		else if LINE(x0)						/* starts on col scan gxLine but not row */
		{
			oncol = true;
			if( INTERIOR  || ZERO( yb, y0, y1, x0, xb ))
			{
				SET( px, jy )
				if( xb < x0 ) OFFX( jy )		/* cross extremum vertex twice  */ 
			}
			else if BETWEEN( x1, x0, xb ) SET( px, jy )
		}
		
		if( endy == jy )
		{
			if( endx == jx ) return;
			if LINE(x1) ++endx;
			pend = xBase + endx;
			--px;
			while( px >= pend )					
			{
				SET( px, jy )
				--px;
			}
			return;
		}
		
		if( endx == jx )
		{
			if LINE( y1 ) ++endy;
			pend = yBase + endy;
			--py;
			while( py >= pend )					
			{
				OFFY( jx )
				--py;
			}
			return;
		}
		
		if(! onrow ) rhi = (ry0 - y0 - HALF) * dx;
			else rhi = -LSH( dx );
		
		if(! oncol ) rlo = (rx0 - x0 - HALF) * dy;
			else rlo = -LSH( dy );
		
		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(y1) ++endy;						/* ends on scan gxLine but dont mark it */
		if LINE(x1) ++endx;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r > 0) 
			{
				--jx;
				--px;
				if( jx < endx ) break;
				SET( px, jy );
				r += incY;
			}
			else
			{
				--jy;
				--py;
				if( jy < endy ) break;
				OFFY( jx );
				r -= incX;
			}
		} while (true );
	}	
	else if( dy < 0 && dx >= 0 )					/* FOURTH QUADRANT CASE includes vertical down */
	{
		if LINE(y0)								/* vector starts on row scan gxLine */
		{				
			onrow = true;
			if LINE(x0)							/* can also start on column scan gxLine */
			{
				oncol = true;
				if( INTERIOR || ZERO( xb, x0, x1, y0, yb ))
				{
					OFFY( jx+1 )
					OFFX( jy+1 )
					if( xb > x0 ) SET( px, jy )
					if( yb < y0 ) SET( py, jx )
				}
				else
				{
					if BETWEEN(y1, y0, yb) OFFY( jx+1 );
					if BETWEEN(xb, x0, x1) OFFX( jy+1 );
				}
			}
			else								/* starts on row scan but not col */
			{
				if( INTERIOR || ZERO( xb, x0, x1, y0, yb ))
				{
					OFFY( jx )
					if( yb < y0 ) SET( py, jx )
				}
				else if BETWEEN(y1, y0, yb) OFFY( jx)
			}
		}
		else if LINE(x0)						/* starts on col scan gxLine but not row */
		{
			oncol = true;
			if( INTERIOR || ZERO( xb, x0, x1, y0, yb ))
			{
				OFFX( jy )
				if( xb > x0 ) SET( px, jy )		/* cross extremum vertex twice  */ 
			}
			else if BETWEEN( xb, x0, x1 ) OFFX( jy )
		}
		
		if( endy == jy )
		{
			if( endx == jx ) return;
			pend = xBase + endx;
			if( oncol ) ++px;
			while( px < pend )
			{
				OFFX( jy )
				++px;
			}
			return;
		}
		
		if( endx == jx )
		{
			if( oncol ) ++jx;					/* straight down gxLine on col scan gxLine */
			if LINE( y1 ) ++endy;
			pend = yBase + endy;
			--py;
			while( py >= pend )					
			{
				OFFY( jx )
				--py;
			}
			return;
		}
		if(! onrow ) rhi = (ry0 - y0 - HALF) * dx;
			else rhi = -LSH( dx );
		
		if(! oncol ) rlo = (rx0 - x0 + HALF) * dy;
		else
		{
			rlo = LSH( dy );
			++jx;
			++px;
		}

		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(y1) ++endy;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r <= 0) 
			{
				if( jx == endx ) break;
				OFFX( jy );
				++px;
				++jx;
				r -= incY;
			}
			else
			{
				--jy;
				--py;
				if( jy < endy ) break;
				OFFY( jx );
				r -= incX;
			}
		} while (true );
	}		
}

/* 4/6/90 Version for banding and no dropout control 
* Same Routine as sc_mark,
* EXCEPT No column scan lines are marked and only rows within band are marked
* To do banding and dropout control, use sc_mark to mark all the crossings, even
*	though only one band of the bit map will be filled.
*
* x0,y0 is the starting point of the vector.
* x1,y1 is the ending point of the vector.
* xb,yb is the starting point of the previous connecting vector.
* xBase is not used in the rows only version of this routine.
* yBase points at the array of pointers to the glyph scan intersection arrays.
* bbox points at the gxBitmap struture that has all the relevant pointers.
* lowRowP is the pointer to the lowest glyph scan row to be processed by this call.
* highRowP is the pointer to the highest glyph scan row to be processed by this call.
* 
*/

#undef	DROUND
#undef	RSH
#undef	LSH
#undef	LINE
#undef	SET
#undef	OFFY
#undef	BETWEEN
#undef	INTERIOR
#undef  ZERO

#define DROUND(a) ((a + HALFM) & INTPART)
#define RSH(a) (tt_int16)(a>>PIXSHIFT)
#define LSH(a) (a<<PIXSHIFT)
#define LINE(a)   ( !((a & FRACPART) ^ HALF))
#define SET(p,val) {register tt_int16 *t = *p; ++*t; *(t+*t)=val;}
#define OFFY(val) {register tt_int16 *s = *py+wideY; ++*s; *(s-*s) = val;}
#define BETWEEN(a,b,c) (a < b && b < c )
#define INTERIOR  ((x0-xb)*dy < (y0-yb)*dx)
#define ZERO( prev, mid, next, more, less ) (prev == mid && next == mid && more > less )

local void sc_markRows( tt_int32 x0, tt_int32 y0, tt_int32 x1, tt_int32 y1, tt_int32 xb, tt_int32 yb,
tt_int16 **yBase, sc_BitMapData *bbox, tt_int16** lowRowP, tt_int16** highRowP )
{
tt_int16 jx, jy, endx, endy, onrow, oncol, wideX, wideY;
register tt_int16 **py, *row;
tt_int16 **pend;
tt_int32 rx0, ry0, rx1, ry1, dy, dx, rhi, rlo;
register tt_int32  r, incX, incY;

rx0 = DROUND( x0 );
jx = RSH( rx0 );
ry0 = DROUND( y0 );
jy = RSH( ry0 );
rx1 = DROUND( x1 );
endx = RSH( rx1 );
ry1 = DROUND( y1 );
endy = RSH( ry1 );
py = yBase + jy;
pend = yBase + endy;
dy = y1 - y0;
dx = x1 - x0;
onrow = false;
oncol = false;
wideX = bbox->nXchanges+1;
wideY = bbox->nYchanges+1;

if( dy >= 0 && dx > 0 )					/* FIRST QUADRANT CASE include ---> */
{
	if( py > highRowP || pend < lowRowP ) return;
	if( py >= lowRowP )
	{
		if LINE(y0)								/* vector starts on row scan gxLine */
		{				
			onrow = true;
			if LINE(x0)							/* can also start on column scan gxLine */
			{
				oncol = true;
				if( INTERIOR || ZERO( yb, y0, y1, xb, x0 ))
				{
					SET( py, jx )
					if( yb > y0 ) OFFY( jx+1 )
				}
				else
				{
					if BETWEEN(yb, y0, y1) SET( py, jx )
				}
			}
			else								/* starts on row scan but not col */
			{
				if( INTERIOR || ZERO( yb, y0, y1, xb, x0 ))
				{
					SET( py, jx )
					if( yb > y0 ) OFFY( jx )
				}
				else if BETWEEN( yb, y0, y1 ) SET( py, jx )
			}
					
		}
		else if LINE(x0) oncol = true;				/* starts on col scan gxLine but not row */
	}
	if( endy == jy ) return;						/* horizontal gxLine */
	
	if( endx == jx )
	{
		if( pend > highRowP ) pend = highRowP+1;
		if( onrow ) ++py;
		while( py < pend )					/* note oncol can't be true */
		{
			if( py >= lowRowP )
			{
				row = *py;
				++*row;
				*(row+*row) = jx;
			}
			++py;
		}
		return;
	}
	
	if(! onrow ) rhi = (ry0 - y0 + HALF) * dx;
	else
	{
		rhi = LSH( dx );
		++jy;
		++py;
	}
	if(! oncol ) rlo = (rx0 - x0 + HALF) * dy;
	else
	{
		rlo = LSH( dy );
		++jx;
	}
	r = rhi - rlo;
	incX = LSH( dx );
	incY = LSH( dy );
	do 
	{
		if( r > 0) 
		{
			if( jx == endx ) return;
			++jx;
			r -= incY;
		}
		else
		{
			if( jy == endy ) return;
			if( py > highRowP ) return;
			if( py >= lowRowP )
			{
				row = *py;
				++*row;
				*(row+*row) = jx;
			}
			++py;
			++jy;
			r += incX;
		}
	} while (true );
}	
else if( dy > 0 && dx <= 0 )					/* SECOND QUADRANT CASE include vertical up*/
	{
		if( py > highRowP || pend < lowRowP ) return;
		if( py >= lowRowP )
		{
			if LINE(y0)								/* vector starts on row scan gxLine */
			{
				onrow = true;
				if LINE(x0)							/* can also start on column scan gxLine */
				{
					oncol = true;
					if( INTERIOR || ZERO( xb, x0, x1, yb, y0 ))
					{
						SET( py, jx )
						if( yb > y0 ) OFFY( jx+1 )
					}
					else
					{
						if BETWEEN(yb, y0, y1) SET( py, jx );
					}
				}
				else								/* starts on row scan but not col */
				{
					if( INTERIOR || ZERO( xb, x0, x1, yb, y0 ))
					{
						SET( py, jx )
						if( yb > y0 ) OFFY( jx )
					}
					else if BETWEEN(yb, y0, y1) SET( py, jx)
				}
			}
			else if LINE(x0) oncol = true;				/* starts on col scan gxLine but not row */
		}
		if( endy == jy ) return;
		
		if( endx == jx )
		{
			if( pend > highRowP ) pend = highRowP+1;
			if( onrow ) ++py;
			while( py < pend )					
			{
				if( py >= lowRowP )
				{
					row = *py;
					++*row;
					*(row+*row) = jx;
				}
				++py;
			}
			return;
		}
		
		if(! onrow ) rhi = (ry0 - y0 + HALF) * dx;
		else
		{
			rhi = LSH( dx );
			++jy;
			++py;
		}
		if(! oncol ) rlo = (rx0 - x0 - HALF) * dy;
			else rlo = -LSH( dy );
			
		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(x1) ++endx;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r <= 0) 
			{
				--jx;
				if( jx < endx ) return;
				r += incY;
			}
			else
			{
				if( jy == endy ) return;
				if( py > highRowP ) return;
				if( py >= lowRowP )
				{
					row = *py;
					++*row;
					*(row+*row) = jx;
				}
				++py;
				++jy;
				r += incX;
			}
		} while (true );
	}	
	else if( dy <= 0 && dx < 0 )				/* THIRD QUADRANT CASE includes <--- */
	{
		if( py < lowRowP || pend > highRowP ) return;
		if( py <= highRowP )
		{
			if LINE(y0)								/* vector starts on row scan gxLine */
			{				
				onrow = true;
				if LINE(x0)							/* can also start on column scan gxLine */
				{
					oncol = true;
					if( INTERIOR || ZERO( yb, y0, y1, x0, xb ))
					{
						OFFY( jx+1 )
						if( yb < y0 ) SET( py, jx )
					}
					else
					{
						if BETWEEN(y1, y0, yb) OFFY( jx+1 );
					}
				}
				else								/* starts on row scan but not col */
				{
					if( INTERIOR || ZERO( yb, y0, y1, x0, xb ))
					{
						OFFY( jx )
						if( yb < y0 ) SET( py, jx )
					}
					else if BETWEEN(y1, y0, yb) OFFY( jx)
				}
			}
			else if LINE(x0) oncol = true;				/* starts on col scan gxLine but not row */
		}
		if( endy == jy ) return;

		if( endx == jx )
		{
			if LINE( y1 ) ++endy;
			pend = yBase + endy;
			if( pend < lowRowP ) pend = lowRowP;
			--py;
			while( py >= pend )					
			{
				if( py <= highRowP ) OFFY( jx )
				--py;
			}
			return;
		}
		
		if(! onrow ) rhi = (ry0 - y0 - HALF) * dx;
			else rhi = -LSH( dx );
		
		if(! oncol ) rlo = (rx0 - x0 - HALF) * dy;
			else rlo = -LSH( dy );
		
		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(y1) ++endy;						/* ends on scan gxLine but dont mark it */
		if LINE(x1) ++endx;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r > 0) 
			{
				--jx;
				if( jx < endx ) return;
				r += incY;
			}
			else
			{
				--jy;
				--py;
				if( jy < endy ) return;
				if( py < lowRowP ) return;
				if( py <= highRowP ) OFFY( jx );
				r -= incX;
			}
		} while (true );
	}	
	else if( dy < 0 && dx >= 0 )					/* FOURTH QUADRANT CASE includes vertical down */
	{
		if( py < lowRowP || pend > highRowP ) return;
		if( py <= highRowP )
		{
			if LINE(y0)								/* vector starts on row scan gxLine */
			{				
				onrow = true;
				if LINE(x0)							/* can also start on column scan gxLine */
				{
					oncol = true;
					if( INTERIOR || ZERO( xb, x0, x1, y0, yb ))
					{
						OFFY( jx+1 )
						if( yb < y0 ) SET( py, jx )
					}
					else
					{
						if BETWEEN(y1, y0, yb) OFFY( jx+1 );
					}
				}
				else								/* starts on row scan but not col */
				{
					if( INTERIOR || ZERO( xb, x0, x1, y0, yb ))
					{
						OFFY( jx )
						if( yb < y0 ) SET( py, jx )
					}
					else if BETWEEN(y1, y0, yb) OFFY( jx)
				}
			}
			else if LINE(x0) oncol = true;				/* starts on col scan gxLine but not row */
		}
		
		if( endy == jy ) return;
		
		if( endx == jx )
		{
			if( oncol ) ++jx;					/* straight down gxLine on col scan gxLine */
			if LINE( y1 ) ++endy;
			pend = yBase + endy;
			if( pend < lowRowP ) pend = lowRowP;
			--py;
			while( py >= pend )					
			{
				if( py <= highRowP ) OFFY( jx )
				--py;
			}
			return;
		}
		
		if(! onrow ) rhi = (ry0 - y0 - HALF) * dx;
			else rhi = -LSH( dx );
		
		if(! oncol ) rlo = (rx0 - x0 + HALF) * dy;
		else
		{
			rlo = LSH( dy );
			++jx;
		}

		r = rhi - rlo;
		incX = LSH( dx );
		incY = LSH( dy );
		if LINE(y1) ++endy;						/* ends on scan gxLine but dont mark it */
		do 
		{
			if( r <= 0) 
			{
				if( jx == endx ) break;
				++jx;
				r -= incY;
			}
			else
			{
				--jy;
				--py;
				if( jy < endy ) break;
				if( py < lowRowP ) return;
				if( py <= highRowP ) OFFY( jx );
				r -= incX;
			}
		} while (true );
	}		
}
#undef	DROUND
#undef	RSH
#undef	LSH
#undef	LINE
#undef	SET
#undef	OFFY
#undef	BETWEEN
#undef	INTERIOR
#undef  ZERO


/* new version 4/4/90 - winding number version assumes that the On transitions are
int the first half of the array, and the Off transitions are in the second half.  Also
assumes that the number of on transitions is in array[0] and the number of off transitions
is in array[n].
*/

/* New version 3/10/90 
Using the crossing information, look for segments that are crossed twice.  First
do Y lines, then do X lines.  For each found segment, look at the three lines in
the more positive adjoining segments.  If there are at least two crossings
of these lines, there is a dropout that needs to be fixed, so fix it.  If the bit on 
either side of the segment is on, quit; else turn the leastmost of the two pixels on.
*/ 

static void sc_orSomeBits( sc_BitMapData *bbox, tt_int32 scanKind)
{
tt_int16 ymin, ymax, xmin, xmax;
register tt_int16 **yBase, **xBase;											/*<9>*/
register tt_int16 scanline, coordOn,
	coordOff=0, /*initialized to avoid warning in Visual C++ */
	nIntOn, nIntOff;
tt_uint32 *bitmapP, *scanP;
tt_int16  *rowPt, longsWide, *pOn, *pOff, *pOff2;
tt_int16 index, incY, incX;
tt_int32 upper, lower;

scanKind &= STUBCONTROL;
ymin = bbox->yMin;
ymax = bbox->yMax-1;
xmin = bbox->xMin;
xmax = bbox->xMax-1;
xBase = bbox->xBase;
yBase = bbox->yBase;
longsWide = bbox->wide >> 5;
if( longsWide == 1 ) bitmapP = bbox->bitMap + bbox->high - 1;
else bitmapP = bbox->bitMap + longsWide*(bbox->high-1);

/* First do Y scanlines
*/
scanP = bitmapP;
incY = bbox->nYchanges + 2;
incX = bbox->nXchanges + 2;
rowPt = *(yBase + ymin );
for (scanline = ymin; scanline <= ymax; ++scanline)
{
	nIntOn = *rowPt;
	nIntOff = *(rowPt + incY - 1);
	pOn = rowPt+1;
	pOff = rowPt + incY - 1 - nIntOff;
	while(nIntOn--)
	{
		coordOn = *pOn++;
		index = nIntOff;
		pOff2 = pOff;
		while (index-- && ((coordOff = *pOff2++) < coordOn)) {};
		
		if( coordOn == coordOff )  /* This segment was crossed twice  */
		{
			if( scanKind )
			{
				upper = nUpperXings(yBase, xBase, scanline, coordOn,
						    (tt_int16)(incY-2), (tt_int16)(incX-2),
						    xmin,(tt_int16)(xmax+1), ymax ); /*<14>*/ 
				lower = nLowerXings(yBase, xBase, scanline, coordOn,
						    (tt_int16)(incY-2), (tt_int16)(incX-2),
						    xmin, (tt_int16)(xmax+1), ymin ); /*<14>*/
				if( upper < 2 || lower < 2 ) continue;
			}
			if(coordOn > xmax  ) invpixOn( (tt_int16)(xmax-xmin), longsWide, scanP );
			else if(coordOn == xmin  ) invpixOn( 0, longsWide, scanP );
			else invpixSegY( (tt_int16)(coordOn-xmin-1), longsWide, scanP );
		}
	}
	rowPt += incY;
	scanP -= longsWide;
}
/* Next do X scanlines */
rowPt = *(xBase + xmin);
for (scanline = xmin ; scanline <= xmax; ++scanline) 
{
	nIntOn = *rowPt;
	nIntOff = *(rowPt + incX - 1);
	pOn = rowPt+1;
	pOff = rowPt + incX - 1 - nIntOff;
	while(nIntOn--)
	{
		coordOn = *pOn++;
		index = nIntOff;
		pOff2 = pOff;
		while (index-- && ((coordOff = *pOff2++) < coordOn)) {};
		if( coordOn == coordOff )
		{
			if( scanKind )
			{
				upper = nUpperXings(xBase, yBase, scanline, coordOn,
						    (tt_int16)(incX-2), (tt_int16)(incY-2),
						    ymin, (tt_int16)(ymax+1), xmax ); /*<14>*/
				lower = nLowerXings(xBase, yBase, scanline, coordOn,
						    (tt_int16)(incX-2), (tt_int16)(incY-2),
						    ymin, (tt_int16)(ymax+1), xmin); /*<14>*/
				if( upper < 2 || lower < 2 ) continue;
			}
			if(coordOn > ymax  ) invpixOn( (tt_int16)(scanline-xmin), longsWide,
						       bitmapP - longsWide*(ymax-ymin) );
			else if(coordOn == ymin ) invpixOn( (tt_int16)(scanline-xmin), longsWide, bitmapP );
			else invpixSegX( (tt_int16)(scanline-xmin), longsWide, bitmapP - longsWide*(coordOn-ymin-1) );
		}
	}
	rowPt += incX;
}
}

/* Pixel oring to fix dropouts   *** inverted gxBitmap version ***
See if the bit on either side of the Y gxLine segment is on, if so return,
else turn on the leftmost bit.

Bitmap array is always K longs wide by H rows high.

Bit locations are numbered 0 to H-1 from top to bottom
and from 0 to 32*K-1 from left to right; gxBitmap pointer points to 0,0, and
all of the columns for one row are stored adjacently.
*/

static void invpixSegY(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP )
{
register tt_uint32 maskL, maskR;								/*<9>*/
	if (k == 1)
	{
		maskL = 0x40000000;
		maskL >>= llx;
		if( *bitmapP & maskL) return;
		maskL <<= 1;
		*bitmapP |= maskL;
	}
	else
	{
		register tt_uint8  nBits = (tt_uint8)(llx & 0x1f);		/*<9>*/
		maskL = 0x80000000;
		maskL >>= nBits; 
		bitmapP += (llx >>5);
		if( *bitmapP & maskL) return;
		if(nBits < 31)
		{
			maskR = maskL >> 1;
			if(*bitmapP & maskR) return;	
			*bitmapP |= maskL;	
		}
		else
		{
			maskR = 0x80000000;
			++bitmapP;
			if(*bitmapP & maskR) return;	
			--bitmapP;	
			*bitmapP |= maskL;	
		}
	}
}
/* Pixel oring to fix dropouts   *** inverted gxBitmap version ***
See if the bit on either side of the X gxLine segment is on, if so return,
else turn on the bottommost bit.

Temporarily assume gxBitmap is set up as in Sampo Converter. 
Bitmap array is always K longs wide by H rows high.

For now, assume bit locations are numbered 0 to H-1 from top to bottom
and from 0 to 32*K-1 from left to right; and that gxBitmap pointer points to 0,0, and
all of the columns for one row are stored adjacently.
*/

static void invpixSegX(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP )
{
register tt_uint32 maskL = 0x80000000;						/*<9>*/
	bitmapP -= k;
	if (k == 1)	maskL >>= llx;
	else
	{
		maskL >>= (unsigned)(llx & 0x1f); 	/*<9>*/
		bitmapP += (llx >>5);
	}
	if( *bitmapP & maskL) return;
	bitmapP += k;
	*bitmapP |= maskL;
}

/* Pixel oring to fix dropouts    ***inverted gxBitmap version ***
This code is used to orin dropouts when we are on the boundary of the gxBitmap.
The bit at llx, lly is colored.

Temporarily assume gxBitmap is set up as in Sampo Converter. 
Bitmap array is always K longs wide by H rows high.

For now, assume bit locations are numbered 0 to H-1 from top to bottom
and from 0 to 32*K-1 from left to right; and that gxBitmap pointer points to 0,0, and
all of the columns for one row are stored adjacently.
*/
static void invpixOn(tt_int16 llx, tt_uint16 k, tt_uint32* bitmapP )
{
register tt_uint32 maskL = 0x80000000;						/*<9>*/
	if (k == 1)	maskL >>= llx;
	else
	{
		maskL >>= (unsigned)(llx & 0x1f);				/*<9>*/
		bitmapP += (llx >>5);
	}
	*bitmapP |= maskL;
}


/* Initialize a two dimensional array that will contain the coordinates of 
gxLine segments that are intersected by scan lines for a simple glyph.  Return
a biased pointer to the array containing the row pointers, so that they can
be accessed without subtracting a minimum value.
	Always reserve room for at least 1 scanline and 2 crossings 
*/
static tt_int16** sc_lineInit( tt_int16* arrayBase, tt_int16** rowBase, tt_int16 nScanlines, tt_int16 maxCrossings,
	tt_int16 minScanline )
{
tt_int16** bias;
register short count = nScanlines;
	if( count ) --count;
	bias = rowBase - minScanline;
	maxCrossings += 1;
	for(; count>=0; --count)
	{
	*rowBase++ = arrayBase;
	*arrayBase = 0;
	arrayBase += maxCrossings;
	*arrayBase++ = 0;
	}
return bias;
}

/* Check the kth scanline (indexed from base) and count the number of onTransition and
 * offTransition contour crossings at the gxLine segment val.  Count only one of each
 * kind of transition, so maximum return value is two. 
 */ 				
static tt_int32 nOnOff( tt_int16** base, tt_int32 k, tt_int16 val, tt_int32 nChanges )
{
register tt_int16* rowP = *(base + k);
register tt_int16* endP = (rowP + *rowP + 1);
register tt_int32 count = 0;
register tt_int16 v;

while(++rowP < endP)
{
	if( (v = *rowP) == val)
	{
		++count;
		break;
	}
	if( v > val ) break;
}
rowP = *(base + k) + nChanges + 1;
endP = (rowP - *rowP - 1);
while(--rowP > endP)
{
	if( (v = *rowP) == val) return ++count;
	if( v < val ) break;
}
return count;
}
/* 8/22/90 - added valMin and valMax checks */
/* See if the 3 gxLine segments on the edge of the more positive quadrant are cut by at
 * least 2 contour lines.
 */

static tt_int32 nUpperXings(tt_int16** lineBase, tt_int16** valBase, tt_int16 gxLine, tt_int16 val,
						tt_int16 lineChanges, tt_int16 valChanges, tt_int16 valMin, tt_int16 valMax, tt_int16 lineMax) /*<14>*/
{
register tt_int32 count = 0;

if( gxLine < lineMax ) count += nOnOff( lineBase, gxLine+1, val, lineChanges ); 	/*<14>*/
if( count > 1 ) return count;
	else if( val > valMin ) count += nOnOff( valBase, val-1, (tt_int16)(gxLine+1), valChanges );
if( count > 1 ) return count;
	else if( val < valMax ) count += nOnOff( valBase, val, (tt_int16)(gxLine+1), valChanges );
return count;
}

/* See if the 3 gxLine segments on the edge of the more negative quadrant are cut by at
 * least 2 contour lines.
 */

static tt_int32 nLowerXings(tt_int16** lineBase, tt_int16** valBase, tt_int16 gxLine, tt_int16 val,
						tt_int16 lineChanges, tt_int16 valChanges, tt_int16 valMin, tt_int16 valMax, tt_int16 lineMin) /*<14>*/
{
register tt_int32 count = 0;

if( gxLine > lineMin ) count += nOnOff( lineBase, gxLine-1, val, lineChanges );		/*<14>*/
if( count > 1 ) return count;
if( val > valMin ) count += nOnOff( valBase, val-1, gxLine, valChanges );
if( count > 1 ) return count;
if( val < valMax ) count += nOnOff( valBase, val, gxLine, valChanges );
return count;
}


/*
 * rwb 02/07/94
 * New extrema procedure.  Use precalculated bounding box.
 * Also eliminate duplicate points.
 * Also count scan-line intersections.
 *
 * 
*/
void sc_FindExtrema4(const fnt_ElementType *glyphPtr, sc_BitMapData *bbox, tt_int32 doc, fsg_SplineKey *key) 
{
	register F26Dot6 	*x, *y;
	register F26Dot6 	tx, ty;									
	tt_int32       			n;
	fastInt 		 	aPoint, endPoint, startPoint, ctr;
	fastInt 			nYchanges, nXchanges, ny, rndX, rndY, prevRx, prevRy;
	tt_int16 				*ptrX, *ptrY, *p;
	F26Dot6 			prevx, prevy;	
	
	memoryContext 		*memContext = 	key->memContext;	
	F26Dot6 			xmin = bbox->xMin;
	F26Dot6 			xmax = bbox->xMax;
	F26Dot6 			ymin = bbox->yMin;
	F26Dot6 			ymax = bbox->yMax;
	
/* 	Now count number of times each scan line is intersected.
	Also remove duplicate points, compressing X and Y arrays.
*/
	ny = ymax - ymin + 1;
	n = ny + (doc ? xmax - xmin + 1 : 0);

	ptrY = (short*) 
			GetPerFontMemory(key->theFont,  2*n); /* ScalerNewScratch( memContext, 2*n);	*/
	ptrX = ptrY + ny;
	p = ptrY;
	if( n ) do{ *p++ = 0; } while (--n);
	
	for ( ctr = 0; ctr < glyphPtr->contourCount; ctr++ ) 
	{
		endPoint = glyphPtr->ep[ctr];
		startPoint = glyphPtr->sp[ctr];
		x = &(glyphPtr->x[startPoint]); 						
		y = &(glyphPtr->y[startPoint]); 						
		if ( startPoint == endPoint ) continue; /* We need to do this for anchor points for composites */
	
		/*  if last point is on top of first point, delete last point, make first point ONcurve */
		if( glyphPtr->x[startPoint] == glyphPtr->x[endPoint] && glyphPtr->y[startPoint] == glyphPtr->y[endPoint])
		{
			--endPoint;
			glyphPtr->onCurve[startPoint] |= ONCURVE;
		}
	
		ty = *(y+endPoint-startPoint);
		rndY = (ty + HALFM) >> PIXSHIFT;
		tx = *(x+endPoint-startPoint);		
		rndX = (tx + HALFM) >> PIXSHIFT;
		for( aPoint = startPoint; aPoint <= endPoint; ++aPoint )
		{
			prevy = ty;
			prevRy = rndY;
			prevx = tx;
			prevRx = rndX;			
			ty = *y++;
			rndY = (ty + HALFM) >> PIXSHIFT;
			tx = *x++;											
			rndX = (tx + HALFM) >> PIXSHIFT;
			p = ptrY +prevRy - ymin;
			if ( ty > prevy )
			{
				n = rndY - prevRy;
				if( n ) do { ++(*p++); } while (--n);
			}
			else if (ty < prevy )
			{
				p = p - prevRy + rndY;
				n =  prevRy - rndY;
				if( n )	do { ++(*p++); } while (--n);
			}
/* This next condition could be speeded up if it happens more than once in a glyph
by keeping track of the duplications, and moving each point only once.  Better yet,
would be changing the parabola rendering so that points on top of each other are ok.
*/
			else if( tx == prevx )  /* adjacent points are on top of each other */
			{													
				fastInt j = aPoint-2-startPoint;
				register F26Dot6 *newx = x-3;  
				register F26Dot6 *oldx = newx++;  
				register F26Dot6 *newy = y-3;  
				register F26Dot6 *oldy = newy++;  
				register tt_int8 *newC = (tt_int8*)&(glyphPtr->onCurve[aPoint-2]);  
				register tt_int8 *oldC = newC++; 
				*(newC+1) |= ONCURVE;
				for( ; j>=0; --j)
				{
					*newx-- = *oldx--;
					*newy-- = *oldy--;
					*newC-- = *oldC--;
				}
				++startPoint;
			}
			if ( (ty & 0x3F) == HALF ) *p += 2;  /* point exactly on scan line, play it safe, add 2*/
			if( doc )
			{
				p = ptrX + prevRx - xmin;
				if ( tx > prevx )
				{
					n = rndX - prevRx;
					if( n ) do { ++(*p++); } while (--n);
				}
				else if (tx < prevx ) 
				{
					p = p - prevRx + rndX;
					n =  prevRx - rndX;
					if( n )	do { ++(*p++); } while (--n);
				}
				if ( (tx & 0x3F) == HALF ) *p += 2;
			}
		}
		glyphPtr->sp[ctr] = startPoint < endPoint ? startPoint : endPoint;
		glyphPtr->ep[ctr] = endPoint;			/* 7/93 change */
	}
	nYchanges = nXchanges = 0;
	p = ptrY - 1;
	n = ymax - ymin + 1;
	do{ if( *++p > nYchanges ) nYchanges = *p; } while (--n);
	if( doc )
	{
		p = ptrX - 1;
		n = xmax - xmin + 1;
		do{ if( *++p > nXchanges ) nXchanges = *p; } while (--n);
	}
	if(nXchanges == 0) nXchanges = 2;
	if(nYchanges == 0) nYchanges = 2;
	bbox->nXchanges = nXchanges;
	bbox->nYchanges = nYchanges;
	ReleasePerFontMemory( key->theFont,ptrY);	/* ScalerDisposeScratch(memContext, ptrY); */
}


/*
 * This function break up a parabola defined by three points (A,B,C) and breaks it
 * up into straight gxLine vectors given a maximium error. The maximum error is
 * 1/resolution * 1/ERRDIV. ERRDIV is defined in sc.h.
 *
 *           
 *         B *-_
 *          /   `-_
 *         /       `-_
 *        /           `-_ 
 *       /               `-_
 *      /                   `* C
 *   A *
 *
 * PARAMETERS:
 *
 * Ax, Ay contains the x and y coordinates for point A. 
 * Bx, By contains the x and y coordinates for point B. 
 * Cx, Cy contains the x and y coordinates for point C.
 * hX, hY are handles to the areas where the straight gxLine vectors are going to be put.
 * count is pointer to a count of how much data has been put into *hX, and *hY.
 *
 * F(t) = (1-t)^2 * A + 2 * t * (1-t) * B + t * t * C, t = 0... 1 =>
 * F(t) = t * t * (A - 2B + C) + t * (2B - 2A) + A  =>
 * F(t) = alfa * t * t + beta * t + A
 * Now say that s goes from 0...N, => t = s/N
 * set: G(s) = N * N * F(s/N)
 * G(s) = s * s * (A - 2B + C) + s * N * 2 * (B - A) + N * N * A
 * => G(0) = N * N * A
 * => G(1) = (A - 2B + C) + N * 2 * (B - A) + G(0)
 * => G(2) = 4 * (A - 2B + C) + N * 4 * (B - A) + G(0) =
 *           3 * (A - 2B + C) + 2 * N * (B - A) + G(1)
 *
 * D(G(0)) = G(1) - G(0) = (A - 2B + C) + 2 * N * (B - A)
 * D(G(1)) = G(2) - G(1) = 3 * (A - 2B + C) + 2 * N * (B - A)
 * DD(G)   = D(G(1)) - D(G(0)) = 2 * (A - 2B + C)
 * Also, error = DD(G) / 8 .
 * Also, a subdivided DD = old DD/4.
 */
local tt_int32  sc_DrawParabola(	tt_int32 Ax,
								tt_int32 Ay,
								tt_int32 Bx,
								tt_int32 By,
								tt_int32 Cx,
								tt_int32 Cy,
								tt_int32 **hX,
								tt_int32 **hY,
								tt_int32 *count,
								tt_int32 inGY )
{
	register tt_int32 GX, GY, DX, DY, DDX, DDY, nsqs;
	register tt_int32 *xp, *yp;
	register tt_int32 tmp;
	tt_int32 i;

    /* Start calculating the first and 2nd order differences */
    GX  = Bx; /* GX = Bx */
	DDX = (DX = (Ax - GX)) - GX + Cx; /* = alfa-x = half of ddx, DX = Ax - Bx */
	GY  = By; /* GY = By */
	DDY = (DY = (Ay - GY)) - GY + Cy; /* = alfa-y = half of ddx, DY = Ay - By */
	/* The calculation is not finished but these intermediate results are useful */

    if ( inGY < 0 ) {
		/* calculate amount of steps necessary = 1 << GY */
		/* calculate the error, GX and GY used a temporaries */
		GX  = DDX < 0 ? -DDX : DDX;
		GY  = DDY < 0 ? -DDY : DDY;
		/* approximate GX = sqrt( ddx * ddx + ddy * ddy ) = Euclididan distance, DDX = ddx/2 here */
		GX += GX > GY ? GX + GY : GY + GY; /* GX = 2*distance = error = GX/8 */
	
		/* error = GX/8, but since GY = 1 below, error = GX/8/4 = GX >> 5, => GX = error << 5 */
#ifdef ERRSHIFT
		for ( GY = 1; GX > (PIXELSIZE << (5 - ERRSHIFT)); GX >>= 2  )  {
#else
		for ( GY = 1; GX > (PIXELSIZE << 5 ) / ERRDIV; GX >>= 2  ) {
#endif
			GY++; /* GY used for temporary purposes */
		}
		/* Now GY contains the amount of subdivisions necessary, number of vectors == (1 << GY )*/
		if ( GY > MAXMAXGY ) GY = MAXMAXGY; /* Out of range => Set to maximum possible. */
		i = 1 << GY;
		if ( (*count = *count + i )  > MAXVECTORS ) {	
		   /* Overflow, not enough space => return */
		   return( 1 );
		}
	} else {
	    GY = inGY;
		i = 1 << GY;
	}

    if ( GY > MAXGY ) {
	    DDX = GY-1; /* DDX used as a temporary */
	    /* Subdivide, this is nummerically stable. */
#define MIDX GX
#define MIDY GY
		MIDX = (Ax + Bx + Bx + Cx + 2) >> 2;
		MIDY = (Ay + By + By + Cy + 2) >> 2;
		DX   = (Ax + Bx + 1) >> 1;
		DY   = (Ay + By + 1) >> 1;
		sc_DrawParabola( Ax, Ay, DX, DY, MIDX, MIDY, hX, hY, count, DDX );
		DX = (Cx + Bx + 1) >> 1;
		DY = (Cy + By + 1) >> 1;
		sc_DrawParabola( MIDX, MIDY, DX, DY, Cx, Cy, hX, hY, count, DDX );
	    return 0;
	}
	nsqs = GY + GY; /* GY = n shift, nsqs = n*n shift */

	/* Finish calculations of 1st and 2nd order differences */
	DX   = DDX - (DX << ++GY); /* alfa + beta * n */
	DDX += DDX;
	DY   = DDY - (DY <<   GY);
	DDY += DDY;

    xp = *hX;
	yp = *hY;

	tmp = 1L << (nsqs-1);								/*<16> factor out of loop below */
	GY = (Ay << nsqs) + tmp; /*  Ay * (n*n) */			/*<16> and add in here */
	GX = (Ax << nsqs) + tmp; /*  Ax * (n*n) */ /* GX and GY used for real now */ /*<16> and here*/
	
	/* OK, now we have the 1st and 2nd order differences,
	   so we go ahead and do the forward differencing loop. */
	do {
		GX += DX;  /* Add first order difference to x coordinate */
		DX += DDX; /* Add 2nd order difference to first order difference. */
		GY += DY;  /* Do the same thing for y. */
		DY += DDY;
		*xp = GX >> nsqs;								/*<16> postpone increment */
		*yp = GY >> nsqs;								/*<16> postpone increment */
		if( *xp == *(xp-1) && *yp == *(yp-1) ) --*count;/*<16> Eliminate zero length vectors */
		else											/*<16>*/
		{												/*<16>*/
			++xp;										/*<16>*/
			++yp;										/*<16>*/
		}												/*<16>*/
	} while ( --i );
	*hX = xp; /* Done, update pointers, so that caller will know how much data we put in. */
	*hY = yp;
	return 0;
}
#undef MIDX
#undef MIDY

#endif
