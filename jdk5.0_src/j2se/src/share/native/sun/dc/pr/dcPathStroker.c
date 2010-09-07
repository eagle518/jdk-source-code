/*
 * @(#)dcPathStroker.c	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStroker.c 3.1 97/11/17
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
#include "dcPathStroker.h"

#include "dcPathStorage.h"
#include "affine.h"
#include "angles.h"
#include "arcs.h"


/*----------------------
 * The data of its instances
 */
typedef struct dcPathStrokerData_ {
    dcPathConsumerData	cr;
    /* state of interaction with client */
    ixx                 inPath;
    /* ---- Changeable when !inPath, frozen when inPath ----- */
    /* stroke parameters */
    f32                 penRadius;
    ixx                 caps;
    ixx                 corners;
    f32                 miterLimit;
    /* pen fitting */
    f32                 unit;
    ixx                 minimumRadius;
    ixx                 penNeedsFitting;
    /* pen transformation */
    f32                 penT4[4];
    ixx                 penT4IsIdentity;
    /* output transformation */
    f32			t6[6];
    ixx                 t6IsIdentity;
    /* where to send the computed path */
    dcPathConsumer      out;
    /* ---- Computed when inPath changes from 0 to 1 ----- */
    /* input transformation */
    f32                 inputT4[4];
    ixx                 inputT4IsIdentity;
    /* output transformation */
    f32                 outputT6[6];
    ixx                 outputT6IsIdentity;
    /* ---- Changes only when inPath, meaningles when !inPath ----- */
    /* path traversal state */
    ixx                 inSubpath;
    /* ---- Change only when inSubpath, meaningles when !inSubpath ----- */
    ixx                 isClosed;
    f32                 firstX, firstY, 
                        lastX, lastY;
    /* storage for each input subpath */
    dcPathStorage       subpath;
} dcPathStrokerData;




/*===============================================================
 * Input processing of arcs to insure uniform angular velocity
 */

/*-----------------------
 * Testing if an arc has nearly uniform angular velocity
 */

#define MAX_QUADRATIC_GAP              anglesDEG(60)
#define MAX_QUADRATIC_DRIFT            anglesDEG(8)

ixx
quadraticHasUAV(f32* difs, i32* tans)
{
    i32 at, ut;
    if (anglesUnsignedSpan(*tans, tans[1]) > MAX_QUADRATIC_GAP) return(0);
    /* other "weirdness" tests? moduli? angle distribution? */
    /* angle drift at t=0.5 */
    at = anglesAtan2(difs[1] + difs[3], difs[0] + difs[2]);
    ut = *tans + (anglesSignedSpan(*tans, tans[1]) / 2);
    if (anglesUnsignedSpan(at, ut) > MAX_QUADRATIC_DRIFT) return(0);
    return(1);
}

#define MAX_CUBIC_GAP                  anglesDEG090
#define MAX_CUBIC_DRIFT                anglesDEG(7)

ixx
cubicHasUAV(f32* difs, i32* tans)
{
    f32  vtx, vty;
    i32  mut, ut, at;
    if (anglesUnsignedSpan(*tans, tans[2]) > MAX_CUBIC_GAP) return(0);
    /* other "weirdness" tests? moduli? angle distribution? */
    /* test at t = 0.5 */
    vtx = (difs[0] * 0.25F) + (0.5F * (difs[2])) + (difs[4] * 0.25F);
    vty = (difs[1] * 0.25F) + (0.5F * (difs[3])) + (difs[5] * 0.25F);
    ut = mut = *tans + (anglesSignedSpan(*tans, tans[2]) / 2);
    at = anglesAtan2(vty, vtx);
    if (anglesUnsignedSpan(at, ut) > MAX_CUBIC_DRIFT) return(0);
    /* test at t = 0.25 */
    vtx = (difs[0] * 0.5625F) + (0.375F * (difs[2])) + (difs[4] * 0.0625F);
    vty = (difs[1] * 0.5625F) + (0.375F * (difs[3])) + (difs[5] * 0.0625F);
    ut = *tans + (anglesSignedSpan(*tans, mut) / 2);
    at = anglesAtan2(vty, vtx);
    if (anglesUnsignedSpan(at, ut) > MAX_CUBIC_DRIFT) return(0);
    /* test at t = 0.75 */
    vtx = (difs[0] * 0.0625F) + (0.375F * (difs[2])) + (difs[4] * 0.5625F);
    vty = (difs[1] * 0.0625F) + (0.375F * (difs[3])) + (difs[5] * 0.5625F);
    ut = mut + (anglesSignedSpan(mut, tans[2]) / 2);
    at = anglesAtan2(vty, vtx);
    if (anglesUnsignedSpan(at, ut) > MAX_CUBIC_DRIFT) return(0);
    return(1);
}


#define DEGENERATE_BRANCH        1.0e-2F
/* Approximately 24 subdivisions before f32's map onto each other */
#define LEVEL_MAX                24

static void
processQuadratic(doeE env, dcPathStorage sp, f32* pnts, i32 level)
{
    dcPathConsumer spc = (dcPathConsumer) sp;
    f32            difs[4], mods[2];
    if (level >= LEVEL_MAX ||
        arcsQuadraticDifsAndMods(difs, mods, pnts, DEGENERATE_BRANCH))
    {
        i32 linetan = anglesAtan2(pnts[5] - pnts[1], pnts[4] - pnts[0]);
        (*spc)->appendLine(env, spc, pnts[4], pnts[5]);
	if (doeError_occurred(env)) return;
        (*sp)->appendTangent(env, sp, linetan);
    } else {
        i32    tans[2];
        tans[0] = anglesAtan2(difs[1], difs[0]);
        tans[1] = anglesAtan2(difs[3], difs[2]);
        if (quadraticHasUAV(difs, tans)) {
            (*spc)->appendQuadratic(env, spc, pnts[2],pnts[3],pnts[4],pnts[5]);
	    if (doeError_occurred(env)) return;
            (*sp)->appendTangents(env, sp, tans[0], tans[1]);
        } else {
            f32  p1[6], p2[6];
            arcsQuadraticDivision(pnts, p1, p2);
            processQuadratic(env, sp, p1, level+1);
	    if (doeError_occurred(env)) return;
            processQuadratic(env, sp, p2, level+1);
        }
    }
}

static void
processCubic(doeE env, dcPathStorage sp, f32* pnts, i32 level)
{
    dcPathConsumer spc = (dcPathConsumer) sp;
    f32            difs[6], mods[3];
    if (level >= LEVEL_MAX ||
        arcsCubicDifsAndMods(difs, mods, pnts, DEGENERATE_BRANCH))
    {
        i32 linetan = anglesAtan2(pnts[7] - pnts[1], pnts[6] - pnts[0]);
        (*spc)->appendLine(env, spc, pnts[6], pnts[7]);
	if (doeError_occurred(env)) return;
        (*sp)->appendTangent(env, sp, linetan);
    } else {
        i32    tans[3];
        tans[0] = anglesAtan2(difs[1], difs[0]);
        tans[1] = anglesAtan2(difs[3], difs[2]);
        tans[2] = anglesAtan2(difs[5], difs[4]);
        if (cubicHasUAV(difs, tans)) {
            (*spc)->appendCubic(env, spc, pnts[2],pnts[3],pnts[4],pnts[5],
					  pnts[6],pnts[7]);
	    if (doeError_occurred(env)) return;
            (*sp)->appendTangents(env, sp, tans[0], tans[2]);
        } else {
            f32  p1[8], p2[8];
            arcsCubicDivision(pnts, p1, p2);
            processCubic(env, sp, p1, level+1);
	    if (doeError_occurred(env)) return;
            processCubic(env, sp, p2, level+1);
        }
    }
}

/*===============================================================
 * Stroking of a stored subpath
 */

/*-----------------------
 * Basic elements of an envolvent
 */

static void
initialPoint(doeE env, dcPathStrokerData* st, f32 x, f32 y, i32 tan)
{ 
    dcPathConsumer out = st->out;
    f32            r   = st->penRadius;
    i32            a   = anglesAdd(tan, anglesDEG090);
    x += (r * anglesCos(a)); 
    y += (r * anglesSin(a));
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoint(st->outputT6, &x, &y);
    (*out)->beginSubpath(env, out, x, y);
}

static void
lineToPoint(doeE env, dcPathStrokerData* st, f32 x, f32 y)
{
    dcPathConsumer out = st->out;
    f32            tmp[2];
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoint(st->outputT6, &x, &y);
    (*out)->appendLine(env, out, x, y);
}

static void
lineToPenPoint(doeE env, dcPathStrokerData* st, f32 x, f32 y, i32 ang)
{ 
    dcPathConsumer out = st->out;
    f32            r   = st->penRadius;
    f32            tmp[2];
    x += (r * anglesCos(ang)); 
    y += (r * anglesSin(ang));
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoint(st->outputT6, &x, &y);
    (*out)->appendLine(env, out, x, y);
}

static void
lineToPolarPoint(doeE env, dcPathStrokerData* st, f32 x, f32 y, f32 r, i32 ang)
{ 
    dcPathConsumer out = st->out;
    f32            tmp[2];
    x += (r * anglesCos(ang)); 
    y += (r * anglesSin(ang));
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoint(st->outputT6, &x, &y);
    (*out)->appendLine(env, out, x, y);
}

static ixx
cubicCircleApproximation(f32* cr, i32* ca0, i32* ca1, i32 a0, i32 a1)
{
    i32 ssp  = anglesSignedSpan(a0, a1),
        uhsp = (ssp >= 0 ) ? ((ssp + 1) / 2) : (((-ssp) + 1) / 2);
    if (uhsp == anglesDEG000) {
        *cr = 1.0F, *ca0 = a0; *ca1 = a1;
        return(0);
    } else {
        f32 cb  = ((4.0F/3.0F) * (1.0F - anglesOct1Cos(uhsp))) / anglesOct1Sin(uhsp);
        i32 dlt = anglesOct1Atan2(cb, 1.0F);
        *ca0 = (ssp >= 0) ? a0 + dlt : a0 - dlt;
        *ca1 = (ssp >= 0) ? a1 - dlt : a1 + dlt;
        *cr  = 1.0F / anglesCos(dlt);
        return(1);
    }
}

/* 
 * The following function is only safe if the angle between 
 * a0 and a1 is less than or equal to MAX_CUBIC_GAP. If
 * the gap is greater the cubic generated may not be a good
 * approximation to a circle
 */
static void
penSection(doeE env, dcPathStrokerData* st, f32 x, f32 y, i32 a0, i32 a1)
{
    dcPathConsumer out    = st->out;
    f32            r      = st->penRadius;
    f32            cr;
    i32            ca0, ca1;
    if (a0 == a1) return;
    if (cubicCircleApproximation(&cr, &ca0, &ca1, a0, a1)) {
        f32 tmp[6];
        cr *= r;
        tmp[0] = x + (cr * anglesCos(ca0));
        tmp[1] = y + (cr * anglesSin(ca0));
        tmp[2] = x + (cr * anglesCos(ca1)); 
        tmp[3] = y + (cr * anglesSin(ca1));
        tmp[4] = x + (r * anglesCos(a1));   
        tmp[5] = y + (r * anglesSin(a1));
        if (!st->outputT6IsIdentity) 
            affineT6TransformPoints(st->outputT6, tmp, 3);
        (*out)->appendCubic(env, out, tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
    } else {
        if (a0 != a1)	lineToPenPoint(env, st, x, y, a1);
    }
}

/* 
 * This function is only safe if the angle between a0
 * and a1 is less than or equal to MAX_QUAD_GAP
 */
static void
quadEnvolvent(doeE env, dcPathStrokerData* st, f32* p1, f32* p2, i32 a0, i32 a1)
{
    dcPathConsumer  out = st->out;
    f32  r              = st->penRadius;
    i32  uhspan         = (anglesUnsignedSpan(a0, a1) + 1)/ 2;
    f32  mr             = r * (2.0F - anglesOct1Cos(uhspan));
    i32  am             = a0 + (anglesSignedSpan(a0, a1) / 2);
    f32  tmp[4];
    tmp[0] = p1[0] + (mr * anglesCos(am));
    tmp[1] = p1[1] + (mr * anglesSin(am));
    tmp[2] = p2[0] + (r * anglesCos(a1));  
    tmp[3] = p2[1] + (r * anglesSin(a1));
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoints(st->outputT6, tmp, 2);
    (*out)->appendQuadratic(env, out, tmp[0], tmp[1], tmp[2], tmp[3]);
}

/* 
 * The following function is only safe if the angle between 
 * a0 and a1 is less than or equal to MAX_CUBIC_GAP
 */
static void
cubicEnvolvent(doeE env, dcPathStrokerData* st,
		f32* p1, f32* p2, f32* p3, i32 a0, i32 a1) {
    dcPathConsumer  out = st->out;
    f32             r   = st->penRadius;
    f32             cr;
    i32             ca0, ca1;
    f32             tmp[6];
    cubicCircleApproximation(&cr, &ca0, &ca1, a0, a1);
    cr *= r;
    tmp[0] = p1[0] + (cr * anglesCos(ca0)); 
    tmp[1] = p1[1] + (cr * anglesSin(ca0));
    tmp[2] = p2[0] + (cr * anglesCos(ca1)); 
    tmp[3] = p2[1] + (cr * anglesSin(ca1));
    tmp[4] = p3[0] + (r * anglesCos(a1));   
    tmp[5] = p3[1] + (r * anglesSin(a1));
    if (!st->outputT6IsIdentity) 
        affineT6TransformPoints(st->outputT6, tmp, 3);
    (*out)->appendCubic(env, out, tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
}


/*-----------------------
 * Generation of caps
 */

static void
clockwiseCap(doeE env, dcPathStrokerData* st, f32 x, f32 y, i32 dir)
{
    dcPathConsumer  out = st->out;
    i32             bnorm, enorm;
    bnorm  = anglesAdd(dir, anglesDEG090);
    enorm  = anglesAdd(dir, anglesDEG270);
    if (st->caps == dcPathStroker_ROUND) {
        penSection(env, st, x, y, bnorm, dir);
	if (doeError_occurred(env)) return;
        penSection(env, st, x, y, dir, enorm);

    } else if (st->caps == dcPathStroker_BUTT) {
        lineToPenPoint(env, st, x, y, enorm);

    } else /* if (st->caps == dcPathStroker_SQUARE) */ {
        i32 bcor = anglesAdd(bnorm, -anglesDEG045),
            ecor = anglesAdd(enorm, anglesDEG045);
        f32 cr   = st->penRadius * 1.414213562F;

        lineToPolarPoint(env, st, x, y, cr, bcor);
	if (doeError_occurred(env)) return;
        lineToPolarPoint(env, st, x, y, cr, ecor);
	if (doeError_occurred(env)) return;
        lineToPenPoint(env, st, x, y, enorm);
    }
}

/*-----------------------
 * Generation of corners
 */

#define MIN_CORNER    anglesDEG(4)

static void
leftCorner(doeE env, dcPathStrokerData* st, f32 x, f32 y, i32 tin, i32 tout)
{
    dcPathConsumer  out = st->out;
    i32             inorm, onorm, sspan, uspan;
    if (tin == tout) return;
    inorm = anglesAdd(tin, anglesDEG090);
    onorm = anglesAdd(tout, anglesDEG090);
    sspan = anglesSignedSpan(inorm, onorm);
    uspan = (sspan < 0) ? -sspan : sspan;
    if (uspan <= MIN_CORNER) return;
    if (sspan > 0) {
        /* inside corner */
        lineToPoint(env, st, x, y);
	if (doeError_occurred(env)) return;
        lineToPenPoint(env, st, x, y, onorm);

    } else if (st->corners == dcPathStroker_ROUND) {
        /* round outside corner */
        if (uspan > MAX_CUBIC_GAP) {
            i32   mnorm = inorm + (anglesSignedSpan(inorm, onorm) / 2);
            penSection(env, st, x, y, inorm, mnorm);
	    if (doeError_occurred(env)) return;
            penSection(env, st, x, y, mnorm, onorm);
        } else {
            penSection(env, st, x, y, inorm, onorm);
        }
    } else if (st->corners == dcPathStroker_BEVEL) {
        /* outside bevel */
        lineToPenPoint(env, st, x, y, onorm);
    } else /* if (st->corners == dcPathStroker_MITER) */ {
        /* outside miter */
        f32 mfactor = 0;
        ixx collapse = ((uspan + 1) >= anglesDEG180);
        if (!collapse) mfactor = 1.0F / anglesCos((uspan + 1)/ 2);
        if (collapse || (mfactor > st->miterLimit)) {
            lineToPenPoint(env, st, x, y, onorm);
        } else {
            i32   mnorm   = inorm + (anglesSignedSpan( inorm, onorm ) / 2);
            lineToPolarPoint(env, st, x, y, st->penRadius * mfactor, mnorm);
	    if (doeError_occurred(env)) return;
            lineToPenPoint(env, st, x, y, onorm);
        }
    }
}

/*-----------------------
 * Envolvent of an arc
 */

#define FORWARD               0
#define BACKWARD              1

static void
envolvent(doeE env, dcPathStrokerData* st, u8 type, f32* pnts,
					    i32* tans, ixx dir)
{
    if (type == dcPathStorageLINE) {
        i32   norm;
        if (dir == FORWARD) {
            norm = anglesAdd(tans[0], anglesDEG090);
            lineToPenPoint(env, st, *(pnts+2), *(pnts+3), norm);
        } else /* if( dir == BACKWARD ) */ {
            norm = anglesAdd(tans[0], anglesDEG270);
            lineToPenPoint(env, st, *pnts, *(pnts+1), norm);
        }
    } else {
        i32    bnorm, enorm;
        if (dir == FORWARD) {
          bnorm = anglesAdd(tans[0], anglesDEG090),
          enorm = anglesAdd(tans[1], anglesDEG090);
          if (type == dcPathStorageQUADRATIC) {
              quadEnvolvent(env, st, pnts+2, pnts+4, bnorm, enorm);
          } else /* if( type == dcPathStorageCUBIC ) */ {
              cubicEnvolvent(env, st, pnts+2, pnts+4, pnts+6, bnorm, enorm);
          }
        } else /* if( dir == BACKWARD ) */ {
            enorm = anglesAdd(tans[0], anglesDEG270),
            bnorm = anglesAdd(tans[1], anglesDEG270);
            if (type == dcPathStorageQUADRATIC) {
                quadEnvolvent(env, st, pnts+2, pnts, bnorm, enorm);
            } else /* if( type == dcPathStorageCUBIC ) */ {
                cubicEnvolvent(env, st, pnts+4, pnts+2, pnts, bnorm, enorm);
            }
        }
    }
}

/*-----------------------
 * To stroke a stored subpath
 */

#define backward(t)       anglesAdd(t, anglesDEG180)

#define subpathEnd(e)     (((e) == dcPathStorageEND_CLOSED_SUBPATH) ||	\
                           ((e) == dcPathStorageEND_OPEN_SUBPATH))

#define nextArc(e,p,t)    {							\
                            if (*e == dcPathStorageLINE) {p+=2; t+=1;}		\
                            else if (*e == dcPathStorageQUADRATIC) {p+=4;t+=2;}	\
                            else/*if(*e == dcPathSTorageCUBIC) */  {p+=6;t+=2;}	\
                            e++;						\
                          }
#define prevArc(e,p,t)    { e--;						\
                            if (*e == dcPathStorageLINE) {p-=2; t-=1;}		\
                            else if (*e == dcPathStorageQUADRATIC) {p-=4;t-=2;}	\
                            else/*if(*e == dcPathSTorageCUBIC ) */ {p-=6;t-=2;}	\
                          }

static void
strokeSubpath(doeE env, dcPathStrokerData* st)
{
    dcPathConsumer  out = st->out;
    dcPathStorage   sp  = st->subpath;
    u8*             elm = (*sp)->getElements(env, sp);
    f32*            pnt = (*sp)->getPoints(env, sp);
    i32*            tan = (*sp)->getTangents(env, sp);
    /* skip the header */
    if (*elm == dcPathStorageBEGIN_PATH_NO_BOX) { 
        elm++;
    } else if (*elm == dcPathStorageBEGIN_PATH_WITH_BOX) { 
        elm++; pnt += 4;
    } else return;					/* empty */
    if (*elm != dcPathStorageBEGIN_SUBPATH)		/* empty */
	return;
    if (subpathEnd( *(elm+1))) {
        /* path consists of a single point */
        if (st->caps != dcPathStroker_ROUND) {
            return;					/* meaningless */
        } else {
            /* stroke is a circle around the only point of the path */
            f32  x = *pnt,
                 y = *(pnt+1);
            initialPoint(env, st, x, y, anglesDEG000);
	    if (doeError_occurred(env)) return;
            clockwiseCap(env, st, x, y, anglesDEG000);
	    if (doeError_occurred(env)) return;
            clockwiseCap(env, st, x, y, anglesDEG180);
	    if (doeError_occurred(env)) return;
            (*out)->closedSubpath(env, out);
        }
    } else {
        /* the subpath has at least one arc */
        i32  firsttan = *tan, 
             lasttan;
        ixx  isclosed;

        initialPoint(env, st, *pnt, *(pnt+1), firsttan);
	if (doeError_occurred(env)) return;
        elm++;

        /* left half of the stroke */
        while (1) {
            envolvent(env, st, *elm, pnt, tan, FORWARD);
	    if (doeError_occurred(env)) return;
            nextArc(elm, pnt, tan);
            if (subpathEnd(*elm)) break;
            leftCorner(env, st, *pnt, *(pnt+1), *(tan-1), *tan);
	    if (doeError_occurred(env)) return;
        }
        /* ending the left half */
        lasttan  = *(tan-1);
        isclosed = ((*elm) == dcPathStorageEND_CLOSED_SUBPATH);
        if (isclosed) {
            leftCorner(env, st, *pnt, *(pnt+1), lasttan, firsttan);
	    if (doeError_occurred(env)) return;
            (*out)->closedSubpath(env, out);
	    if (doeError_occurred(env)) return;
            initialPoint(env, st, *pnt, *(pnt+1), backward(lasttan));
        } else {
            clockwiseCap(env, st, *pnt, *(pnt+1), lasttan);
        }
	if (doeError_occurred(env)) return;

        /* right (clockwise) half of the stroke */
        while (1) {
            prevArc(elm, pnt, tan);
            envolvent(env, st, *elm, pnt, tan, BACKWARD);
	    if (doeError_occurred(env)) return;
            if (*(elm-1) == dcPathStorageBEGIN_SUBPATH) break; /* first arc */
            leftCorner(env, st, *pnt, *(pnt+1), backward(*tan), backward(*(tan-1)));
	    if (doeError_occurred(env)) return;
        }
        /* ending for the right half */
        if (isclosed) {
            leftCorner(env, st, *pnt, *(pnt+1), backward(*tan), backward(lasttan));
        } else {
            clockwiseCap(env, st, *pnt, *(pnt+1), backward(*tan));
        }
	if (doeError_occurred(env)) return;

        /* the end */
        (*out)->closedSubpath(env, out);
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
    return "dcPathStroker";
}

static void    dcPathStroker_copyinit(doeE, dcPathStroker, dcPathStroker  src);
static doeObject
copy(doeE env, doeObject o)
{
    dcPathStrokerData* st = 
        (dcPathStrokerData*)doeMem_malloc(env, sizeof(dcPathStrokerData));
    if (st == NULL) {
        doeError_setNoMemory(env);
	return NULL;
    }
    dcPathStroker_copyinit(env, (dcPathStroker)st, (dcPathStroker)o);
    return (doeObject)st;
}

static void
_cleanup(doeE env, doeObject o)
{
    dcPathStrokerData* st = (dcPathStrokerData*)o;
    (*(doeObject)(st->subpath))->_cleanup(env, (doeObject)(st->subpath));
    doeMem_free(env, (void*)(st->subpath));
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    dcPathStrokerData* st = (dcPathStrokerData*)o;
    cb->enumerate(env, cb, (doeObject)st->out);
    cb->enumerate(env, cb, (doeObject)st->subpath);
}

/*----------------------
 * The implementation of the dcPathConsumer interface
 */

static f32
fittingScale(f32 c1, f32 c2, f32 rad, f32 unit, ixx minradinunits)
{
    i32 angmax       = anglesAtan2(c2, c1);
    f32 max          = rad * ((c1 * anglesCos(angmax)) + (c2 * anglesSin(angmax)));
    i32 maxinunits   = (i32)((max / unit) + 0.5F);
    if (maxinunits < 0) maxinunits = -maxinunits;
    if (maxinunits < minradinunits) maxinunits = minradinunits;
    return (((f32)maxinunits * unit) / max);
}

static void
computeTransformations(dcPathStrokerData* st)
{
    f32  r[4];
    f32  d[2];
    ixx  isidentity,
         isjustt6;
    affineT4DxyFromT6(r, d, st->t6);
    isidentity = affineT6IsIdentity(st->t6);
    isjustt6 = 1;
    if (!st->penT4IsIdentity) {
        f32  tmp4[4];
        affineT4Copy(tmp4, r);
        affineT4Multiply(r, st->penT4, tmp4);
        isidentity = 0;
        isjustt6 = 0;
    }
    if (st->penNeedsFitting) {
        f32  fit4[4];
        f32  tmp4[4];
        fit4[0] = fittingScale(r[0], r[2], st->penRadius, st->unit, st->minimumRadius);
        fit4[1] = fit4[2] = 0.0F;
        fit4[3] = fittingScale(r[1], r[3], st->penRadius, st->unit, st->minimumRadius);
        affineT4Copy(tmp4, r);
        affineT4Multiply(r, tmp4, fit4);
        isidentity = 0;
        isjustt6 = 0;
    }
    /* set the input transformation */
    if (isidentity || isjustt6) {
        affineT4MakeIdentity(st->inputT4);
        st->inputT4IsIdentity = 1;
    } else {
        f32  invr[4];
        affineT4Invert(invr, r);
        affineT4Multiply(st->inputT4, st->t6, invr);
        st->inputT4IsIdentity = 0;
    }
    /* set the output transformation */
    if (isidentity && st->t6IsIdentity) {
        affineT6MakeIdentity(st->outputT6);
        st->outputT6IsIdentity = 1;
    } else {
        affineT6FromT4Dxy(st->outputT6, r, d);
        st->outputT6IsIdentity = 0;
    }
}

static void
endOfSubpath(doeE env, dcPathStrokerData* st)
{
    dcPathStorage  sp  = st->subpath;
    dcPathConsumer spc = (dcPathConsumer)sp;

    if ((st->isClosed)) {
        if ((st->firstX != st->lastX) || (st->firstY != st->lastY)) {
	    (*spc)->appendLine(env, spc, st->firstX, st->firstY);
	    if (doeError_occurred(env)) return;
	    (*sp)->appendTangent(env, sp,
			anglesAtan2(st->firstY - st->lastY, st->firstX - st->lastX));
        }
        (*spc)->closedSubpath(env, spc);
	if (doeError_occurred(env)) return;
    }
    (*spc)->endPath(env, spc);
    if (doeError_occurred(env)) return;

    strokeSubpath(env, st); 
    if (doeError_occurred(env)) return;
    (*sp)->reset(env, sp);
}

static f32
boxEnlargement(dcPathStrokerData* st)
{
    f32 factor = (st->caps == dcPathStroker_SQUARE) ? 1.414213562F : 1.0F;
    if ((st->corners == dcPathStroker_MITER) && (st->miterLimit > factor))
        factor = st->miterLimit;
    return (st->penRadius * factor);
}

static void
beginPath(doeE env, dcPathConsumer o)
{
    dcPathStrokerData* st   = (dcPathStrokerData*)o;
    dcPathConsumer     out  = st->out;
    if (st->inPath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_beginPath);
	return;
    }
    st->inPath = 1;
    st->inSubpath = 0;
    computeTransformations(st);
    (*out)->beginPath(env, out);
}

static void
beginSubpath(doeE env, dcPathConsumer o, f32 x0, f32 y0)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathConsumer     spc = (dcPathConsumer)(st->subpath);
    if (!st->inPath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_beginSubpath);
	return;
    }
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
    } else
        st->inSubpath = 1;
    st->isClosed = 0;
    if (!st->inputT4IsIdentity)
        affineT4TransformPoint(st->inputT4, &x0, &y0);
    st->firstX = st->lastX = x0;
    st->firstY = st->lastY = y0;
    (*spc)->beginPath(env, spc);
    if (doeError_occurred(env)) return;
    (*spc)->beginSubpath(env, spc, x0, y0);
}

static void
appendLine(doeE env, dcPathConsumer o, f32 x1, f32 y1)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathStorage      sp  = st->subpath;
    dcPathConsumer     spc = (dcPathConsumer)sp;
    if (!st->inSubpath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_appendLine);
	return;
    }
    if (!st->inputT4IsIdentity)
        affineT4TransformPoint(st->inputT4, &x1, &y1);
    (*spc)->appendLine(env, spc, x1, y1);
    if (doeError_occurred(env)) return;
    (*sp)->appendTangent(env, sp, anglesAtan2(y1 - st->lastY, x1 - st->lastX));
    st->lastX = x1; st->lastY = y1;
}

static void
appendQuadratic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathStorage      sp  = st->subpath;
    f32                pnts[6];
    if (!st->inSubpath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_appendQuadratic);
	return;
    }
    pnts[0] = st->lastX; pnts[1] = st->lastY; 
    pnts[2] = x1; pnts[3] = y1;
    pnts[4] = x2; pnts[5] = y2;
    if (!st->inputT4IsIdentity) 
        affineT4TransformPoints(st->inputT4, pnts+2, 2);
    processQuadratic(env, sp, pnts, 0);
    if (doeError_occurred(env)) return;
    st->lastX = pnts[4]; st->lastY = pnts[5];
}

static void
appendCubic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2,
					 f32 x3, f32 y3)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathStorage      sp  = st->subpath;
    f32                pnts[8];
    if (!st->inSubpath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_appendCubic);
	return;
    }
    pnts[0] = st->lastX; pnts[1] = st->lastY; 
    pnts[2] = x1; pnts[3] = y1;
    pnts[4] = x2; pnts[5] = y2;
    pnts[6] = x3; pnts[7] = y3;
    if (!st->inputT4IsIdentity)
        affineT4TransformPoints(st->inputT4, pnts+2, 3);
    processCubic(env, sp, pnts, 0);
    if (doeError_occurred(env)) return;
    st->lastX = pnts[6]; st->lastY = pnts[7];
}

static void
closedSubpath(doeE env, dcPathConsumer o)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathConsumer     spc  = (dcPathConsumer)(st->subpath);
    if (!st->inSubpath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_closedSubpath);
	return;
    }
    st->isClosed = 1;
    (*spc)->closedSubpath(env, spc);
}

static void
endPath(doeE env, dcPathConsumer o)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    dcPathConsumer     out = st->out;
    dcPathStorage      sp  = st->subpath;
    if (!st->inPath) {
        doeError_set(env, dcPathError, dcPathError_UNEX_endPath);
	return;
    }
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
        st->isClosed = 0;
        st->inSubpath = 0;
    } 
    st->inPath = 0;
    (*out)->endPath(env, out);
}

static void
useProxy(doeE env, dcPathConsumer o, dcFastPathProducer fpp)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
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
setPenDiameter(doeE env, dcPathStroker o, f32 diameter)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setPenDiameter);
	return;
    }
    if (diameter < 0.0F) {
        doeError_set(env, dcPRError, dcPRError_BAD_pendiam);
	return;
    }
    st->penRadius = diameter / 2.0F;
}

static void
setPenT4(doeE env, dcPathStroker o, f32* t4)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setPenT4);
	return;
    }
    if (t4 == NULL) {
        affineT4MakeIdentity(st->penT4);
        st->penT4IsIdentity = 1;
    } else {
        if (affineT4IsSingular(t4)) {
	    doeError_set(env, dcPRError, dcPRError_BAD_pent4_singular);
	    return;
	}
        affineT4Copy(st->penT4, t4);
        st->penT4IsIdentity = affineT4IsIdentity(st->penT4);
    }
}

static void
setPenFitting(doeE env, dcPathStroker o, f32 unit, ixx mindiameter)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if ((unit < 0.0F) || (mindiameter < 0)) { 
        doeError_set(env, dcPRError, dcPRError_BAD_penfit);
	return;
    }
    st->penNeedsFitting = ((unit > 0.0F) && (mindiameter > 0));
    st->unit = unit / 2.0F;
    st->minimumRadius = mindiameter;  
}

static void
setCaps(doeE env, dcPathStroker o, ixx caps)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setCaps);
	return;
    }
    if (!((caps == dcPathStroker_ROUND)  ||
	  (caps == dcPathStroker_SQUARE) ||
          (caps == dcPathStroker_BUTT))) {
	doeError_set(env, dcPRError, dcPRError_UNK_caps);
	return;
    }
    st->caps = caps;
}

static void
setCorners(doeE env, dcPathStroker o, ixx corners, f32 miterlimit)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setCorners);
	return;
    }
    if (!((corners == dcPathStroker_ROUND) || 
          (corners == dcPathStroker_BEVEL) ||
          (corners == dcPathStroker_MITER))) {
	doeError_set(env, dcPRError, dcPRError_UNK_corners);
	return;
    }
    if ((corners == dcPathStroker_MITER) && (miterlimit < 1.0F)) {
	doeError_set(env, dcPRError, dcPRError_BAD_miterlimit);
	return;
    }
    st->corners = corners;
    st->miterLimit = miterlimit;
}

static void
setOutputT6(doeE env, dcPathStroker o, f32* t6)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setOutputT6);
	return;
    }
    if (t6 == NULL) {
        affineT6MakeIdentity(st->t6);
    } else {
        if (affineT6IsSingular(t6)) {
	    doeError_set(env, dcPRError, dcPRError_BAD_outputt6_singular);
	    return;
	}
        affineT6Copy( st->t6, t6 );
   }
}

static void
setOutputConsumer(doeE env, dcPathStroker o, dcPathConsumer dst)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    if (st->inPath) { 
        doeError_set(env, dcPRError, dcPRError_UNEX_setOutputConsumer);
	return;
    }
    st->out = dst;
}

static void
defaultStateValues(dcPathStrokerData* st)
{
    st->inPath = 0;
    st->penRadius = 0.5;
    st->caps = dcPathStroker_ROUND;
    st->corners = dcPathStroker_ROUND;
    st->miterLimit = 0.0;
    st->unit = 0.0F;
    st->minimumRadius = 0;
    st->penNeedsFitting = 0;
    affineT4MakeIdentity(st->penT4);
    st->penT4IsIdentity = 1;
    affineT6MakeIdentity(st->t6);
    st->t6IsIdentity = 1;
}

static void
reset(doeE env, dcPathStroker o)
{
    dcPathStrokerData* st  = (dcPathStrokerData*)o;
    defaultStateValues(st);
    (*(st->subpath))->reset(env, st->subpath);
}


/*----------------------
 * The class variable
 */

dcPathStrokerFace dcPathStrokerClass = {    
    {
	{
	    sizeof(dcPathStrokerData),
	    className,                     /* object interface */
	    copy,
	    _cleanup,
	    _enumCoObs,
	    doeObject_uproot
	},

	beginPath,                     /* PathConsumer interface */
	beginSubpath,
	appendLine,
	appendQuadratic,
	appendCubic,
	closedSubpath,
	endPath,
	useProxy
    },

    setPenDiameter,               /* PathStroker interface */
    setPenT4,
    setPenFitting,
    setCaps,
    setCorners,
    setOutputT6,
    setOutputConsumer,
    reset
};

/*----------------------
 * The class-related global functions
 */

static void
dcPathStroker_init(doeE env, dcPathStroker o, dcPathConsumer destination)
{
    dcPathStrokerData* st = (dcPathStrokerData*)o;
    dcPathConsumer_init(env, (dcPathConsumer)o);
    *o = &dcPathStrokerClass;
    defaultStateValues(st);
    st->out = destination;
    st->subpath = dcPathStorage_create(env, 1);
}

dcPathStroker
dcPathStroker_create(doeE env, dcPathConsumer destination)
{
    dcPathStrokerData* st = 
        (dcPathStrokerData*)doeMem_malloc(env, sizeof(dcPathStrokerData));
    if (st == NULL) { 
        doeError_setNoMemory(env);
	return NULL;
    }
    dcPathStroker_init(env, (dcPathStroker)st, destination);
    return  (dcPathStroker)st;
}

#define objectCopy(env,t,o) (t)((*(doeObject)(o))->copy(env, (doeObject)(o)))

void
dcPathStroker_copyinit(doeE env, dcPathStroker o, dcPathStroker src)
{
    dcPathStrokerData* st = (dcPathStrokerData*)o;
    dcPathStrokerData* srcst = (dcPathStrokerData*)src;

    dcPathConsumer_copyinit(env, (dcPathConsumer)o, (dcPathConsumer)src);
    st->inPath = srcst->inPath;
    st->penRadius = srcst->penRadius;
    st->caps = srcst->caps;
    st->corners = srcst->corners;
    st->miterLimit = srcst->miterLimit;
    st->unit = srcst->unit;
    st->minimumRadius = srcst->minimumRadius;
    st->penNeedsFitting = srcst->penNeedsFitting;
    affineT4Copy(st->penT4, srcst->penT4);
    st->penT4IsIdentity = srcst->penT4IsIdentity;
    affineT6Copy(st->t6, srcst->t6);
    st->t6IsIdentity = srcst->t6IsIdentity;
    st->out = srcst->out;
    affineT4Copy(st->inputT4, srcst->inputT4);
    st->inputT4IsIdentity = srcst->inputT4IsIdentity;
    affineT6Copy(st->outputT6, srcst->outputT6);
    st->outputT6IsIdentity = srcst->outputT6IsIdentity;
    st->inSubpath = srcst->inSubpath;
    st->subpath = objectCopy(env, dcPathStorage, srcst->subpath);
}

void dcPathStroker_staticInitialize(doeE env) {}
void dcPathStroker_staticFinalize  (doeE env) {}
