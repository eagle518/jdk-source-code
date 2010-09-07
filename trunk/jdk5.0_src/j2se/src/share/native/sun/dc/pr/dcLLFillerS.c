/*
 * @(#)dcLLFillerS.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFillerS.c 3.2 97/11/18
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "dcLLFillerS.h"
#include "dcLLFillerST.h"
#include "doe.h"
#include "doeObject-p.h"
#include "dcPRError.h"

#define BASE doeObject

#define	 fbitsU		dcLLFillerS_subGridL2S
#define  fmaskU		((1 << fbitsU) - 1)
#define ffbitsU		(fbitsU << 1)
#define	ffmaskU		((1 << ffbitsU) - 1)
#define  jbitsU		(fbitsU + 1)
#define	 jmaskU		((1 << jbitsU) - 1)
#define	jjbitsU		(jbitsU << 1)
#define	jjmaskU		((1 << jjbitsU) - 1)

#define	int2U(i)	((i) << fbitsU)

#define	zeroU		int2U(0)
#define	 oneU		int2U(1)
#define	halfU		(oneU >> 1)
#define	float1U		(f32)oneU
#define	ceilU(u)	(((u) + fmaskU) & (~fmaskU))
#define	U2int(u)	((u) >> fbitsU)
#define U2float(u)	((f32)u/float1U)

#define	xyjDI(njumps)	((njumps) << 1)
#define	xyjL		xyjDI(4 * (1 << dcLLFillerS_tileSizeL2S))
#define xyjBufferSize	xyjL + xyjDI(1)	/* one extra jump */

#define	tileDI(npix)	((npix) << 1)
#define	tileAllocW	tileDI(1 + (1 << dcLLFillerS_tileSizeL2S) + 1)
#define	tileAllocH	      (1 + (1 << dcLLFillerS_tileSizeL2S) + 1)

typedef struct dcLLFillerSData_ {
    doeObjectData	mu;

    ixx			fillMode;
    ixx			sizeXU, sizeYU;
    ixx			sizeX;

    ixx			xyjN;
    i8*			xyjBuffer;
    ixx			xyjX0, xyjY0;
    ixx			xyjIsInsideTile;
    ixx			x0, y0;

    i8*			tile;
} dcLLFillerSData;

static char*
className(doeE env, doeObject target) { return "LLFillerS"; }

static doeObject
copy(doeE env, doeObject target) { return target; /*!!*/ }

static void
_cleanup(doeE env, doeObject target) {
    dcLLFillerSData*	p = (dcLLFillerSData*)target;

    if (p->xyjBuffer != NULL)	doeMem_free(env, p->xyjBuffer);
    if (p->tile != NULL)	doeMem_free(env, p->tile);

    BASE__cleanup(env, target);
}

static void
_enumCoObs(doeE env, doeObject target, doeObjectEnumCb cb)
{
    BASE__enumCoObs(env, target, cb);
}

/*  PUBLIC INTERFACE - INITIALIZATION	*/

void
setParams(doeE env, dcLLFiller target, ixx fillmode, ixx sizex, ixx sizey)
{
    dcLLFillerSData* p = (dcLLFillerSData*)target;

    p->fillMode = fillmode;
    p->sizeX  = sizex;
    p->sizeXU = int2U(sizex);
    p->sizeYU = int2U(sizey);
}

/*  PUBLIC INTERFACE - PATH DESCRIPTION	*/

static ixx
tileIndexXYU(ixx x, ixx y) {
    x = U2int(x);
    y = U2int(y);
    x++; /* convert from (0,0) relative to base-relative */
    y++;
    return y * tileAllocW + tileDI(x);
}

void
processLeftRun(doeE env, dcLLFiller target, int fromy, int toy)
{
    dcLLFillerSData* p = (dcLLFillerSData*)target;
    i8		     dg;
    ixx		     i,l;

    if (fromy == toy) return;

    if (fromy < toy) {
	dg =  1;
    } else {
	int	t;

	dg = -1;
	t = fromy;
	fromy = toy;
	toy = t;
    }
    /* now fromy < toy */

    /* the run affects all global counts between ceil(fromy)
    (included) and ceil(toy) (excluded) */
    fromy = ceilU(fromy);
      toy = ceilU(  toy);

    i = tileIndexXYU(-oneU, fromy);
    l = tileIndexXYU(-oneU,   toy);
    while (i < l) {
	p->tile[i] += dg;
	i += tileAllocW;
    }
}

/* Tile marking: process a part of [xyjBuffer] beginning at position
   [x0,y0], index [xyjix], [xyjn] jumps long; the jumps are guaranteed
   to lie inside the tile.
*/

void
processSubBufferInTile(dcLLFillerSData* p, int xyjix, int xyjn, int x0, int y0)
{
    ixx		tileix;
    i8		lcache;
    i32		ff00;

    tileix = tileIndexXYU(x0, y0);
    ff00 = (((x0 & fmaskU) << fbitsU) | (y0 & fmaskU)) << jjbitsU;

    lcache = p->tile[tileix + 1];
    do {
	ixx jx = p->xyjBuffer[xyjix++];
	ixx jy = p->xyjBuffer[xyjix++];
	ixx ffjj = ff00 | ((jx & jmaskU) << jbitsU) | (jy & jmaskU);
	ixx ai  = ffjjActions[ffjj];
	for (;;) {
	    u8 action = actionCode[ai++];
	    if        (action < 128) {
		lcache += (action - 64);
	    } else if (action < 192) {
		ixx dx = (action >> 4) & 3;
		ixx dy = (action >> 2) & 3;
		ixx dg = (action     ) & 3;

		p->tile[tileix + 1] = lcache;
		if (dx != 0) {
		    if (dx == 1)	tileix += tileDI(1);
		    else		tileix -= tileDI(1);
		}
		if (dy != 0) {
		    if (dy == 1)	tileix += tileAllocW;
		    else		tileix -= tileAllocW;
		}
		lcache = p->tile[tileix + 1];

		if (dg != 0) {
		    if (dg == 3) dg = -1;
		    p->tile[tileix] += dg;
		}
	    } else {
		ff00 = (action & ffmaskU) << jjbitsU;
		break;
	    }
	}
	xyjn -= xyjDI(1);
    } while (xyjn > 0);
    p->tile[tileix + 1] = lcache;
}

/* Tile marking: process the whole [xyjBuffer].  This is the general
   case; the jumps in the buffer may fall in any of the following
   classes:
 	1. Jumps that do not afect the area of interest - that is, are
 	   entirely above, below or to the right of it. Except for
 	   their effect in the position, such jumps must be ignored.
 	2. Jumps whose bounding boxes do not intersect the area of
 	   interest but do intersect the left-unbounded region to the
 	   left of the area of interest.
 	3. Jumps whose bounding boxes intersect the area of interest.
   This method decomposes the buffer in subsequences of jumps that can
   be directly handled by [processSubBufferInTile] or by
   [processLeftRun].
*/

void
processJumpBuffer(doeE env, dcLLFillerSData* p)
{
    ixx	i = 0;
    ixx	j = xyjDI(1);
    ixx	l = p->xyjN;
    ixx k;
    ixx dx, dy;
    ixx	x0, y0, x1, y1;
    ixx	lox, loy, hix, hiy;
    ixx jrunx0, jruny0, jruny1;

    p->xyjBuffer[l]     =
    p->xyjBuffer[l + 1] = (i8)zeroU;	/* to avoid purify UIM error (the
					   loops below read 1 jump past
					   limit); the existence of the space
					   is the the caller's responsibility */

    dx = p->xyjBuffer[0];
    dy = p->xyjBuffer[1];
    x0 = p->xyjX0;
    y0 = p->xyjY0;
    x1 = x0 + dx;
    y1 = y0 + dy;

    if (dx >= 0)	{ lox = x0; hix = x1; }
    else		{ lox = x1; hix = x0; }
    if (dy >= 0)	{ loy = y0; hiy = y1; }
    else		{ loy = y1; hiy = y0; }

    /* We look at the buffer as containing triples {s1, s2, s3} where:
	    s1: longest possible sequence of jumps not affecting AOI
	    s2: longest possible sequence of jumps left of AOI
	    s3: longest possible sequence of jumps intersecting AOI
       (any two sequences may be empty for any given triple)
     */

    while (i < l) {
	/* skip the (possibly empty) sequence of jumps that do not
	   affect the area of interest (s1) */
	while (i < l) {
	    if (lox < p->sizeXU && loy < p->sizeYU && hiy > zeroU)
		break;

	    i = j; j += xyjDI(1);
	    dx = p->xyjBuffer[i];
	    dy = p->xyjBuffer[i + 1];
	    x0 = x1; x1 += dx;
	    y0 = y1; y1 += dy;
	    if (dx >= 0)    { lox = x0; hix = x1; }
	    else	    { lox = x1; hix = x0; }
	    if (dy >= 0)    { loy = y0; hiy = y1; }
	    else	    { loy = y1; hiy = y0; }
	}

	/* find and process - if not empty - the left run (s2) */
	jruny0 = y0;
	while (i < l && hix <= zeroU) {
	    i = j; j += xyjDI(1);
	    dx = p->xyjBuffer[i];
	    dy = p->xyjBuffer[i + 1];
	    x0 = x1; x1 += dx;
	    y0 = y1; y1 += dy;
	    if (dx >= 0)    { lox = x0; hix = x1; }
	    else	    { lox = x1; hix = x0; }
	    if (dy >= 0)    { loy = y0; hiy = y1; }
	    else	    { loy = y1; hiy = y0; }
	}
	jruny1 = y0;

	if (jruny0 < zeroU)	jruny0 = zeroU;
	if (jruny0 > p->sizeYU)	jruny0 = p->sizeYU;
	if (jruny1 < zeroU)	jruny1 = zeroU;
	if (jruny1 > p->sizeYU)	jruny1 = p->sizeYU;
	if (jruny0 != jruny1)
	    processLeftRun(env, (dcLLFiller)p, jruny0, jruny1);

	/* find and process - if not empty - the sequence of jumps
	   that intersect the area of interest (s3) */
	k = i;
	jrunx0 = x0;
	jruny0 = y0;
	while (i < l) {
	    if (lox >= p->sizeXU)	break;
	    if (hix <= zeroU)		break;
	    if (loy >= p->sizeYU)	break;
	    if (hiy <= zeroU)		break;

	    i = j; j += xyjDI(1);
	    dx = p->xyjBuffer[i];
	    dy = p->xyjBuffer[i + 1];
	    x0 = x1; x1 += dx;
	    y0 = y1; y1 += dy;
	    if (dx >= 0)    { lox = x0; hix = x1; }
	    else	    { lox = x1; hix = x0; }
	    if (dy >= 0)    { loy = y0; hiy = y1; }
	    else	    { loy = y1; hiy = y0; }
	}
	if (i > k) {
	    processSubBufferInTile(p, k, i - k, jrunx0, jruny0);
	}
    }
}

void
beginSubpath(doeE env, dcLLFiller target, i32 x0, i32 y0) {
    dcLLFillerSData* p = (dcLLFillerSData*)target;

    /* process existing jumps, if any */
    if (p->xyjN > 0) {
	if (p->xyjIsInsideTile)
	    processSubBufferInTile(p, 0, p->xyjN, p->xyjX0, p->xyjY0);
	else
	    processJumpBuffer(env, p);
    }

    /* reset buffer */
    p->xyjX0 = p->x0 = x0;
    p->xyjY0 = p->y0 = y0;
    p->xyjN = 0;
    p->xyjIsInsideTile =	x0 >= 0         && y0 >= 0
			     && x0 <= p->sizeXU && y0 <= p->sizeYU;
}

static ixx
log2StepsForDiamXDegree(i32 ddu) {
    i32		p2u = oneU - 1;
    ixx		l2  = 0;
    while (p2u < ddu) {
	p2u <<= 1;
	l2++;
    }
    return l2;
}

#define	ddaSubShift	9			/* 512 steps max */
#define	ddaShift	(3 * ddaSubShift)
#define	ddaHalf		(1 << (ddaShift - 1))
#define	ddaFMask	((1 << ddaShift) - 1)
#define dda2U(dda)	((dda) >> ddaShift)

void
appendArc1(doeE env, dcLLFiller target, i32 x1, i32 y1)
{
    dcLLFillerSData*
		p = (dcLLFillerSData*)target;
    i32		x01, y01;
    i32		diam, temp;
    ixx		stepsl2s, steps;
    ixx		i;

    x01 = x1 - p->x0;
    y01 = y1 - p->y0;

    if (x01==0 && y01==0)
	    return;

    /* compute diameter, steps */
    diam = ABS(x01);
    temp = ABS(y01);  if (temp > diam) diam = temp;
    stepsl2s = log2StepsForDiamXDegree(diam);
    steps    = 1 << stepsl2s;

    /* check space in xyjBuffer for as many jumps as steps and force
       it empty if necessary; set the local auxiliary index [i] */
    i = p->xyjN;
    if (i + xyjDI(steps) >= xyjL) {
	beginSubpath(env, target, p->x0, p->y0);
	i = 0;
    }

    /* update [xyjIsInsideTile] */
    p->xyjIsInsideTile =   p->xyjIsInsideTile
			&& x1 >= 0         && y1 >= 0
			&& x1 <= p->sizeXU && y1 <= p->sizeYU;

    if (diam < oneU) {	/* special case: very short line */
	p->xyjBuffer[i++] = (i8)x01;
	p->xyjBuffer[i++] = (i8)y01;
    } else if (x01 == 0) {	/* trace a vertical line */
	i32	d0y = ddaHalf;
	i32	d1y = y01 << (ddaShift - stepsl2s);
	while (steps-- > 0) {
	    d0y += d1y;
	    p->xyjBuffer[i++] = (i8)zeroU;
	    p->xyjBuffer[i++] = (i8)dda2U(d0y);
	    d0y &= ddaFMask;
	}
    } else if (y01 == 0) {	/* trace a horizontal line */
	i32	d0x = ddaHalf;
	i32	d1x = x01 << (ddaShift - stepsl2s);
	while (steps-- > 0) {
	    d0x += d1x;
	    p->xyjBuffer[i++] = (i8)dda2U(d0x);
	    p->xyjBuffer[i++] = (i8)zeroU;
	    d0x &= ddaFMask;
	}
    } else {		/* trace a general line */
	i32 d0x = ddaHalf;
	i32 d1x = x01 << (ddaShift - stepsl2s);
	i32 d0y = ddaHalf;
	i32 d1y = y01 << (ddaShift - stepsl2s);
	while (steps-- > 0) {
	    d0x += d1x;
	    d0y += d1y;
	    p->xyjBuffer[i++] = (i8)dda2U(d0x);
	    p->xyjBuffer[i++] = (i8)dda2U(d0y);
	    d0x &= ddaFMask;
	    d0y &= ddaFMask;
	}
    }

    /* update the buffer index and the current position */
    p->xyjN = i;
    p->x0 = x1;
    p->y0 = y1;

    return;
}

void appendArc2(doeE env, dcLLFiller target, i32 x1, i32 y1, i32 x2, i32 y2)
{
    dcLLFillerSData*
		p = (dcLLFillerSData*)target;
    i32		x01, y01, x12, y12;
    i32		diam, temp;
    ixx		stepsl2s, steps;
    ixx		i;

    x01 = x1 - p->x0;
    y01 = y1 - p->y0;
    x12 = x2 - x1;
    y12 = y2 - y1;

    /* compute diameter, steps */
    diam = ABS(x01);
    temp = ABS(y01);  if (temp > diam) diam = temp;
    temp = ABS(x12);  if (temp > diam) diam = temp;
    temp = ABS(y12);  if (temp > diam) diam = temp;
    stepsl2s = log2StepsForDiamXDegree(diam + diam);
    steps    = 1 << stepsl2s;

    /* check space in xyjBuffer for as many jumps as steps and force
       it empty if necessary; set the local auxiliary index [i] */
    i = p->xyjN;
    if (i + xyjDI(steps) >= xyjL) {
	beginSubpath(env, target, p->x0, p->y0);
	i = 0;
    }

    /* update [xyjIsInsideTile] */
    p->xyjIsInsideTile =  p->xyjIsInsideTile
			&& x1 >= 0         && y1 >= 0
			&& x1 <= p->sizeXU && y1 <= p->sizeYU
			&& x2 >= 0         && y2 >= 0
			&& x2 <= p->sizeXU && y2 <= p->sizeYU;

    if (diam < oneU) {	/* special case: very short quadratic */
	if (x01 != 0 || y01 != 0) {
	    p->xyjBuffer[i++] = (i8)x01;
	    p->xyjBuffer[i++] = (i8)y01;
	}
	if (x12 != 0 || y12 != 0) {
	    p->xyjBuffer[i++] = (i8)x12;
	    p->xyjBuffer[i++] = (i8)y12;
	}
    } else {		/* trace the quadratic */
	i32 d0x, d1x, d2x;
	i32 d0y, d1y, d2y;

	temp = ddaShift - stepsl2s;
	d0x  = ddaHalf;
	d2x  = (x12 - x01) << (temp - stepsl2s);
	d1x  = (x01 << (temp + 1)) + d2x;
	d2x += d2x;
	d0y  = ddaHalf;
	d2y  = (y12 - y01) << (temp - stepsl2s);
	d1y  = (y01 << (temp + 1)) + d2y;
	d2y += d2y;
	while (steps-- > 0) {
	    i8	dx, dy;

	    d0x += d1x; d1x += d2x;
	    d0y += d1y; d1y += d2y;
	    dx = (i8)dda2U(d0x);
	    dy = (i8)dda2U(d0y);
	    if (dx != 0 || dy != 0) {
		p->xyjBuffer[i++] = dx;
		p->xyjBuffer[i++] = dy;
	    }
	    d0x &= ddaFMask;
	    d0y &= ddaFMask;
	}
    }

    /* update the buffer index and the current position */
    p->xyjN = i;
    p->x0 = x2;
    p->y0 = y2;

    return;
}

void appendArc3(	doeE env, dcLLFiller target,
			i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3) {
    dcLLFillerSData*
		p = (dcLLFillerSData*)target;
    i32		x01, y01, x12, y12, x23, y23;
    i32		diam, temp;
    ixx		stepsl2s, steps;
    ixx		i;

    x01 = x1 - p->x0;
    y01 = y1 - p->y0;
    x12 = x2 - x1;
    y12 = y2 - y1;
    x23 = x3 - x2;
    y23 = y3 - y2;

    /* compute diameter, steps */
    diam = ABS(x01);
    temp = ABS(y01);  if (temp > diam) diam = temp;
    temp = ABS(x12);  if (temp > diam) diam = temp;
    temp = ABS(y12);  if (temp > diam) diam = temp;
    temp = ABS(x23);  if (temp > diam) diam = temp;
    temp = ABS(y23);  if (temp > diam) diam = temp;
    stepsl2s = log2StepsForDiamXDegree(diam + diam + diam);
    steps    = 1 << stepsl2s;

    /* check space in xyjBuffer for as many jumps as steps and force
       it empty if necessary; set the local auxiliary index [i] */
    i = p->xyjN;
    if (i + xyjDI(steps) >= xyjL) {
	beginSubpath(env, target, p->x0, p->y0);
	i = 0;
    }

    /* update [xyjIsInsideTile] */
    p->xyjIsInsideTile =  p->xyjIsInsideTile
			&& x1 >= 0         && y1 >= 0
			&& x1 <= p->sizeXU && y1 <= p->sizeYU
			&& x2 >= 0         && y2 >= 0
			&& x2 <= p->sizeXU && y2 <= p->sizeYU
			&& x3 >= 0         && y3 >= 0
			&& x3 <= p->sizeXU && y3 <= p->sizeYU;

    if (diam < oneU) {	/* special case: very short cubic */
	if (x01 != 0 || y01 != 0) {
	    p->xyjBuffer[i++] = (i8)x01;
	    p->xyjBuffer[i++] = (i8)y01;
	}
	if (x12 != 0 || y12 != 0) {
	    p->xyjBuffer[i++] = (i8)x12;
	    p->xyjBuffer[i++] = (i8)y12;
	}
	if (x23 != 0 || y23 != 0) {
	    p->xyjBuffer[i++] = (i8)x23;
	    p->xyjBuffer[i++] = (i8)y23;
	}
    } else {		/* trace the cubic */
	int d0x, d1x, d2x, d3x;
	int d0y, d1y, d2y, d3y;

	temp  = ddaShift - stepsl2s;
	d0x   = ddaHalf;
	d1x   = x01 << temp;
	temp -= stepsl2s;
	d2x   = (x12 - x01) << temp;
	temp -= stepsl2s;
	d3x   = (x23 - (x12 << 1) + x01) << temp;
	d1x  += d2x;
	d1x  += d1x + d1x + d3x;
	d2x   = (d2x << 3) - (d2x << 1);
	d3x   = (d3x << 3) - (d3x << 1);
	d2x  += d3x;

	temp  = ddaShift - stepsl2s;
	d0y   = ddaHalf;
	d1y   = y01 << temp;
	temp -= stepsl2s;
	d2y   = (y12 - y01) << temp;
	temp -= stepsl2s;
	d3y   = (y23 - (y12 << 1) + y01) << temp;
	d1y  += d2y;
	d1y  += d1y + d1y + d3y;
	d2y   = (d2y << 3) - (d2y << 1);
	d3y   = (d3y << 3) - (d3y << 1);
	d2y  += d3y;

	while (steps-- > 0) {
	    i8	dx, dy;

	    d0x += d1x; d1x += d2x; d2x += d3x;
	    d0y += d1y; d1y += d2y; d2y += d3y;
	    dx = (i8)dda2U(d0x);
	    dy = (i8)dda2U(d0y);
	    if (dx != 0 || dy != 0) {
		p->xyjBuffer[i++] = dx;
		p->xyjBuffer[i++] = dy;
	    }
	    d0x &= ddaFMask;
	    d0y &= ddaFMask;
	}
    }

    /* update the buffer index and the current position */
    p->xyjN = i;
    p->x0 = x3;
    p->y0 = y3;

    return;
}

static void
writeAlpha8NZ(dcLLFillerSData* p, u8* alpha0, i32 xstride, i32 ystride, i32 pix0offset)
{
    i8*	    rowp;
    i8*	    rowl;
    i8*	    pixp;
    i8*	    pixl;
    u8*	    alpha;
    i32	    gcnt;
    u8	    ga8;
    i32	    pixw = tileIndexXYU(p->sizeXU, zeroU) - tileIndexXYU(zeroU, zeroU);

    alpha0 += pix0offset;

    rowp = p->tile + tileIndexXYU(zeroU, zeroU);
    rowl = p->tile + tileIndexXYU(zeroU, p->sizeYU);

    for (  ; rowp < rowl; rowp += tileAllocW) {
	pixp = rowp;
	pixl = rowp + pixw;
	alpha = alpha0;
	alpha0 += ystride;

	gcnt = (pixp - tileDI(1))[0]; /* previous pixel's G */
	ga8  = gcnt != 0 ? 0xFF : 0x00;

	for (  ; pixp < pixl; pixp += tileDI(1)) {
	    if (pixp[1] != 0) {
		i32    coverage;
		coverage = gCOVER(gcnt) + pixp[1];
		if (coverage < 0)	coverage = -coverage;
		if (coverage > COVER1)  coverage = COVER1;
		*alpha = cover64ToAlpha8[coverage];
	    } else {
		*alpha = ga8;
	    }
	    alpha += xstride;
	    if (pixp[0] != 0) {
		gcnt += pixp[0];
		ga8 = gcnt != 0 ? 0xFF : 0x00;
	    }
	}
    }
}

static void
writeAlpha8EO(dcLLFillerSData* p, u8* alpha0, i32 xstride, i32 ystride, i32 pix0offset)
{
    i8*	    rowp;
    i8*	    rowl;
    i8*	    pixp;
    i8*	    pixl;
    u8*	    alpha;
    i32	    gcnt;
    u8	    ga8;
    i32	    pixw = tileIndexXYU(p->sizeXU, zeroU) - tileIndexXYU(zeroU, zeroU);

    alpha0 += pix0offset;

    rowp = p->tile + tileIndexXYU(zeroU, zeroU);
    rowl = p->tile + tileIndexXYU(zeroU, p->sizeYU);

    for (  ; rowp < rowl; rowp += tileAllocW) {
	pixp = rowp;
	pixl = rowp + pixw;
	alpha = alpha0;
	alpha0 += ystride;

	gcnt = (pixp - tileDI(1))[0]; /* previous pixel's G */
	ga8  = gcnt & 1 ? 0xFF : 0x00;

	for (  ; pixp < pixl; pixp += tileDI(1)) {
	    if (pixp[1] != 0) {
		i32 coverage;

		coverage = pixp[1];
		if (coverage < 0)   coverage = -coverage;
		if (gcnt & 1)	    coverage = COVER1 - coverage;
		*alpha = cover64ToAlpha8[coverage];
	    } else {
		*alpha = ga8;
	    }
	    alpha += xstride;
	    if (pixp[0] != 0) {
		gcnt += pixp[0];
		ga8 = gcnt & 1 ? 0xFF : 0x00;
	    }
	}
    }
}


static void
writeAlpha16NZ(dcLLFillerSData* p, u16* alpha0, i32 xstride, i32 ystride, i32 pix0offset)
{
    i8*	    rowp;
    i8*	    rowl;
    i8*	    pixp;
    i8*	    pixl;
    u16*    alpha;
    i32	    gcnt;
    u16	    ga16;
    i32	    pixw = tileIndexXYU(p->sizeXU, zeroU) - tileIndexXYU(zeroU, zeroU);

    alpha0 += pix0offset;

    rowp = p->tile + tileIndexXYU(zeroU, zeroU);
    rowl = p->tile + tileIndexXYU(zeroU, p->sizeYU);

    for (  ; rowp < rowl; rowp += tileAllocW) {
	pixp = rowp;
	pixl = rowp + pixw;
	alpha = alpha0;
	alpha0 += ystride;

	gcnt = (pixp - tileDI(1))[0]; /* previous pixel's G */
	ga16  = gcnt != 0 ? 0xFFFF : 0x0000;

	for (  ; pixp < pixl; pixp += tileDI(1)) {
	    if (pixp[1] != 0) {
		i32    coverage;
		coverage = gCOVER(gcnt) + pixp[1];
		if (coverage < 0)	coverage = -coverage;
		if (coverage > COVER1)  coverage = COVER1;
		*alpha = cover64ToAlpha16[coverage];
	    } else {
		*alpha = ga16;
	    }
	    alpha += xstride;
	    if (pixp[0] != 0) {
		gcnt += pixp[0];
		ga16 = gcnt != 0 ? 0xFFFF : 0x0000;
	    }
	}
    }
}

static void
writeAlpha16EO(dcLLFillerSData* p, u16* alpha0, i32 xstride, i32 ystride, i32 pix0offset)
{
    i8*	    rowp;
    i8*	    rowl;
    i8*	    pixp;
    i8*	    pixl;
    u16*    alpha;
    i32	    gcnt;
    u16	    ga16;
    i32	    pixw = tileIndexXYU(p->sizeXU, zeroU) - tileIndexXYU(zeroU, zeroU);

    alpha0 += pix0offset;

    rowp = p->tile + tileIndexXYU(zeroU, zeroU);
    rowl = p->tile + tileIndexXYU(zeroU, p->sizeYU);

    for (  ; rowp < rowl; rowp += tileAllocW) {
	pixp = rowp;
	pixl = rowp + pixw;
	alpha = alpha0;
	alpha0 += ystride;

	gcnt = (pixp - tileDI(1))[0]; /* previous pixel's G */
	ga16  = gcnt & 1 ? 0xFFFF : 0x0000;

	for (  ; pixp < pixl; pixp += tileDI(1)) {
	    if (pixp[1] != 0) {
		i32 coverage;

		coverage = pixp[1];
		if (coverage < 0)   coverage = -coverage;
		if (gcnt & 1)	    coverage = COVER1 - coverage;
		*alpha = cover64ToAlpha16[coverage];
	    } else {
		*alpha = ga16;
	    }
	    alpha += xstride;
	    if (pixp[0] != 0) {
		gcnt += pixp[0];
		ga16 = gcnt & 1 ? 0xFFFF : 0x0000;
	    }
	}
    }
}

static void
reset(dcLLFillerSData* p)
{
    i8* rowp;
    i8* rowl;
    i8* pixp;
    i8* pixl;
    i32 pixw =  tileIndexXYU(p->sizeXU + oneU,	zeroU)
	      - tileIndexXYU(		-oneU,	zeroU);

    rowp = p->tile + tileIndexXYU(-oneU, zeroU);
    rowl = p->tile + tileIndexXYU(-oneU, p->sizeYU + oneU);

    for (  ; rowp < rowl; rowp += tileAllocW) {
	pixp = rowp;
	pixl = rowp + pixw;
	for (  ; pixp < pixl; pixp += sizeof(u32)/sizeof(i8))
	    *(u32*)pixp = 0;
    }
}

void
writeAlpha8 (	doeE env, dcLLFiller target,
		 u8* alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    dcLLFillerSData* p = (dcLLFillerSData*)target;

    /* process existing jumps, if any */
    if (p->xyjN > 0) {
	if (p->xyjIsInsideTile)
	    processSubBufferInTile(p, 0, p->xyjN, p->xyjX0, p->xyjY0);
	else
	    processJumpBuffer(env, p);
	p->xyjN = 0;
    }

    if (p->fillMode==dcLLFiller_NZFILL)
	writeAlpha8NZ (p, alpha, xstride, ystride, pix0offset);
    else
	writeAlpha8EO (p, alpha, xstride, ystride, pix0offset);

    reset(p);
}

void
writeAlpha16(	doeE env, dcLLFiller target,
		u16* alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    dcLLFillerSData* p = (dcLLFillerSData*)target;

    /* process existing jumps, if any */
    if (p->xyjN > 0) {
	if (p->xyjIsInsideTile)
	    processSubBufferInTile(p, 0, p->xyjN, p->xyjX0, p->xyjY0);
	else
	    processJumpBuffer(env, p);
	p->xyjN = 0;
    }

    if (p->fillMode==dcLLFiller_NZFILL)
	writeAlpha16NZ(p, alpha, xstride, ystride, pix0offset);
    else
	writeAlpha16EO(p, alpha, xstride, ystride, pix0offset);

    reset(p);
}


dcLLFillerFace	dcLLFillerSClass = {
    {
	sizeof(dcLLFillerSData),

	className,
	
	copy,
	_cleanup,
	_enumCoObs,
	doeObject_uproot,
    },

    setParams,
    processLeftRun,
    beginSubpath,
    appendArc1,
    appendArc2,
    appendArc3,
    writeAlpha8,
    writeAlpha16
};

void
dcLLFillerS_init(doeE env, dcLLFiller target) {
    dcLLFillerSData*	p = (dcLLFillerSData *)target;
    i8			*tilep, *tilel;

    BASE_init(env, target);
    if (doeError_occurred(env)) {
	BASE__cleanup(env, target);
	return;
    }

    *target = &dcLLFillerSClass;

    p->xyjBuffer = doeMem_malloc(env, (i32)sizeof(i8)*xyjL);
    p->tile      = doeMem_malloc(env, (i32)sizeof(i8)*tileAllocW*tileAllocH);
    if (p->xyjBuffer == NULL || p->tile == NULL) {
	doeError_setNoMemory(env);
	_cleanup(env, (doeObject)target);
	return;
    }

    tilep = p->tile;
    tilel = tilep + (tileAllocW * tileAllocH);
    while (tilep < tilel)
	*tilep++ = 0;
    p->xyjN = 0;
}

dcLLFiller
dcLLFillerS_create(doeE env) {
    dcLLFillerSData*	p = doeMem_malloc(env, (i32)sizeof(dcLLFillerSData));
    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcLLFillerS_init(env, (dcLLFiller)p);
    if (doeError_occurred(env)) {
	doeMem_free(env, p);
    }

    return (dcLLFiller)p;
}


static doeMutex		fillerMutex;
static ixx		clients = 0;
static dcLLFiller	filler;

dcLLFiller
dcLLFillerS_get(doeE env) {
    dcLLFiller	f;

#ifdef DEBUG_MT
    (env->reporter)(env, "LLFillerS requested");
#endif
    doeMutex_lock(env, fillerMutex);
#ifdef DEBUG_MT
    (env->reporter)(env, "LLFillerS obtained");
#endif
    return filler;
}

void
dcLLFillerS_release(doeE env, dcLLFiller f) {
#ifdef DEBUG_MT
    (env->reporter)(env, "LLFillerS released");
#endif
    doeMutex_unlock(env, fillerMutex);
}


void
dcLLFillerS_staticInitialize(doeE env)
{
    if (clients++ > 0) return;
    fillerMutex = doeMutex_create(env);
    filler = dcLLFillerS_create(env);
}

void
dcLLFillerS_staticFinalize(doeE env)
{
    if (--clients > 0) return;

    _cleanup(env, (doeObject)filler);
    doeMem_free(env, filler);

    doeMutex_destroy(env, fillerMutex);
}

#undef BASE

