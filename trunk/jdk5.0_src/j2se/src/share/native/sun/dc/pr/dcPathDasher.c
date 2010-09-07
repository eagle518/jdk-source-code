/*
 * @(#)dcPathDasher.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathDasher.c 3.2 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcPRError.h"
#include "dcPRException.h"

#include "dcPathError.h"

#include "dcPathConsumer-p.h"
#include "dcPathDasher.h"

#include "dcPathStorage.h"
#include "affine.h"
#include "angles.h"
#include "arcs.h"


/*----------------------
 * The data of its instances
 */
typedef struct dcPathDasherData_ {
    dcPathConsumerData	cr;
    /* client interaction state */
    ixx                 inPath;
    /* ---- Changeable when !inPath, frozen when inPath ----- */
    /* dash pattern definition */
    ixx                 patternMaximumSize;
    float*              pattern;
    ixx                 patternSize;
    float               offset;
    float               characteristicDash;
    /* dash transformation */
    f32                 dashT4[4];
    ixx                 dashT4IsIdentity;
    /* output transformation */
    f32			t6[6];
    ixx          	t6IsIdentity;
    /* where to send the dashed path */
    dcPathConsumer      out;
    /* ---- Computed when inPath changes from 0 to 1 ----- */
    /* input transformation */
    f32                 inputT4[4];
    ixx                 inputT4IsIdentity;
    /* output transformation */
    f32                 outputT6[6];
    ixx                 outputT6IsIdentity;
    /* initial pattern traversal state */
    ixx                 initialIndex;
    float               initialLength;
    ixx                 initialDashOn;
    ixx                 initialZeroDash;
    /* ---- Changes only when inPath, meaningles when !inPath ----- */
    ixx                 inSubpath;
    /* ---- Change only when inSubpath, meaningles when !inSubpath ----- */
    ixx                 index;
    float               length;
    ixx                 dashOn;
    ixx                 zeroDash;
    /* input path traversal state */
    float               firstX, firstY;
    float               lastX, lastY;
    ixx                 isClosed;
    /* dash generation and processing */
    ixx                 insideDash;
    ixx                 doingFirstDash;
    dcPathStorage       firstDash;
} dcPathDasherData;



/*===============================================================
 * Management of various parts of the state
 */

/*-----------------------
 * Managing the dash pattern part of the state
 */

static f32
characteristicDash(f32* dashes, ixx dashcnt)
{
    f32 chardash = 0;
    ixx notinit = 1;
    while (dashcnt--) {
        f32 tmp = *dashes++;
        if (tmp > 0.0) {
            if (notinit) {
                chardash = tmp;
                notinit  = 0; 
            } else if (tmp < chardash) {
                chardash = tmp;
            }
        }
    }
    return (chardash);
}

#define  INITIAL_MAX_SIZE   20

static void
patternInit(doeE env, dcPathDasherData* st)
{
    st->patternMaximumSize = INITIAL_MAX_SIZE;
    st->pattern = (f32*)doeMem_malloc(env, INITIAL_MAX_SIZE * sizeof(f32));
    if (st->pattern == NULL) {
        doeError_setNoMemory(env);
	return;
    }
    st->patternSize = 0;
    st->offset = 0.0F;
    st->characteristicDash = 0.0F;
}

static void
patternCopy(doeE env, dcPathDasherData* st, dcPathDasherData* srcst) {
    st->patternMaximumSize = srcst->patternMaximumSize;
    st->pattern = (f32*)doeMem_malloc(env, st->patternMaximumSize * sizeof(f32));
    if (st->pattern == NULL) {
        doeError_setNoMemory(env);
	return;
    }
    st->patternSize = srcst->patternSize;
    st->offset = srcst->offset;
    st->characteristicDash = srcst->characteristicDash;
    {
        f32* src = srcst->pattern;
        f32* dst = st->pattern;
        ixx  cnt = st->patternSize;
        while (cnt--) *dst++ = *src++;
    }
}

static void
patternNew(doeE env, dcPathDasherData* st, f32* pattern, ixx size, f32 offset)
{
    if( (pattern == NULL) || (size <= 0) ) {
        st->patternSize = 0;
        st->offset = offset;
        st->characteristicDash = 0.0F;
    } else {
        f32* dst;
        if (size > st->patternMaximumSize) {
            f32* tmp = (f32*)doeMem_realloc(env, st->pattern, size * sizeof(f32));
            if (tmp == NULL) {
		doeError_setNoMemory(env);
		return;
	    }
            st->pattern = tmp;
            st->patternMaximumSize = size;
        }
        st->patternSize = size;
        st->offset = offset;
        st->characteristicDash = characteristicDash(pattern, size);
        dst = st->pattern;
        while (size--) *dst++ = *pattern++;
    }
}

static void
patternBeginTraversal(dcPathDasherData* st)
{
    f32 offset = st->offset;
    ixx index = 0,
        pendown = 1;
    if( offset > 0.0F ) {
        while( offset >= st->pattern[index] ) {
            offset -= st->pattern[index];
            index += 1;
            if( index >= st->patternSize ) index = 0;
            pendown = !pendown;
        }
    }
    st->initialIndex = st->index = index;
    st->initialLength = st->length = st->pattern[index] - offset;
    st->initialDashOn = st->dashOn = pendown;
    st->initialZeroDash = (st->pattern[index] == 0.0F);
}

static void
patternReset(dcPathDasherData* st) {
    st->index = st->initialIndex;
    st->length = st->initialLength;
    st->dashOn = st->initialDashOn;
    st->zeroDash = st->initialZeroDash;
}

static void
patternNextDash(dcPathDasherData* st) {
    st->index += 1;
    if (st->index >= st->patternSize) st->index = 0;
    st->length = st->pattern[st->index];
    st->dashOn = !st->dashOn;
    st->zeroDash = (st->length == 0.0F);
}

/*-----------------------
 * Dash generation
 */

static void
dashingReset(doeE env, dcPathDasherData* st)
{
    dcPathConsumer fdc = (dcPathConsumer)st->firstDash;
    st->insideDash = 0;
    st->doingFirstDash = st->dashOn;
    (*fdc)->beginPath(env, fdc);
}

static void
dashingSegment(doeE env, dcPathDasherData* st, ixx deg, f32* pnts, ixx continues) {
    dcPathConsumer target = (st->doingFirstDash) ? (dcPathConsumer)(st->firstDash)
                                                 : (st->out);
    if (!st->outputT6IsIdentity) {
        f32* start = st->insideDash ? (pnts + 2) : pnts;
        ixx  cnt   = st->insideDash ? deg : (deg + 1);
        affineT6TransformPoints(st->outputT6, start, cnt);
    }
    if (!st->insideDash) {
        (*target)->beginSubpath(env, target, pnts[0], pnts[1]);
	if (doeError_occurred(env)) return;
    }
    if (deg == 1)
        (*target)->appendLine(env, target, pnts[2], pnts[3]);
    else if (deg == 2)
        (*target)->appendQuadratic(env, target, pnts[2],pnts[3],pnts[4],pnts[5]);
    else /* if (deg == 3) */
        (*target)->appendCubic(env, target, pnts[2], pnts[3],
					    pnts[4], pnts[5], pnts[6], pnts[7]);
    if (doeError_occurred(env)) return;
    st->doingFirstDash = (st->doingFirstDash && continues);
    st->insideDash = continues;
}

static void
dashingNullSegment(dcPathDasherData* st)
{
    st->doingFirstDash = 0;
    st->insideDash = 0;
}

static void
dashingFlush(doeE env, dcPathDasherData* st)
{
    dcPathStorage  fd  = st->firstDash;
    dcPathConsumer fdc = (dcPathConsumer)fd;
    ixx            mask = dcPathStorageARC_MASK | dcPathStorageCLOSED_MASK;
    /* complete the first dash */
    if (st->isClosed && st->doingFirstDash) {
        (*fdc)->closedSubpath(env, fdc);
	if (doeError_occurred(env)) return;
    }
    (*fdc)->endPath(env, fdc);
    if (doeError_occurred(env)) return;

    /* issue the first dash */
    if (st->doingFirstDash || !st->isClosed || !st->insideDash) 
        mask |= dcPathStorageSUBPATH_MASK;
    (*fd)->sendToConsumer(env, fd, st->out, mask);
    if (doeError_occurred(env)) return;
    (*fd)->reset(env, fd);
}

/*===============================================================
 * Breaking arcs into dashes. Each arcs is broken into one or
 * more "segments". A dash is made up of one or more segments, 
 * but no two segments from the same arc.
 */

/*-----------------------
 * Testing if the modulus of the velocity of an arc 
 * changes in a nearly linear fashion
 */

#define DASH_TOLERANCE                 0.08F
#define LVMV_TOLERANCE                 (DASH_TOLERANCE * 1.3F)
#define LENGTH_TOLERANCE               (DASH_TOLERANCE * 0.1F)

ixx
quadraticHasLVMV(f32* length, f32* difs, f32* mods, f32 replength)
{
    f32 cmod, pmod, errorbound;
    cmod = anglesModulus(difs[0] + difs[2], difs[1] + difs[3]);
    pmod = mods[0] + mods[1];
    if ((pmod - cmod) > (cmod * LENGTH_TOLERANCE)) return(0);
    *length = ((cmod * 2.0F) + pmod) / 3.0F;
    errorbound = *length - (mods[0] + mods[1]);
    if (errorbound < 0.0F) errorbound = -errorbound;
    if (errorbound > LVMV_TOLERANCE) return(0);
    if (replength >= *length) return(1);
    errorbound = ((mods[0] - mods[1]) / *length) * (1.0F - (replength / *length));
    if (errorbound < 0.0F) errorbound = -errorbound;
    if (errorbound > DASH_TOLERANCE) return(0);
    return (1);
}

ixx
cubicHasLVMV(f32* length, f32* difs, f32* mods, f32 replength)
{
    f32 cx, cy, cmod, pmod, errorbound;
    cx = difs[0] + difs[2] + difs[4], 
    cy = difs[1] + difs[3] + difs[5],
    cmod = anglesModulus(cx, cy),
    pmod = mods[0] + mods[1] + mods[2];
    if ((pmod - cmod) > (cmod * LENGTH_TOLERANCE)) return(0);
    *length = (cmod + pmod) / 2.0F;
    errorbound = *length - (1.5F * (mods[0] + mods[2]));
    if (errorbound < 0.0F) errorbound = -errorbound;
    if (errorbound > LVMV_TOLERANCE) return(0);
    if (replength >= *length) return(1);
    errorbound = 1.5F * ((mods[0] - mods[2]) / *length) * (1.0F - (replength / *length));
    if (errorbound < 0.0F) errorbound = -errorbound;
    if (errorbound > DASH_TOLERANCE) return(0);
    return (1);
}

/*-----------------------
 * Dividing an arc into segments between two values of
 * the parameter
 */

void
progressiveDifferences(f32* prdifs, ixx deg, f32* pnts)
{
    f32* difs = prdifs;
    ixx    cnt = (deg + 1) << 1;
    while (cnt--) *difs++ = *pnts++;
    difs = prdifs;
    while (deg--) {
        f32  newx, newy;
        f32* tmp;
        newx = *difs; newy = *(difs+1);
        difs += 2;
        tmp = difs;
        cnt = deg + 1;
        while (cnt--) {
            f32  oldx = newx,
                 oldy = newy;
            newx = *tmp; newy = *(tmp+1);
            *tmp = newx - oldx; *(tmp+1) = newy - oldy;
            tmp += 2;
        }
    }
}

void
arcSegment(f32* seg, ixx deg, f32* pdifs, f32 t0, f32 dt)
{
    f32 t1 = t0 + dt, 
        t0x2, t0xx2, t0xx3, 
        t1x2, t1xx2, t1xx3;
    if (deg == 1) {
        seg[0] = pdifs[0] + (t0 * pdifs[2]);
        seg[1] = pdifs[1] + (t0 * pdifs[3]);
        seg[2] = pdifs[0] + (t1 * pdifs[2]);
        seg[3] = pdifs[1] + (t1 * pdifs[3]);
    } else if (deg == 2) {
        t0x2 = t0 * 2.0F; t0xx2 = t0 * t0;
        t1x2 = t1 * 2.0F; t1xx2 = t1 * t1; 
        seg[0] = pdifs[0] + (t0x2 * pdifs[2]) + (t0xx2 * pdifs[4]); 
        seg[1] = pdifs[1] + (t0x2 * pdifs[3]) + (t0xx2 * pdifs[5]);
        seg[4] = pdifs[0] + (t1x2 * pdifs[2]) + (t1xx2 * pdifs[4]); 
        seg[5] = pdifs[1] + (t1x2 * pdifs[3]) + (t1xx2 * pdifs[5]);
        seg[2] = seg[0] + (pdifs[2] + (t0 * pdifs[4])) * dt;
        seg[3] = seg[1] + (pdifs[3] + (t0 * pdifs[5])) * dt;
    } else if (deg == 3) {
        t0x2 = t0 * 2.0F; t0xx2 = t0 * t0; t0xx3 = t0 * t0xx2;
        t1x2 = t1 * 2.0F; t1xx2 = t1 * t1; t1xx3 = t1 * t1xx2;
        seg[0] = pdifs[0] + 
                 (3.0F * ((t0 * pdifs[2]) + (t0xx2 * pdifs[4]))) + 
                 (t0xx3 * pdifs[6]); 
        seg[1] = pdifs[1] + 
                 (3.0F * ((t0 * pdifs[3]) + (t0xx2 * pdifs[5]))) + 
                 (t0xx3 * pdifs[7]); 
        seg[6] = pdifs[0] + 
                 (3.0F * ((t1 * pdifs[2]) + (t1xx2 * pdifs[4]))) + 
                 (t1xx3 * pdifs[6]); 
        seg[7] = pdifs[1] + 
                 (3.0F * ((t1 * pdifs[3]) + (t1xx2 * pdifs[5]))) + 
                 (t1xx3 * pdifs[7]); 
        seg[2] = seg[0] + 
                 (pdifs[2] + (t0x2 * pdifs[4]) + (t0xx2 * pdifs[6])) * dt;
        seg[3] = seg[1] + 
                 (pdifs[3] + (t0x2 * pdifs[5]) + (t0xx2 * pdifs[7])) * dt;
        seg[4] = seg[6] - 
                 (pdifs[2] + (t1x2 * pdifs[4]) + (t1xx2 * pdifs[6])) * dt;
        seg[5] = seg[7] - 
                 (pdifs[3] + (t1x2 * pdifs[5]) + (t1xx2 * pdifs[7])) * dt;
    }
}

void
arcChord(f32* chord, ixx deg, f32* pdifs, f32 t0, f32 dt)
{
    f32 t1 = t0 + dt, 
        t0x2, t0xx2, t0xx3, 
        t1x2, t1xx2, t1xx3;
    if (deg == 1) {
        chord[0] = pdifs[0] + (t0 * pdifs[2]);
        chord[1] = pdifs[1] + (t0 * pdifs[3]);
        chord[2] = pdifs[0] + (t1 * pdifs[2]);
        chord[3] = pdifs[1] + (t1 * pdifs[3]);
    } else if (deg == 2) {
        t0x2 = t0 * 2.0F; t0xx2 = t0 * t0;
        t1x2 = t1 * 2.0F; t1xx2 = t1 * t1; 
        chord[0] = pdifs[0] + (t0x2 * pdifs[2]) + (t0xx2 * pdifs[4]); 
        chord[1] = pdifs[1] + (t0x2 * pdifs[3]) + (t0xx2 * pdifs[5]);
        chord[2] = pdifs[0] + (t1x2 * pdifs[2]) + (t1xx2 * pdifs[4]); 
        chord[3] = pdifs[1] + (t1x2 * pdifs[3]) + (t1xx2 * pdifs[5]);
    } else if (deg == 3) {
        t0xx2 = t0 * t0; t0xx3 = t0 * t0xx2;
        t1xx2 = t1 * t1; t1xx3 = t1 * t1xx2;
        chord[0] = pdifs[0] + 
                   (3.0F * ((t0 * pdifs[2]) + (t0xx2 * pdifs[4]))) + 
                   (t0xx3 * pdifs[6]); 
        chord[1] = pdifs[1] + 
                   (3.0F * ((t0 * pdifs[3]) + (t0xx2 * pdifs[5]))) + 
                   (t0xx3 * pdifs[7]); 
        chord[2] = pdifs[0] + 
                   (3.0F * ((t1 * pdifs[2]) + (t1xx2 * pdifs[4]))) + 
                   (t1xx3 * pdifs[6]); 
        chord[3] = pdifs[1] + 
                   (3.0F * ((t1 * pdifs[3]) + (t1xx2 * pdifs[5]))) + 
                   (t1xx3 * pdifs[7]); 
    }
}


/*-----------------------
 * Dividing an arc into dashes using the state's dash pattern
 */

#define MIN_DASH_FRACTION   1.0e-2F

static void
computeDashes(doeE env, dcPathDasherData* st, ixx deg, f32* pnts, f32 arclength)
{
    if (arclength == 0.0F) return;
    if (arclength < st->length) {
        /* current dash contains entire arc */
        if (st->dashOn) {
            dashingSegment(env, st, deg, pnts, 1);
	    if (doeError_occurred(env)) return;
	}
        st->length -= arclength;
    } else {
        /* at least one dash (on or off) terminates in the arc */
        f32 consumed  = 0.0F,
            tconsumed = 0.0F,
            mintinc   = (st->characteristicDash * MIN_DASH_FRACTION) / arclength,
            tinc, segpnts[8], pdifs[8];
        progressiveDifferences(pdifs, deg, pnts);
        while ((arclength - consumed) >= st->length) {
            /* current dash terminating in the arc */
            tinc = st->length / arclength;
            if (st->dashOn) {
                if (tinc > mintinc) {
                    arcSegment(segpnts, deg, pdifs, tconsumed, tinc);
                    dashingSegment(env, st, deg, segpnts, 0);
		    if (doeError_occurred(env)) return;
                } else {
                    if (st->zeroDash) {
                        arcChord(segpnts, deg, pdifs, tconsumed, mintinc);
                        dashingSegment(env, st, 1, segpnts, 0);
			if (doeError_occurred(env)) return;
                    } else /* tiny fraction of non-zero dash -- don't bother */
                        dashingNullSegment(st);
                }
            }
            tconsumed += tinc;
            consumed += st->length;
            patternNextDash(st);
        }
        if (consumed < arclength) {
            /* last dash of arc, possibly to be continued in the next arc */
            if (st->dashOn) {
                f32 leftover = 1.0F - tconsumed;
                if (leftover > mintinc) {
                    arcSegment(segpnts, deg, pdifs, tconsumed, leftover);
                    dashingSegment(env, st, deg, segpnts, 1);
		    if (doeError_occurred(env)) return;
                } else {
                    /* tiny fraction of non-zero dash -- don't bother */
                    dashingNullSegment(st);
                }
            }
            st->length -= (arclength - consumed);
        }
    }
}


/*===============================================================
 * Processing of the input arcs to insure that the modulus
 * of the velocity changes approximately linearly along
 * them
 */

static void
processLine(doeE env, dcPathDasherData* st, f32* pnts)
{
    f32  difs[2], length;
    difs[0] = pnts[2] - pnts[0]; difs[1] = pnts[3] - pnts[1];
    length  = anglesModulus(difs[0], difs[1]);
    computeDashes(env, st, 1, pnts, length);
}

#define DEGENERATE_BRANCH_FRACTION        1.0e-3F

static void
processQuadratic(doeE env, dcPathDasherData* st, f32* pnts)
{
    f32 minbranch = st->characteristicDash * DEGENERATE_BRANCH_FRACTION,
        difs[4], mods[2];
    if (arcsQuadraticDifsAndMods(difs, mods, pnts, minbranch)) {
        f32 linepnts[4];
        linepnts[0] = pnts[0]; linepnts[1] = pnts[1];
        linepnts[2] = pnts[4]; linepnts[3] = pnts[5];
        processLine(env, st, linepnts);
    } else {
        f32 length;
        if (quadraticHasLVMV(&length, difs, mods, st->characteristicDash)) {
            computeDashes(env, st, 2, pnts, length );
        } else {
            f32  p1[6], p2[6];
            arcsQuadraticDivision(pnts, p1, p2);
            processQuadratic(env, st, p1);
	    if (doeError_occurred(env)) return;
            processQuadratic(env, st, p2);
        }
    }
}

static void
processCubic(doeE env, dcPathDasherData* st, f32* pnts)
{
    f32 minbranch = st->characteristicDash * DEGENERATE_BRANCH_FRACTION,
        difs[6], mods[3];
    if (arcsCubicDifsAndMods(difs, mods, pnts, minbranch)) {
        f32 linepnts[4];
        linepnts[0] = pnts[0]; linepnts[1] = pnts[1];
        linepnts[2] = pnts[6]; linepnts[3] = pnts[7];
        processLine(env, st, linepnts);
    } else {
        f32 length;
        if (cubicHasLVMV(&length, difs, mods, st->characteristicDash)) {
            computeDashes(env, st, 3, pnts, length);
        } else {
            f32  p1[8], p2[8];
            arcsCubicDivision(pnts, p1, p2);
            processCubic(env, st, p1);
	    if (doeError_occurred(env)) return;
            processCubic(env, st, p2);
        }
    }
}

/*===============================================================
 * The class methods
 */

/*----------------------
 * The overriden doeObject methods
 */

static char*
className(doeE env, doeObject o)
{
    return  "dcPathDasher";
}

static void    dcPathDasher_copyinit(doeE, dcPathDasher, dcPathDasher src);

static doeObject
copy(doeE env, doeObject o)
{
    dcPathDasherData* st = 
        (dcPathDasherData*)doeMem_malloc(env, sizeof(dcPathDasherData));
    if (st == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcPathDasher_copyinit(env, (dcPathDasher)st, (dcPathDasher)o);
    return (doeObject)st;
}

static void
_cleanup(doeE env, doeObject o)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    (*(doeObject)(st->firstDash))->_cleanup(env, (doeObject)(st->firstDash));
    doeMem_free(env, (void*)(st->firstDash));
    doeMem_free(env, st->pattern);
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    dcPathDasherData* st = (dcPathDasherData*)o;
    cb->enumerate(env, cb, (doeObject)st->out);
    cb->enumerate(env, cb, (doeObject)st->firstDash);
}


/*----------------------
 * The implementation of the dcPathConsumer interface
 */

static void
computeTransformations(dcPathDasherData* st)
{
    if (st->dashT4IsIdentity) {
        affineT4MakeIdentity(st->inputT4);
        st->inputT4IsIdentity = 1;
        affineT6Copy(st->outputT6, st->t6);
        st->outputT6IsIdentity = st->t6IsIdentity;
    } else {
        f32  totalt4[4], dxy[2], tmp4[4];
        affineT4Invert(st->inputT4, st->dashT4);
        st->inputT4IsIdentity = 0;
        affineT4DxyFromT6(tmp4, dxy, st->t6);
        affineT4Multiply(totalt4, st->dashT4, tmp4);
        affineT6FromT4Dxy(st->outputT6, totalt4, dxy);
        st->outputT6IsIdentity = 0;
    }
}

static void
endOfSubpath(doeE env, dcPathDasherData* st)
{
    if (st->isClosed && ((st->firstX != st->lastX)||(st->firstY != st->lastY))) {
	if (st->patternSize) {
            f32  pnts[4];
            pnts[0] = st->lastX;  pnts[1] = st->lastY; 
            pnts[2] = st->firstX; pnts[3] = st->firstY;
            processLine(env, st, pnts);
        } else
            (*(st->out))->appendLine(env, st->out, st->firstX, st->firstY);
    }
    if (doeError_occurred(env)) return;
    if (st->patternSize) 
	dashingFlush(env, st);
}

static void
beginPath(doeE env, dcPathConsumer o)
{
    dcPathDasherData*  st = (dcPathDasherData*)o;
    if (st->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_beginPath);
	return;
    }
    st->inPath = 1;
    st->inSubpath = 0;
    computeTransformations(st);
    if (st->patternSize)
        patternBeginTraversal(st);

    (*(st->out))->beginPath(env, st->out);
}

static void
beginSubpath(doeE env, dcPathConsumer o, f32 x0, f32 y0)
{
    dcPathDasherData*   st = (dcPathDasherData*)o;
    if (!st->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_beginSubpath);
	return;
    }
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
    } else 
        st->inSubpath = 1;

    if (!st->inputT4IsIdentity)
        affineT4TransformPoint(st->inputT4, &x0, &y0);
    if (st->patternSize) {
        patternReset(st);
        dashingReset(env, st);
    } else {
        if (!st->outputT6IsIdentity)
            affineT6TransformPoint(st->outputT6, &x0, &y0);
        (*(st->out))->beginSubpath(env, st->out, x0, y0);
    }
    /* if (doeError_occurred(env)) return; should be able to skip this check */
    st->firstX = x0; st->firstY = y0;
    st->lastX  = x0; st->lastY  = y0;
    st->isClosed = 0;
}

static void
appendLine(doeE env, dcPathConsumer o, f32 x1, f32 y1)
{
    dcPathDasherData*   st = (dcPathDasherData*)o;
    if (!st->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendLine);
	return;
    }
    if (!st->inputT4IsIdentity)
        affineT4TransformPoint(st->inputT4, &x1, &y1);
    if (st->patternSize) {
        f32  pnts[4];
        pnts[0] = st->lastX; pnts[1] = st->lastY; 
        pnts[2] = x1; pnts[3] = y1;
        processLine(env, st, pnts);
    } else {
        if (!st->outputT6IsIdentity)
            affineT6TransformPoint(st->outputT6, &x1, &y1);
        (*(st->out))->appendLine(env, st->out, x1, y1);
    }
    /* if (doeError_occurred(env)) return; should be able to skip this check */
    st->lastX = x1; st->lastY = y1;
}

static void
appendQuadratic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2)
{
    dcPathDasherData*   st = (dcPathDasherData*)o;
    if (!st->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendQuadratic);
	return;
    }
    if (!st->inputT4IsIdentity) {
        affineT4TransformPoint(st->inputT4, &x1, &y1);
        affineT4TransformPoint(st->inputT4, &x2, &y2);
    }
    if (st->patternSize) {
        f32  pnts[6];
        pnts[0] = st->lastX; pnts[1] = st->lastY; 
        pnts[2] = x1; pnts[3] = y1;
        pnts[4] = x2; pnts[5] = y2;
        processQuadratic(env, st, pnts);
    } else {
        if (!st->outputT6IsIdentity) {
            affineT6TransformPoint(st->outputT6, &x1, &y1);
            affineT6TransformPoint(st->outputT6, &x2, &y2);
        }
        (*(st->out))->appendQuadratic(env, st->out, x1, y1, x2, y2);
    }
    /* if (doeError_occurred(env)) return; should be able to skip this check */
    st->lastX = x2; st->lastY = y2;
}

static void
appendCubic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3) {
    dcPathDasherData*   st = (dcPathDasherData*)o;
    if (!st->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendCubic);
	return;
    }
    if (!st->inputT4IsIdentity) {
        affineT4TransformPoint(st->inputT4, &x1, &y1);
        affineT4TransformPoint(st->inputT4, &x2, &y2);
        affineT4TransformPoint(st->inputT4, &x3, &y3);
    }
    if (st->patternSize) {
        f32  pnts[8];
        pnts[0] = st->lastX; pnts[1] = st->lastY; 
        pnts[2] = x1; pnts[3] = y1;
        pnts[4] = x2; pnts[5] = y2;
        pnts[6] = x3; pnts[7] = y3;
        processCubic(env, st, pnts);
    } else {
        if (!st->outputT6IsIdentity) {
            affineT6TransformPoint(st->outputT6, &x1, &y1);
            affineT6TransformPoint(st->outputT6, &x2, &y2);
            affineT6TransformPoint(st->outputT6, &x3, &y3);
        }
        (*(st->out))->appendCubic(env, st->out, x1, y1, x2, y2, x3, y3);
    }
    /* if (doeError_occurred(env)) return; should be able to skip this check */
    st->lastX = x3; st->lastY = y3;
}

static void
closedSubpath(doeE env, dcPathConsumer o)
{
    dcPathDasherData*   st = (dcPathDasherData*)o;
    if (!st->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_closedSubpath);
	return;
    }
    st->isClosed = 1;
    if (!st->patternSize)
        (*(st->out))->closedSubpath(env, st->out);
}

static void
endPath(doeE env, dcPathConsumer o)
{
    dcPathDasherData*  st = (dcPathDasherData*)o;
    if (!st->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_endPath);
	return;
    }
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
        st->inSubpath = 0;
    }
    st->inPath = 0;
    (*(st->out))->endPath(env, st->out);
}

static void
useProxy(doeE env, dcPathConsumer o, dcFastPathProducer fpp)
{
    dcPathDasherData*  st = (dcPathDasherData*)o;
    if (st->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_useProxy);
	return;
    }
    (*fpp)->sendTo(env, fpp, o);
}


/*----------------------
 * The class-specific methods
 */

static void
setDash(doeE env, dcPathDasher o, f32* pattern, ixx cnt, f32 offset)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    if (st->inPath) { 
	doeError_set(env, dcPRError, dcPRError_UNEX_setDash);
	return;
    }
    if ((offset < 0.0F) || (cnt < 0)) {
	doeError_set(env, dcPRError, dcPRError_BAD_dashpattern);
	return;
    }
    if (cnt > 0) {
        f32  totlen = 0.0;
        ixx  tmp = cnt;
        f32* len = pattern;
        while (tmp--) {
            if (*len < 0.0F) {
		doeError_set(env, dcPRError, dcPRError_BAD_dashpattern);
		return;
	    }
            totlen += *len++;
        }
        if (totlen == 0.0F) {
	    doeError_set(env, dcPRError, dcPRError_BAD_dashpattern);
	    return;
	}
    }
    patternNew(env, st, pattern, cnt, offset);
}
 
static void
setDashT4(doeE env, dcPathDasher o, f32* t4)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    if (st->inPath) {
	doeError_set(env, dcPRError, dcPRError_UNEX_setDashT4);
	return;
    }
    if (t4 == NULL) {
        st->dashT4IsIdentity = 1;
        affineT4MakeIdentity(st->dashT4);
    } else {
        if (affineT4IsSingular(t4)) {
	    doeError_set(env, dcPRError, dcPRError_BAD_dasht4_singular);
	    return;
	}
        affineT4Copy(st->dashT4, t4);
        st->dashT4IsIdentity = affineT4IsIdentity(t4);
    }
}

static void
setOutputT6(doeE env, dcPathDasher o, f32* t6)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    if (st->inPath) {
	doeError_set(env, dcPRError, dcPRError_UNEX_setOutputT6);
	return;
    }
    if (t6 == NULL) {
        affineT6MakeIdentity( st->t6 );
        st->t6IsIdentity = 1;
    } else {
        if (affineT6IsSingular(t6)) {
	    doeError_set(env, dcPRError, dcPRError_BAD_outputt6_singular);
	    return;
	}
        affineT6Copy(st->t6, t6);
        st->t6IsIdentity = affineT6IsIdentity(t6);
    }
}

static void
setOutputConsumer(doeE env, dcPathDasher o, dcPathConsumer dst)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    if (st->inPath) {
	doeError_set(env, dcPRError, dcPRError_UNEX_setOutputConsumer);
	return;
    }
    st->out = dst;
}

static void
reset(doeE env, dcPathDasher o)
{
    dcPathDasherData* st  = (dcPathDasherData*)o;
    st->inPath = 0;
    st->patternSize = 0;
    affineT4MakeIdentity(st->dashT4);
    st->dashT4IsIdentity = 1;
    affineT6MakeIdentity(st->t6);
    st->t6IsIdentity = 1;
}


/*----------------------
 * The class variable
 */

dcPathDasherFace dcPathDasherClass = {    
    {
	{
	    sizeof(dcPathDasherData),
	    className,			/* object interface */
	    copy,
	    _cleanup,
	    _enumCoObs,
	    doeObject_uproot
	},

	beginPath,			/* PathConsumer interface */
	beginSubpath,
	appendLine,
	appendQuadratic,
	appendCubic,
	closedSubpath,
	endPath,
	useProxy
    },

    setDash,			/* PathDasher interface */
    setDashT4,
    setOutputT6,
    setOutputConsumer,
    reset
};

/*----------------------
 * The class-related global functions
 */

static void
dcPathDasher_init(doeE env, dcPathDasher o, dcPathConsumer destination)
{
    dcPathDasherData* st = (dcPathDasherData*)o;
    dcPathConsumer_init(env, (dcPathConsumer)o);
    *o = &dcPathDasherClass;
    st->inPath = 0;
    patternInit(env, st);
    if (doeError_occurred(env)) return;

    affineT4MakeIdentity(st->dashT4);
    st->dashT4IsIdentity = 1;
    affineT6MakeIdentity(st->t6);
    st->t6IsIdentity = 1;
    st->out = destination;
    st->firstDash = dcPathStorage_create(env, 0);
}

dcPathDasher
dcPathDasher_create(doeE env, dcPathConsumer destination) {
    dcPathDasherData* st = 
        (dcPathDasherData*)doeMem_malloc(env, sizeof(dcPathDasherData));
    if (st == NULL) {
        doeError_setNoMemory(env);
	return NULL;
    }
    dcPathDasher_init(env, (dcPathDasher)st, destination);
    return (dcPathDasher)st;
}


#define objectCopy(env,t,o) (t)((*(doeObject)(o))->copy(env, (doeObject)(o)))

void
dcPathDasher_copyinit(doeE env, dcPathDasher o, dcPathDasher src)
{
    dcPathDasherData* st	= (dcPathDasherData*)o;
    dcPathDasherData* srcst	= (dcPathDasherData*)src;

    dcPathConsumer_copyinit(env, (dcPathConsumer)o, (dcPathConsumer)src);
    st->inPath = srcst->inPath;
    patternCopy(env, st, srcst);
    if (doeError_occurred(env)) return;

    affineT4Copy(st->dashT4, srcst->dashT4);
    st->dashT4IsIdentity = srcst->dashT4IsIdentity;
    affineT6Copy(st->t6, srcst->t6);
    st->t6IsIdentity = srcst->t6IsIdentity;
    st->out = srcst->out;
    affineT4Copy(st->inputT4, srcst->inputT4);
    st->inputT4IsIdentity = srcst->inputT4IsIdentity;
    affineT6Copy(st->outputT6, srcst->outputT6);
    st->outputT6IsIdentity = srcst->outputT6IsIdentity;
    st->initialIndex = srcst->initialIndex;
    st->initialLength = srcst->initialLength;
    st->initialDashOn = srcst->initialDashOn;
    st->index = srcst->index;
    st->length = srcst->length;
    st->dashOn = srcst->dashOn;
    st->inSubpath = srcst->inSubpath;
    st->firstX = srcst->firstX;
    st->firstY = srcst->firstY;
    st->lastX = srcst->lastX;
    st->lastY = srcst->lastY;
    st->isClosed = srcst->isClosed;
    st->insideDash = srcst->insideDash;
    st->doingFirstDash = srcst->doingFirstDash;
    st->firstDash = objectCopy(env, dcPathStorage, srcst->firstDash);
}

void dcPathDasher_staticInitialize(doeE env) { }
void dcPathDasher_staticFinalize  (doeE env) { }
