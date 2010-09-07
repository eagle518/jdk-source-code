/*
 * @(#)sc.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
   
  /*
 * This module scanconverts a gxShape defined by quadratic bezier splines
 *
 *  © Apple Computer Inc. 1987, 1988, 1989.
 *
   * 
 */

#ifndef scanConverterIncludes
#define scanConverterIncludes

#include "Fnt.h"
 
/* DO NOT change these constants without understanding implications:
   overflow, out of range, out of memory, quality considerations, etc... */
   
#define PIXELSIZE 64 /* number of units per pixel. It has to be a power of two */
#define PIXSHIFT   6 /* should be 2log of PIXELSIZE */
#define ERRDIV     16 /* maximum error is  (pixel/ERRDIV) */
#define ERRSHIFT 4  /* = 2log(ERRDIV), define only if ERRDIV is a power of 2 */
#define ONE 0x40			/* constants for 26.6 arithmetic */
#define HALF 0x20
#define HALFM (HALF-1)
#define HALFP (HALF+1)
#define FRACPART 0x3F
#define INTPART 0xFFFFFFC0
#define STUBCONTROL 0x10000
#define NODOCONTROL 0x20000

/* The maximum number of vectors a spline segment is broken down into
 * is 2 ^ MAXGY 
 * MAXGY can at most be:
 * (31 - (input range to sc_DrawParabola 15 + PIXSHIFT = 21)) / 2
 */
#define MAXGY 5
#define MAXMAXGY 8 /* related to MAXVECTORS */

/* RULE OF THUMB: xPoint and yPoints will run out of space when
 *                MAXVECTORS = 176 + ppem/4 ( ppem = pixels per EM )  */
#define MAXVECTORS 262  /* must be at least 257  = (2 ^ MAXMAXGY) + 1  <3> added 5 because vecount
							is now initialized to 5 rather than 1 plus 1 for slop*/

#define sc_outOfMemory 0x01 /* For the error field */
#define sc_freeBitMap  0x01 /* For the info field */

struct sc_BitMapData	{
	tt_uint32		*bitMap;
	tt_int16		*xLines, *yLines, **xBase, **yBase;
    tt_int16		xMin, yMin, xMax, yMax;
	tt_uint16		nXchanges, nYchanges;
	tt_uint16		high, wide;
};

typedef struct sc_BitMapData sc_BitMapData;

/* rwb 4/2/90  New definition of sc_BitMapData.

bitMap is high bits tall, and wide bits wide, but wide is rounded up to
a long.  The actual gxBitmap width is xMax - xMin. xMin and yMin represent the
rounded integer value of the minimum 26.6 coordinate, but j.5 is rounded down
to j rather than up to j+1.  xMax and yMax represent the rounded up integer
value of the maximum 26.6 coordinat, and j.5 does round up to j+1.  The actual
pixel center scan lines that are represented in the gxBitmap are xMin ... xMax-1
and yMin...to ...yMax-1.

nYchanges is the total number of times that all of the contours in a glyph changed
y direction.  It is always an even number, and represents the maximum number
of times that a row scan gxLine can intersect the glyph.  Similarly, nXchanges
is the total number of x direction changes.

yLines is an array of arrays.  Each array corresoponds to one row scan gxLine.  Each
array is nYchanges+2 entries long. The 0th entry contains the number of times that row 
intersects the glyph contours in an On Transition and then the pixel columns where
the intersections occur.  These intersections are sorted from left to right.  The last
entry contains the number of OFF transition intersections, and the immediately
preceding entries contain the pixel column numbers where the intersections occur.
These are also sorted from left to right.  yBase is an array of
pointers; each pointer pointing to one of the arrays in yLines.

Similarly, xLines and xBase describe the intersection of column scan lines with
the glyph conotours.  These arrays are only used to fix dropouts.
*/

struct sc_GlobalData {
    tt_int32 xPoints[ MAXVECTORS ];   /* vectors */
    tt_int32 yPoints[ MAXVECTORS ];
};

typedef struct sc_GlobalData sc_GlobalData;

typedef struct fsg_SplineKey fsg_SplineKey;	/* Entire state of a scaler call */

/* Internal flags for the onCurve array */
#define OVERLAP 0x02 /* can not be the same as ONCURVE in sfnt.h */
#define DROPOUTCONTROL 0x04 /* can not be the same as ONCURVE in sfnt.h */

/*
 * Returns the gxBitmap
 * This is the top level call to the scan converter.
 *
 * Assumes that (*handle)->bbox.xmin,...xmax,...ymin,...ymax
 * are already set by sc_FindExtrema()
 *
 * PARAMETERS:
 *
 * lowBand   is lowest scan gxLine to be included in the band.
 * highBand  is the highest scan gxLine to be included in the band.
 * if highBand < lowBand then no banding will be done.
 * Always keep lowBand and highband within range: [ymin, (ymin+1) ....... ymax];
 * scPtr->bitMap always points at the actual memory.
 * the first row of pixels above the baseLine is numbered 0, and the next one up is 1.
 * => the y-axis definition is the normal one with the y-axis pointing straight up.
 *
 */

extern int sc_ScanChar2(const fnt_ElementType *glyphPtr, sc_GlobalData *scPtr, 
			sc_BitMapData *bbox, tt_int16 lowBand, tt_int16 highBand,
			tt_int32 scanKind );

void sc_FindExtrema4(const fnt_ElementType *glyphPtr, sc_BitMapData *bbox, 
		     tt_int32 doc, struct fsg_SplineKey *key);

#endif
