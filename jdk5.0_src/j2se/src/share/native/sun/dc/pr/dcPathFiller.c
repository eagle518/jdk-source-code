/*
 * @(#)dcPathFiller.c	1.22 04/03/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathFiller.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 *	
 *	Methods used in this code are protected by U.S. Patent 5,438,656.
 *	Foreign patents are pending.
 * ---------------------------------------------------------------------
 *
 */

#include "dcPathFiller.h"
#include "dcPathStore-p.h"
#include "dcPool.h"
#include "doe.h"
#include "doeObject-p.h"
#include "dcPRError.h"
#include "dcPRException.h"
#include "dcPathError.h"
#include "dcPathException.h"

#ifdef DEBUG_MEM
    static char	aux[1000];
#endif

ixx	dcPathFiller_tileSizeL2S;
ixx	dcPathFiller_tileSize;
f32	dcPathFiller_tileSizeF;

typedef struct RunData_*		Run;
typedef struct LeftSideData_*		LeftSide;

typedef struct FastOutputPCFace_**	FastOutputPC;


#define BASE dcPathStore

typedef struct dcPathFillerData_ {
    dcPathStoreData	mu;

    bool		redundantReset;

    dcFastPathProducer	thisFPP; /* ie, the FPP face of this */

    ixx			state;
#define	setFillModeDone		1
#define	setOutputAreaDone	2
#define stateResetAll(p)	(p)->state = 0
#define stateReset(p,f)		(p)->state &= (~(f))
#define stateSet(p,f)		(p)->state |= (f)
#define stateCheck(p,f)		((p)->state&(f))

    ixx			fillmode;

    f32			pathBox[4];

    Run**		tileRuns;
    ixx			tileXI, tileYI;

    ixx			rowH;	    /* in pixels */
    f32			rowHTiF;    /* tileSize-scaled (ie, <= 1.0F) */

    bool		fastOutput;
    f32			outLoX,  outLoY;	/* pixels */
    i32			outW,    outH;		/* pixels */
    i32			outWTi,  outHTi;	/* tiles */
    f32			outWTiF, outHTiF;	/* tiles */

    f32			*xt, *yt; /* aux arrays used in [processToRunsArc1] */
    i32			xtsize, ytsize;

    dcPathConsumer	runsBuilder;
    LeftSide		lsEffects;
    FastOutputPC	fastOutputPC;

    dcPool		poolRun;
    dcPool		poolLeftSide;
} dcPathFillerData;

static char*		className(doeE env, doeObject o) { return "dcPathFiller"; }

static doeObject
copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
_cleanup(doeE env, doeObject o)
{
    dcPathFiller	pf = (dcPathFiller)o;
    dcPathFillerData*	p  = (dcPathFillerData*)o;
    doeObject		po;

    (*pf)->reset(env, pf);

    po = (doeObject)(p->runsBuilder);
    if (po != NULL) {
	(*po)->_cleanup(env, po);
	doeMem_free(env, po);
    }

    po = (doeObject)(p->fastOutputPC);
    if (po != NULL) {
	(*po)->_cleanup(env, po);
	doeMem_free(env, po);
    }

    dcPool_destroy(env, p->poolRun);
    dcPool_destroy(env, p->poolLeftSide);

    if (p->xt != NULL)	doeMem_free(env, p->xt);
    if (p->yt != NULL)	doeMem_free(env, p->yt);

    BASE__cleanup(env, o);
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    dcPathFillerData*	p  = (dcPathFillerData*)o;

    if (p->runsBuilder != NULL)
	cb->enumerate(env, cb, (doeObject)(p->runsBuilder));
    if (p->fastOutputPC != NULL)
	cb->enumerate(env, cb, (doeObject)(p->fastOutputPC));

    BASE__enumCoObs(env, o, cb);
}

#undef BASE

/*  internal static class Run (just a data structure) ------------------*/

#define	runarcsLength		50
#define rspyImpossibleTss	10.0F

typedef struct RunData_ {
    Run			next;

    f32			rspy0, rspy1; /* tile-relative, tileSize-scaled */

/*  runs have the form
	{ x y {arctype { x y }+ }+ }
    where
	arctype in {TYPE_Arc1, TYPE_Arc2, TYPE_Arc3}
    coordinates are tile-relative U pixels, stored
    as [i16]; [ra1stavail] is the index of the 1st
    empty entry in [runarcs] */
#define Run_TYPE_Arc1	1
#define Run_TYPE_Arc2	2
#define Run_TYPE_Arc3	3
    i16			runarcs[runarcsLength];
    ixx			ra1stavail;

    f32			lastxend, lastyend;
} RunData;

static Run
Run_create(doeE env, dcPool pool, i16 x0, i16 y0)
{
    Run r = (Run)dcPool_getItem(env, pool);
    if (r == NULL)	return NULL;

    r->rspy1 = rspyImpossibleTss;
    r->runarcs[0] = x0;
    r->runarcs[1] = y0;
    r->ra1stavail = 2;
    r->lastxend = r->lastyend = 0.0F;
    r->next = NULL;

    return r;
}

static void
Run_releaseList(doeE env, Run l)
{
    Run p;

    while (l != NULL) {
	p = l->next;
	dcPool_staticReleaseItem(env, l);
	l = p;
    }
}

/*- internal (nonstatic) class RunsBuilder (a PathConsumer) ----------*/

#define BASE doeObject

typedef struct RunsBuilderData_ {
    doeObjectData	mu;

    f32			spx0, spy0;	/* subpath begin,  OA-rel, tss */
    f32			x0,   y0;	/* previous point, OA-rel, tss */
    bool		subpathIs1st;

    dcPathFillerData*	pfp;		/* this.this */
} RunsBuilderData;

static char*
RunsBuilder_className(doeE env, doeObject o) { return "PathFiller_RunsBuilder"; }

static doeObject
RunsBuilder_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
RunsBuilder__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

static void
RunsBuilder__enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    BASE__enumCoObs(env, o, cb);
}

/* class PathFiller private methods used in RunsBuilder */

/*		NOTE: All [processToRuns*] methods, as well as
		[appendToRun*] and [runCheckForArcAppend] are
		PathFiller's private methods; they are invoked only
		from methods in PathFiller's internal class
		RunsBuilder, which refers to the enclosing PathFiller
		instance through a [PathFillerData*] rather than the
		usual [PathFiller], so signatures have been modified
		accordingly.
 */

static void	processToRunsArc1(doeE env, dcPathFillerData* pfp,	f32 x0, f32 y0,
									f32 x1, f32 y1);
static void	processToRunsArc2(doeE env, dcPathFillerData* pfp, 	f32 x0, f32 y0,
									f32 x1, f32 y1,
									f32 x2, f32 y2);
static void	processToRunsArc3(doeE env, dcPathFillerData* pfp,	f32 x0, f32 y0,
									f32 x1, f32 y1,
									f32 x2, f32 y2,
									f32 x3, f32 y3);

static void
RunsBuilder_beginPath(doeE env, dcPathConsumer pc)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    p->subpathIs1st = TRUE;
}

static void
RunsBuilder_beginSubpath(	doeE env, dcPathConsumer pc,
				f32 x0, f32 y0)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    if (!p->subpathIs1st && (p->x0 != p->spx0 || p->y0 != p->spy0)) {
	processToRunsArc1(env, pfp, p->x0, p->y0, p->spx0, p->spy0);
    }

    x0 -= pfp->outLoX; x0 /= dcPathFiller_tileSizeF;
    y0 -= pfp->outLoY; y0 /= dcPathFiller_tileSizeF;

    p->spx0 = p->x0 = x0;
    p->spy0 = p->y0 = y0;

    p->subpathIs1st = FALSE;
}

static void
RunsBuilder_appendLine(		doeE env, dcPathConsumer pc,
				f32 x1, f32 y1)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    x1 -= pfp->outLoX; x1 /= dcPathFiller_tileSizeF;
    y1 -= pfp->outLoY; y1 /= dcPathFiller_tileSizeF;

    processToRunsArc1(env, pfp, p->x0, p->y0, x1, y1);

    p->x0 = x1;
    p->y0 = y1;
}

static void
RunsBuilder_appendQuadratic(	doeE env, dcPathConsumer pc,
				f32 x1, f32 y1, f32 x2, f32 y2)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    x1 -= pfp->outLoX; x1 /= dcPathFiller_tileSizeF;
    y1 -= pfp->outLoY; y1 /= dcPathFiller_tileSizeF;
    x2 -= pfp->outLoX; x2 /= dcPathFiller_tileSizeF;
    y2 -= pfp->outLoY; y2 /= dcPathFiller_tileSizeF;

    processToRunsArc2(env, pfp, p->x0, p->y0, x1, y1, x2, y2);

    p->x0 = x2;
    p->y0 = y2;
}

static void
RunsBuilder_appendCubic(	doeE env, dcPathConsumer pc,
				f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    x1 -= pfp->outLoX; x1 /= dcPathFiller_tileSizeF;
    y1 -= pfp->outLoY; y1 /= dcPathFiller_tileSizeF;
    x2 -= pfp->outLoX; x2 /= dcPathFiller_tileSizeF;
    y2 -= pfp->outLoY; y2 /= dcPathFiller_tileSizeF;
    x3 -= pfp->outLoX; x3 /= dcPathFiller_tileSizeF;
    y3 -= pfp->outLoY; y3 /= dcPathFiller_tileSizeF;

    processToRunsArc3(env, pfp, p->x0, p->y0, x1, y1, x2, y2, x3, y3);

    p->x0 = x3;
    p->y0 = y3;
}

static void
RunsBuilder_closedSubpath(	doeE env, dcPathConsumer pc) {}

static void
RunsBuilder_endPath(		doeE env, dcPathConsumer pc)
{
    RunsBuilderData*	p = (RunsBuilderData*)pc;
    dcPathFillerData*	pfp = p->pfp;

    if (!p->subpathIs1st && (p->x0 != p->spx0 || p->y0 != p->spy0)) {
	processToRunsArc1(env, pfp, p->x0, p->y0, p->spx0, p->spy0);
    }
}

static void
RunsBuilder_useProxy(		doeE env, dcPathConsumer pc,
				dcFastPathProducer proxy)
{
    /* impossible */
}

static dcPathConsumerFace RunsBuilderClass = {
    {
	sizeof(RunsBuilderData),
	
	RunsBuilder_className,
	RunsBuilder_copy,
	RunsBuilder__cleanup,
	RunsBuilder__enumCoObs,
	doeObject_uproot
    },
    
	RunsBuilder_beginPath,
	RunsBuilder_beginSubpath,
	RunsBuilder_appendLine,
	RunsBuilder_appendQuadratic,
	RunsBuilder_appendCubic,
	RunsBuilder_closedSubpath,
	RunsBuilder_endPath,
	RunsBuilder_useProxy
};

static void
RunsBuilder_init(doeE env, dcPathConsumer target, dcPathFillerData* pfp)
{
    RunsBuilderData*	p = (RunsBuilderData*)target;

    BASE_init(env, target);
    if (doeError_occurred(env)) {
	BASE__cleanup(env, target);
	return;
    }

    *target = &RunsBuilderClass;

    p->pfp = pfp;
}

static dcPathConsumer
RunsBuilder_create(doeE env, dcPathFillerData* pfp)
{
    dcPathConsumer	p = doeMem_malloc(env, (i32)sizeof(RunsBuilderData));

    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }

    RunsBuilder_init(env, p, pfp);
    if (doeError_occurred(env)) {
	doeMem_free(env, p);
	p = NULL;
    }

    return p;
}

/*- implementation of processToRuns methods -----------------------*/

/*- other PathFiller private methods called by processToRuns ------*/

static void appendToRunArc1(	doeE env, dcPathFillerData* p,
				f32 x0, f32 y0, f32 x1, f32 y1,
				i32 tilexi, i32 tileyi);
static void appendToRunsArc2(	doeE env, dcPathFillerData* p,
				f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2,
				i32 tileloxi, i32 tileloyi,
				i32 tilehixi, i32 tilehiyi);
static void appendToRunsArc3(	doeE env, dcPathFillerData* p,
				f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3,
				i32 tileloxi, i32 tileloyi,
				i32 tilehixi, i32 tilehiyi);

static void
integralCoordTs(f32* c, f32 c0, f32 c1) {
/*  computes and returns an array [c] containing the parameter values
    corresponding to the initial coordinate point [c0] (that is, 0.0),
    to any intermediate points where the coordinate is integer, and to
    the final point [c1] (i.e., 1.0) */

    f32	dc = c1 - c0;
    i32 i0, i1;
    i32 n, i;

    c[0] = 0.0F;
    c[1] = 1.0F;

    if (dc == 0)	return;

    if (dc > 0) {
	i0 = (i32)floor(c0) + 1; /* i0 > c0 */
	i1 = (i32)ceil (c1) - 1; /* i1 < c1 */
	n = i1 - i0 + 1; /* ints between [c0] and [c1], both excluded */

	if (n <= 0)	return;

	i = 1;
	do {
	    c[i++] = ((f32)i0 - c0) / dc;
	    i0++;
	} while (i0 <= i1);

    } else {
	i0 = (i32)ceil (c0) - 1; /* i0 < c0 */
	i1 = (i32)floor(c1) + 1; /* i1 > c1 */
	n = i0 - i1 + 1;
	if (n <= 0)	return;

	i = 1;
	do {
	    c[i++] = ((f32)i0 - c0)/dc;
	    i0--;
	} while (i0 >= i1);
    }
    c[i] = 1.0F;

    return;
}

static void
processToRunsArc1(doeE env, dcPathFillerData* p,	f32 x0, f32 y0,
							f32 x1, f32 y1)
			/* OA-relative, tileSize-scaled coordinates */
{
    /*  NOTE: lines affecting multiple tiles are divided into segments
	affecting exactly one; in that sense, this method is rather
	different from its ...Arc2 and ...Arc3 cousins */
    {
	f32 arclox, arcloy, archix, archiy;

	if (x0 < x1)	{ arclox = x0; archix = x1; }
	else		{ arclox = x1; archix = x0; }
	if (y0 < y1)	{ arcloy = y0; archiy = y1; }
	else		{ arcloy = y1; archiy = y0; }

	/* if the arc cannot possibly affect OA, skip it */
	if (arclox >= p->outWTiF || archiy <= 0.0F || arcloy >= p->outHTiF) {
	    return;
	}

	/* unless the line is partially outside OA (not the most common
	   case), skip further testing and prunning */
	if (arclox < 0.0F || archix > p->outWTiF || arcloy < 0.0F || archiy > p->outHTiF) {
	/* prune the parts of the line above, below and to the right of OA */
	    f32 r = 0, outhtif, outwtif;

	    outhtif = p->outHTiF;
	    outwtif = p->outWTiF;

	    if (y1 != y0) {
		r = (x1 - x0) / (y1 - y0);
		if (y0 < 0.0F)      {	x0 -= y0 * r;		    y0 = 0.0F; }
	 	if (y1 < 0.0F)      {	x1 -= y1 * r;		    y1 = 0.0F; }
		if (y0 > outhtif) {	x0 += (outhtif - y0) * r;   y0 = outhtif; }
		if (y1 > outhtif) {	x1 += (outhtif - y1) * r;   y1 = outhtif; }
	    }
	    if (x1 != x0) {
		r = (y1 - y0) / (x1 - x0);
		if (x0 > outwtif) { y0 += (outwtif - x0) * r;   x0 = outwtif; }
		if (x1 > outwtif) { y1 += (outwtif - x1) * r;   x1 = outwtif; }
	    }

	    /* recompute arclox, archix */
	    if (x0 < x1)	{ arclox = x0; archix = x1; }
	    else		{ arclox = x1; archix = x0; }

	    /* if the clipped line cannot possibly affect OA, skip it */
	    if (arclox >= outwtif) {
		return;
	    }

	    /* if the line is partly or completely W of OA... */
	    if (arclox < 0.0F) {
		/* if the line is completely W of OA, use its projection;
		   if it is just partially W of OA, break it at x=0 and
		   process the parts recursively: one will be completly
		   inside OA, the other completely W (this may not be the
		   fastest way to do it, but the case seems sufficiently
		   rare to merit this simpler treatment) */
		if (archix <= 0.0F) {
		    x0 = x1 = arclox = archix = 0.0F;
		} else {
		    f32 ym = (x0 < 0.0F)? y0 - x0 * r : y1 - x1 * r;
		    processToRunsArc1(env, p, x0, y0, 0.0F, ym);
		    processToRunsArc1(env, p, 0.0F, ym, x1, y1);
		    return;
		}
	    }
	}
    }

    {
	/* the line is now inside OA (0<=x<=outW, 0<=y<=outH); divide
	   it in segments affecting exactly one tile */
	f32	dx = x1 - x0;
	f32	dy = y1 - y0;
	f32	*xt, *yt;
	i32	maxcoords;
	f32	t = 0;
	f32	xprev = x0;
	f32	yprev = y0;
	i32	xi = 1; /* xt[0] = yt[0] = 0.0F <-> x0,y0 */
	i32	yi = 1;

	maxcoords = ABS(dx); maxcoords += 3;
	if (p->xtsize < maxcoords) {
	    p->xt = doeMem_malloc(env, sizeof(f32)*maxcoords);
	    if (p->xt == NULL) {
		doeError_setNoMemory(env);
		return;
	    }
	    p->xtsize = maxcoords;
	}
	xt = p->xt;
	integralCoordTs(xt, x0, x1);

	maxcoords = ABS(dy); maxcoords += 3;
	if (p->ytsize < maxcoords) {
	    p->yt = doeMem_malloc(env, sizeof(f32)*maxcoords);
	    if (p->yt == NULL) {
		doeError_setNoMemory(env);
		return;
	    }
	    p->ytsize = maxcoords;
	}
	yt = p->yt;
	integralCoordTs(yt, y0, y1);

	do {
	    f32		xtt = xt[xi];
	    f32		ytt = yt[yi];
	    bool	xley = xtt <= ytt;
	    bool	ylex = ytt <= xtt;
	    i32		tileloxi, tileloyi;

	    if (xley) {	t = xtt; xi++; }
	    if (ylex) { t = ytt; yi++; }
	    if (t != 1.0F) {
		x1 = x0 + dx * t;	if (xley) x1 = ROUND(x1);
		y1 = y0 + dy * t;	if (ylex) y1 = ROUND(y1);
	    } else {
		x1 = x0 + dx;
		y1 = y0 + dy;
	    }
	    tileloxi = (i32)floor((dx > 0)? xprev : x1) + 1; /* tileRuns[0][0] is west of OA */
	    tileloyi = (i32)floor((dy > 0)? yprev : y1);
	    appendToRunArc1(env, p, xprev, yprev, x1, y1, tileloxi, tileloyi);
	    xprev = x1;
	    yprev = y1;
	} while (t != 1.0F);
    }
}

/*  In PathFiller.java, under the heading

	DIVIDING ARCS

    there's a lengthy and somewhat bogus discussion on the merits of
    dividing an arc vs. tracing it as is; it won't be repeated
    here... */

static f32	runCheckCost;
static f32	KArc2;
static f32	LArc2;
static f32	MArc2;
static f32	NArc2;
static f32	DIV2Arc2;
static f32	DIV4Arc2;
static f32	KArc3;
static f32	LArc3;
static f32	MArc3;
static f32	NArc3;
static f32	DIV2Arc3;
static f32	DIV4Arc3;

static void
processToRunsArc2(doeE env, dcPathFillerData* p,	f32 x0, f32 y0,
							f32 x1, f32 y1,
							f32 x2, f32 y2)
				/* OA-relative, tileSize-scaled coordinates */
{
    {
	f32 arclox, arcloy, archix, archiy;
	f32 dimx, dimy;

	if (x0 < x1)	{ arclox = x0; archix = x1; }
	else		{ arclox = x1; archix = x0; }
	if (y0 < y1)	{ arcloy = y0; archiy = y1; }
	else		{ arcloy = y1; archiy = y0; }
	if (x2 < arclox)	arclox = x2;
	if (x2 > archix)	archix = x2;
	if (y2 < arcloy)	arcloy = y2;
	if (y2 > archiy)	archiy = y2;

	/* if the arc cannot possibly affect OA, skip it */
	if (arclox >= p->outWTiF || archiy <= 0.0F || arcloy >= p->outHTiF) {
	    return;
	}

	/* if the arc lies W of OA, substitute a line whose effect on OA
	   is identical to the arc's */
	if (archix <= 0.0F) {
	    processToRunsArc1(env, p, 0.0F, y0, 0.0F, y2);
	    return;
	}

	/* compute the arc's maximum dimension */
	dimx = archix - arclox;
	dimy = archiy - arcloy;

	if ((dimx < 1.0F) && (dimy < 1.0F)) { /* otherwise there is no point
					     in checking... */
	    /* compute the extreme indeces [lo,]hi of the tiles
	       affected by this arc; */
	
	    i32 tileloxi = (i32)floor(arclox) + 1; /* tile[0] is W of OA */
	    i32 tilehixi = (i32)ceil (archix) + 1;
	    i32 tileloyi = (i32)floor(arcloy);
	    i32 tilehiyi = (i32)ceil (archiy);

	    i32 tilesx, tilesy;
	    f32 dimmax;

	    /* force tileloxi >= 0, tilehixi > tileloxi: a degenerate
	       vertical line between two tiles affects the right one */
	    if (tileloxi < 0)			tileloxi = 0;
	    if (tilehixi > p->outWTi + 1)	tilehixi = p->outWTi + 1;
	    if (tilehixi <= tileloxi)		tilehixi = tileloxi + 1;

	    /* force tileloyi >= 0, tilehiyi >= tileloyi: a degenerate
	       horizontal line between two tiles dissappears */
	    if (tileloyi < 0)			tileloyi = 0;
	    if (tilehiyi > p->outHTi)		tilehiyi = p->outHTi;
	    if (tilehiyi <= tileloyi)
		return;

	    tilesx = tilehixi - tileloxi;
	    tilesy = tilehiyi - tileloyi;
	    dimmax = (dimx >= dimy)? dimx : dimy;

	    /* by now 1<=tilesx<=2, 1<=tilesy<=2 */
	    if ((tilesx == 1 && tilesy == 1)	    ||
		(tilesx == 1 && dimy <= DIV2Arc2)   ||
		(tilesy == 1 && dimx <= DIV2Arc2)   ||
		(dimmax < DIV4Arc2)) {
		appendToRunsArc2(env, p,
				 x0, y0, x1, y1, x2, y2,
				 tileloxi, tileloyi, tilehixi, tilehiyi);
		return;
	    }
	}
    }

    {
	/* divide the arc and recursively process its halves */
	f32 x01 = .5F * (x0 + x1);
	f32 y01 = .5F * (y0 + y1);
	f32 x12 = .5F * (x1 + x2);
	f32 y12 = .5F * (y1 + y2);
	f32 x012 = .5F * (x01 + x12);
	f32 y012 = .5F * (y01 + y12);
	processToRunsArc2(env, p, x0, y0, x01, y01, x012, y012);
	processToRunsArc2(env, p, x012, y012, x12, y12, x2, y2);

	return;
    }
}


static void
processToRunsArc3(doeE env, dcPathFillerData* p,	f32 x0, f32 y0,
							f32 x1, f32 y1,
							f32 x2, f32 y2,
							f32 x3, f32 y3)
				/* OA-relative, tileSize-scaled coordinates */
{
    {
	f32 arclox, arcloy, archix, archiy;
	f32 dimx, dimy;

	if (x0 < x1)	{ arclox = x0; archix = x1; }
	else		{ arclox = x1; archix = x0; }
	if (y0 < y1)	{ arcloy = y0; archiy = y1; }
	else		{ arcloy = y1; archiy = y0; }
	if (x2 < arclox)	arclox = x2;
	if (x2 > archix)	archix = x2;
	if (y2 < arcloy)	arcloy = y2;
	if (y2 > archiy)	archiy = y2;
	if (x3 < arclox)	arclox = x3;
	if (x3 > archix)	archix = x3;
	if (y3 < arcloy)	arcloy = y3;
	if (y3 > archiy)	archiy = y3;

	/* if the arc cannot possibly affect OA, skip it */
	if (arclox >= p->outWTiF || archiy <= 0.0F || arcloy >= p->outHTiF) {
	    return;
	}

	/* if the arc lies W of OA, substitute a line whose effect on OA
	   is identical to the arc's */
	if (archix <= 0.0F) {
	    processToRunsArc1(env, p, 0.0F, y0, 0.0F, y3);
	    return;
	}

	/* compute the arc's maximum dimension */
	dimx = archix - arclox;
	dimy = archiy - arcloy;

	if ((dimx < 1.0F) && (dimy < 1.0F)) { /* otherwise there is no point
					     in checking... */
	    /* compute the extreme indeces [lo,]hi of the tiles
	       affected by this arc; */
	
	    i32 tileloxi = (i32)floor(arclox) + 1; /* tile[0] is W of OA */
	    i32 tilehixi = (i32)ceil (archix) + 1;
	    i32 tileloyi = (i32)floor(arcloy);
	    i32 tilehiyi = (i32)ceil (archiy);

	    i32 tilesx, tilesy;
	    f32 dimmax;

	    /* force tileloxi >= 0, tilehixi > tileloxi: a degenerate
	       vertical line between two tiles affects the right one */
	    if (tileloxi < 0)			tileloxi = 0;
	    if (tilehixi > p->outWTi + 1)	tilehixi = p->outWTi + 1;
	    if (tilehixi <= tileloxi)		tilehixi = tileloxi + 1;

	    /* force tileloyi >= 0, tilehiyi >= tileloyi: a degenerate
	       horizontal line between two tiles dissappears */
	    if (tileloyi < 0)			tileloyi = 0;
	    if (tilehiyi > p->outHTi)		tilehiyi = p->outHTi;
	    if (tilehiyi <= tileloyi)
		return;

	    tilesx = tilehixi - tileloxi;
	    tilesy = tilehiyi - tileloyi;
	    dimmax = (dimx >= dimy)? dimx : dimy;

	    /* by now 1<=tilesx<=2, 1<=tilesy<=2 */
	    if ((tilesx == 1 && tilesy == 1)	    ||
		(tilesx == 1 && dimy <= DIV2Arc2)   ||
		(tilesy == 1 && dimx <= DIV2Arc2)   ||
		(dimmax < DIV4Arc2)) {
		appendToRunsArc3(env, p,
				 x0, y0, x1, y1, x2, y2, x3, y3,
				 tileloxi, tileloyi, tilehixi, tilehiyi);
		return;
	    }
	}
    }
    {
	/* divide the arc and recursively process its halves */
	f32 x01 = .5F * (x0 + x1);
	f32 y01 = .5F * (y0 + y1);
	f32 x12 = .5F * (x1 + x2);
	f32 y12 = .5F * (y1 + y2);
	f32 x23 = .5F * (x2 + x3);
	f32 y23 = .5F * (y2 + y3);
	f32 x012 = .5F * (x01 + x12);
	f32 y012 = .5F * (y01 + y12);
	f32 x123 = .5F * (x12 + x23);
	f32 y123 = .5F * (y12 + y23);
	f32 x0123 = .5F * (x012 + x123);
	f32 y0123 = .5F * (y012 + y123);
	processToRunsArc3(env, p, x0, y0, x01, y01, x012, y012, x0123, y0123);
	processToRunsArc3(env, p, x0123, y0123, x123, y123, x23, y23, x3, y3);

	return;
    }
}

static Run
runCheckForArcAppend(	doeE env, dcPathFillerData* p,
			i32 tilexi, i32 tileyi,
			bool rtaa, /* is tile the rightmost tile affected by the arc? */
			f32 x0,   f32 y0,
			f32 xend, f32 yend,
			ixx entriesneeded)
/* point coordinates are tile-relative, tileSize-scaled */
{
    bool	run_is_1st,
		arc_extends_run,
		run_has_rsp,
		arc_extends_rsp;
    Run		r = p->tileRuns[tilexi][tileyi];

    /*  is this the first run for this tile? */
    run_is_1st = (r == NULL);

    /* does this arc extend the current run for this tile?  it does
    when the current run exists and its last arc ends where this one
    begins, of course, but also, suprisingly, when there is no current
    run!  (this is because "the current run" in this case means the
    new empty run that will be created) */
    arc_extends_run = run_is_1st || (r->lastxend==x0 && r->lastyend==y0);

    /* does the current run have a rightside projection? */
    run_has_rsp = !(run_is_1st || r->rspy1 == rspyImpossibleTss);

    /* does the arc extend the current run's rightside projection?  it
    does if there is a rightside projection that ends at the same Y
    where the arc begins, and also when there is no rightside
    projection (an empty one is extended by any arc) */
    arc_extends_rsp = !run_has_rsp || (r->rspy1 == y0);

    /* reasons to create a new current run for this tile:
       *	there is no run;
       *	the arc does not extend the current run;
       *	the arc does not extend the current run's rightside
		projection AND does not affect the rightside tile (if
		it did, it would not need to extend the rightside
		projection);
       *	there is no space for the arc
     */
    if (run_is_1st			||
	!arc_extends_run		||
	(!arc_extends_rsp && rtaa)	||
	(entriesneeded > runarcsLength - r->ra1stavail)) {

	Run newr = Run_create(env, p->poolRun, (i16)dcLLFiller_tss2U(x0), (i16)dcLLFiller_tss2U(y0));
	newr->next = r;
	p->tileRuns[tilexi][tileyi] = newr;
	r = newr;

	run_has_rsp = FALSE; /* a new run does not yet have a rightside projection */
    }

    /* update the end point */
    r->lastxend = xend;
    r->lastyend = yend;

    /* if appropriate - ie, if the arc does not directly affect the
    tile to the right of this one - update rightside projection */
    if (rtaa) {
	if (!run_has_rsp) {
	    r->rspy0 = y0;
	}
	r->rspy1 = yend;
    }

    return r;
}

static void
appendToRunArc1(doeE env, dcPathFillerData* p,
		f32 x0, f32 y0, f32 x1, f32 y1,
		int tilexi, int tileyi)
		/* coordinates are OA-relative, tileSize-scaled */
{
    Run r;
    f32	tileloxtss = (f32)tilexi - 1;
    f32	tileloytss = (f32)tileyi;

    /* convert to tile-relative */
    x0 -= tileloxtss;
    y0 -= tileloytss;
    x1 -= tileloxtss;
    y1 -= tileloytss;

    /* check and append */
    r = runCheckForArcAppend(env, p, tilexi, tileyi, TRUE, x0, y0, x1, y1, 3);
    r->runarcs[r->ra1stavail++] = Run_TYPE_Arc1;
    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x1);
    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y1);
}
				
static void
appendToRunsArc2(	doeE env, dcPathFillerData* p,
			f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2,
			int tileloxi, int tileloyi, int tilehixi, int tilehiyi)
			/* coordinates are OA-relative, tileSize-scaled */
{
    int xi, yi;

    for (yi = tileloyi; yi < tilehiyi; yi++) {
	/* convert to tile-relative */
	f32 tileloytss = (f32)yi;
	f32 y0t = y0 - tileloytss;
	f32 y1t = y1 - tileloytss;
	f32 y2t = y2 - tileloytss;

	for (xi = tileloxi; xi < tilehixi; xi++) {
	    bool rtaa = (xi == tilehixi - 1); /* rtaa = rightmost tile affected by arc */

	    /* convert to tile-relative */
	    f32	tileloxtss = (f32)xi - 1.0F;
	    f32	x0t = x0 - tileloxtss;
	    f32	x1t = x1 - tileloxtss;
	    f32	x2t = x2 - tileloxtss;

	    /* check and append */
	    Run r = runCheckForArcAppend(env, p, xi, yi, rtaa, x0t, y0t, x2t, y2t, 5);
	    r->runarcs[r->ra1stavail++] = Run_TYPE_Arc2;
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x1t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y1t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x2t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y2t);
	}
    }
}
				
static void
appendToRunsArc3(	doeE env, dcPathFillerData* p,
			f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3,
			int tileloxi, int tileloyi, int tilehixi, int tilehiyi)
			/* coordinates are OA-relative, tileSize-scaled */
{
    int xi, yi;
    for (yi = tileloyi; yi < tilehiyi; yi++) {
	/* convert to tile-relative */
	f32 tileloytss = (f32)yi;
	f32 y0t = y0 - tileloytss;
	f32 y1t = y1 - tileloytss;
	f32 y2t = y2 - tileloytss;
	f32 y3t = y3 - tileloytss;

	for (xi = tileloxi; xi < tilehixi; xi++) {
	    bool rtaa = (xi == tilehixi - 1); /* rtaa = rightmost affected tile */

	    /* convert to tile-relative */
	    f32	tileloxtss = (f32)xi - 1.0F;
	    f32	x0t = x0 - tileloxtss;
	    f32	x1t = x1 - tileloxtss;
	    f32	x2t = x2 - tileloxtss;
	    f32	x3t = x3 - tileloxtss;

	    /* check and append */
	    Run r = runCheckForArcAppend(env, p, xi, yi, rtaa, x0t, y0t, x3t, y3t, 7);
	    r->runarcs[r->ra1stavail++] = Run_TYPE_Arc3;
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x1t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y1t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x2t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y2t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(x3t);
	    r->runarcs[r->ra1stavail++] = (i16)dcLLFiller_tss2U(y3t);
	}
    }
}

#undef BASE

/*- internal static class LeftSide (just a data structure) -------------*/

typedef struct LeftSideData_ {
    LeftSide	next;

    /* a run of arcs beginning at y0 and ending at y1,
    both tile-relative, tileSize-scaled */
    f32		y0, y1;
} LeftSideData;

static LeftSide
LeftSide_create(doeE env, dcPool pool)
{
    LeftSide p = (LeftSide)dcPool_getItem(env, pool);
    if (p == NULL)	return NULL;

    p->next = NULL;

    return p;
}

static void
LeftSide_releaseList(doeE env, LeftSide l)
{
    LeftSide p;

    while (l != NULL) {
	p = l->next;
	dcPool_staticReleaseItem(env, l);
	l = p;
    }
}

/*- internal static class FastOutputPC (a PathConsumer) -------------*/

#define BASE dcPathConsumer

typedef struct FastOutputPCFace_ {
    dcPathConsumerFace	mu;
    void		(*setUpAlpha8 )(doeE env, FastOutputPC,
					dcLLFiller ll, f32 pathdx, f32 pathdy,
					u8 * alpha, i32 xstride, i32 ystride, i32 pix0offset);
    void		(*setUpAlpha16)(doeE env, FastOutputPC,
					dcLLFiller ll, f32 pathdx, f32 pathdy,
					u16* alpha, i32 xstride, i32 ystride, i32 pix0offset);
} FastOutputPCFace;

typedef struct FastOutputPCData_ {
    doeObjectData	mu;
    dcLLFiller		ll;
    f32			pathDispX, pathDispY;
    u8*			alpha8;
    u16*		alpha16;
    bool		alphaIs8;
    i32			xstride, ystride;
    i32			pix0offset;
    i32			spux0, spuy0; /* subpath start, OA-rel, U */
    bool		subpathIs1st;
} FastOutputPCData;

static char*
FastOutputPC_className(doeE env, doeObject o) { return "PathFiller_FastOutputPC"; }

static doeObject
FastOutputPC_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
FastOutputPC__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

static void
FastOutputPC__enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    BASE__enumCoObs(env, o, cb);
}

static void
FastOutputPC_setUpAlpha8 (doeE env, FastOutputPC pc,
		dcLLFiller ll, f32 pathdx, f32 pathdy,
		u8 * alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;

    p->ll = ll;
    p->pathDispX = pathdx;
    p->pathDispY = pathdy;
    p->alpha8 = alpha;
    p->alphaIs8 = TRUE;
    p->xstride = xstride;
    p->ystride = ystride;
    p->pix0offset = pix0offset;
}

static void
FastOutputPC_setUpAlpha16(doeE env, FastOutputPC pc,
		dcLLFiller ll, f32 pathdx, f32 pathdy,
		u16* alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;

    p->ll = ll;
    p->pathDispX = pathdx;
    p->pathDispY = pathdy;
    p->alpha16 = alpha;
    p->alphaIs8 = FALSE;
    p->xstride = xstride;
    p->ystride = ystride;
    p->pix0offset = pix0offset;
}

static void
FastOutputPC_beginPath(		doeE env, dcPathConsumer pc)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;

    p->subpathIs1st = TRUE;
}

/* coordinates are pixels, (not tss!) */
static void
FastOutputPC_beginSubpath(	doeE env, dcPathConsumer pc,
				f32 x0, f32 y0)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;
    dcLLFiller		ll = p->ll;

    x0 += p->pathDispX;
    y0 += p->pathDispY;
    if (!p->subpathIs1st) {	/* close */
	(*ll)->appendArc1(env, ll, p->spux0, p->spuy0);
    }
    p->spux0 = dcLLFiller_pix2U(x0);
    p->spuy0 = dcLLFiller_pix2U(y0);
    p->subpathIs1st = FALSE;
    (*ll)->beginSubpath(env, ll, p->spux0, p->spuy0);

}

static void
FastOutputPC_appendLine(	doeE env, dcPathConsumer pc,
				f32 x1, f32 y1)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;
    dcLLFiller		ll = p->ll;
    i32			ux1, uy1;

    x1 += p->pathDispX;
    y1 += p->pathDispY;
    ux1 = dcLLFiller_pix2U(x1);
    uy1 = dcLLFiller_pix2U(y1);
    (*ll)->appendArc1(env, ll, ux1, uy1);
}

static void
FastOutputPC_appendQuadratic(	doeE env, dcPathConsumer pc,
				f32 x1, f32 y1, f32 x2, f32 y2)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;
    dcLLFiller		ll = p->ll;
    i32			ux1, uy1, ux2, uy2;

    x1 += p->pathDispX;
    y1 += p->pathDispY;
    x2 += p->pathDispX;
    y2 += p->pathDispY;
    ux1 = dcLLFiller_pix2U(x1);
    uy1 = dcLLFiller_pix2U(y1);
    ux2 = dcLLFiller_pix2U(x2);
    uy2 = dcLLFiller_pix2U(y2);
    (*ll)->appendArc2(env, ll, ux1, uy1, ux2, uy2);
}

static void
FastOutputPC_appendCubic(	doeE env, dcPathConsumer pc,
				f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;
    dcLLFiller		ll = p->ll;
    i32			ux1, uy1, ux2, uy2, ux3, uy3;

    x1 += p->pathDispX;
    y1 += p->pathDispY;
    x2 += p->pathDispX;
    y2 += p->pathDispY;
    x3 += p->pathDispX;
    y3 += p->pathDispY;
    ux1 = dcLLFiller_pix2U(x1);
    uy1 = dcLLFiller_pix2U(y1);
    ux2 = dcLLFiller_pix2U(x2);
    uy2 = dcLLFiller_pix2U(y2);
    ux3 = dcLLFiller_pix2U(x3);
    uy3 = dcLLFiller_pix2U(y3);
    (*ll)->appendArc3(env, ll, ux1, uy1, ux2, uy2, ux3, uy3);
}

static void
FastOutputPC_closedSubpath(	doeE env, dcPathConsumer pc) {}

static void
FastOutputPC_endPath(		doeE env, dcPathConsumer pc)
{
    FastOutputPCData*	p = (FastOutputPCData*)pc;
    dcLLFiller		ll = p->ll;

    (*ll)->appendArc1(env, ll, p->spux0, p->spuy0);
    if (p->alphaIs8) {
	(*ll)->writeAlpha8 (env, ll, p->alpha8, p->xstride, p->ystride, p->pix0offset);
    } else {
	(*ll)->writeAlpha16(env, ll, p->alpha16, p->xstride, p->ystride, p->pix0offset);
    }
}

static void
FastOutputPC_useProxy(		doeE env, dcPathConsumer pc,
				dcFastPathProducer proxy)
{
    /* impossible */
}

static FastOutputPCFace FastOutputPCClass = {
    {
	{
	    sizeof(FastOutputPCData),
	    
	    FastOutputPC_className,
	    FastOutputPC_copy,
	    FastOutputPC__cleanup,
	    FastOutputPC__enumCoObs,
	    doeObject_uproot
	},
	    
	FastOutputPC_beginPath,
	FastOutputPC_beginSubpath,
	FastOutputPC_appendLine,
	FastOutputPC_appendQuadratic,
	FastOutputPC_appendCubic,
	FastOutputPC_closedSubpath,
	FastOutputPC_endPath,
	FastOutputPC_useProxy
    },

    FastOutputPC_setUpAlpha8,
    FastOutputPC_setUpAlpha16
};

static void
FastOutputPC_init(doeE env, dcPathConsumer target)
{
    FastOutputPCData*	p = (FastOutputPCData*)target;

    BASE_init(env, target);
    if (doeError_occurred(env)) {
	BASE__cleanup(env, target);
	return;
    }

    *target = (dcPathConsumerFace*)&FastOutputPCClass;
}

static dcPathConsumer
FastOutputPC_create(doeE env)
{
    dcPathConsumer	p = doeMem_malloc(env, (i32)sizeof(FastOutputPCData));

    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }

    FastOutputPC_init(env, p);
    if (doeError_occurred(env)) {
	doeMem_free(env, p);
	p = NULL;
    }

    return p;
}

static void
FastOutputPC_staticInitialize(doeE env) {}
static void
FastOutputPC_staticFinalize(doeE env) {}

#undef BASE


/*- PathFiller methods ----------------------------*/

#define BASE dcPathStore

static void
setFillMode(doeE env, dcPathFiller pf, ixx fillmode)
{
    dcPathFillerData* p = (dcPathFillerData*)pf;

    if (stateCheck(p, setFillModeDone)) {
	doeError_set(env, dcPRError, dcPRError_UNEX_setUsage);
	return;
    }

    if (fillmode != dcPathFiller_NZFILL && fillmode != dcPathFiller_EOFILL) {
	doeError_set(env, dcPRError, dcPRError_UNK_fillmode);
	return;
    }
    p->redundantReset = FALSE;
    p->fillmode = fillmode;
    stateSet(p, setFillModeDone);
}

/* PathConsumer methods inherited from PathStore, with the exception
   of endPath and useProxy */
static void
endPath(doeE env, dcPathConsumer pc)
{
    dcPathFillerData*	p = (dcPathFillerData*)pc;
    dcFastPathProducer	fpp = p->thisFPP;

    ((dcPathConsumerFace*)&dcPathStoreClass)->endPath(env, pc);
    if (doeError_occurred(env))	return;

    (*fpp)->getBox(env, fpp, p->pathBox);
    if (doeError_occurred(env))	return;

#ifdef DEBUG
   /* 
    * Set the exception to indicate that some segments of the path have
    * way too large coordinates. The exception will be eaten at java level.
    * For more info see bug 4485298.
    */
    if (!(	dcPathFiller_validLoCoord(p->pathBox[0])	&&
		dcPathFiller_validLoCoord(p->pathBox[1])	&&
		dcPathFiller_validHiCoord(p->pathBox[2])	&&
		dcPathFiller_validHiCoord(p->pathBox[3]))) {
	doeError_set(env, dcPathException, dcPathException_BAD_PATH_endPath);
    }
#endif

}

static void
useProxy(doeE env, dcPathConsumer pc, dcFastPathProducer proxy)
{
    dcPathFillerData*	p = (dcPathFillerData*)pc;
    dcFastPathProducer	fpp = p->thisFPP;

    ((dcPathConsumerFace*)&dcPathStoreClass)->useProxy(env, pc, proxy);
    if (doeError_occurred(env))	return;

    (*proxy)->getBox(env, proxy, p->pathBox);
    if (doeError_occurred(env))	return;

#ifdef DEBUG
   /*
    * Set the exception to indicate that some segments of the path have
    * way too large coordinates. The exception will be eaten at java level.
    * See 4485298 for more info.
    */
    if (!(	dcPathFiller_validLoCoord(p->pathBox[0])	&&
		dcPathFiller_validLoCoord(p->pathBox[1])	&&
		dcPathFiller_validHiCoord(p->pathBox[2])	&&
		dcPathFiller_validHiCoord(p->pathBox[3]))) {
	doeError_set(env, dcPathException, dcPathException_BAD_PATH_useProxy);
    }
#endif

 }

static void
getAlphaBox(doeE env, dcPathFiller pf, i32* box)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;

    if (!((dcPathStoreData*)p)->pathDefined) {
	doeError_set(env, dcPRError, dcPRError_UNEX_getAlphaBox);
	return;
    }

    box[0] = (i32)floor(p->pathBox[0]);
    box[1] = (i32)floor(p->pathBox[1]);
    box[2] = (i32)ceil (p->pathBox[2]);
    box[3] = (i32)ceil (p->pathBox[3]);
}

static void
setOutputArea(doeE env, dcPathFiller pf, f32 outlox, f32 outloy, i32 w, i32 h)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;
    dcFastPathProducer	fpp = p->thisFPP;
    i32			xi, yi;

    /* check state, parameters */
    if (!((dcPathStoreData*)p)->pathDefined) {
	doeError_set(env, dcPRError, dcPRError_UNEX_setOutputArea);
	return;
    }
    if (w <= 0 || h <= 0) {
	doeError_set(env, dcPRError, dcPRError_BAD_outputarea);
	return;
    }
    if (!(	dcPathFiller_validLoCoord(outlox)	&&
		dcPathFiller_validLoCoord(outloy)	&&
		dcPathFiller_validHiCoord(outlox + w)	&&
		dcPathFiller_validHiCoord(outloy + h))) {
	doeError_set(env, dcPRException, dcPRException_BAD_COORD_setOutputArea);
	return;
    }

    stateSet(p, setOutputAreaDone);

    p->outLoX = outlox;
    p->outLoY = outloy;
    p->outW   = w;
    p->outH   = h;
    p->outWTiF = w / dcPathFiller_tileSizeF;
    p->outHTiF = h / dcPathFiller_tileSizeF;

    if (p->tileRuns != NULL)
	doeMem_free(env, p->tileRuns);

    p->fastOutput =	w <= dcPathFiller_tileSize	&&
			h <= dcPathFiller_tileSize	&&
			p->pathBox[0] >= outlox		&&
			p->pathBox[1] >= outloy		&&
			p->pathBox[2] <= outlox + w	&&
			p->pathBox[3] <= outloy + h;

    if (p->fastOutput)
	return;

    /* allocate [tileRuns], indexed [xindex][yindex];
       (here we reuse [w] and [h] as width and height in tiles) */
    p->outWTi = w = (p->outW + dcPathFiller_tileSize - 1) >> dcPathFiller_tileSizeL2S;
    p->outHTi = h = (p->outH + dcPathFiller_tileSize - 1) >> dcPathFiller_tileSizeL2S;
    
    p->tileRuns = doeMem_malloc(env, 
			  sizeof(Run*)*(w + 1)	/* array  of (Run*), indexed by X */
			+ sizeof(Run )*(w + 1)*h/* arrays of (Run),  indexed by Y */
		  );
    if (p->tileRuns == NULL) {
	doeError_setNoMemory(env);
	return;
    }
    for (xi = 0; xi <= w; xi++) {
	p->tileRuns[xi] = (Run*)(p->tileRuns + w + 1) + xi * h;
    }
    for (yi = 0; yi < h; yi++) {
	for (xi = 0; xi <= w; xi++) {
	    p->tileRuns[xi][yi] = NULL;
	}
    }

    (*fpp)->sendTo(env, fpp, p->runsBuilder);
    if (doeError_occurred(env)) {
	doeError_setNoMemory(env);
	return;
    }

    /* set things so [nextTile] will advance to the 1st tile */
    p->tileXI = w;
    p->tileYI = -1;
    (*pf)->nextTile(env, pf);
}
				
static void
nextTile(doeE env, dcPathFiller pf)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;

    if (!stateCheck(p, setOutputAreaDone)) {
	doeError_set(env, dcPRError, dcPRError_UNEX_nextTile);
	return;
    }

    if (p->fastOutput) {
	stateReset(p, setOutputAreaDone);
	return;
    }

    /* advance x tile index */
    p->tileXI++;
    if (p->tileXI > p->outWTi) { /* tileXI in [1, outWITi] */
	/* clear left side effects */
	LeftSide_releaseList(env, p->lsEffects);
	p->lsEffects = NULL;

	p->tileXI = 1; /* 0 is left-open tile */
	p->tileYI++;

	if (p->tileYI >= p->outHTi) {
	    stateReset(p, setOutputAreaDone);
	    return;
	}

	p->rowH = p->outH - (p->tileYI << dcPathFiller_tileSizeL2S);
	if (p->rowH > dcPathFiller_tileSize)
	    p->rowH = dcPathFiller_tileSize;
	p->rowHTiF = (f32)p->rowH / dcPathFiller_tileSizeF;
    }

    /* update left side effects */
    {   Run r;

	for (r = p->tileRuns[p->tileXI - 1][p->tileYI]; r != NULL; r = r->next) {
	    f32		rspy0 = r->rspy0;
	    f32		rspy1 = r->rspy1;
	    LeftSide	ls, lslink, tmp;

	    /* no rightside projection? */
	    if (rspy1 == rspyImpossibleTss) {
		continue;
	    }

	    /* constrain [rsp] to the interval [0,rowHTiF] */
	    if (rspy0 < 0.0F)		rspy0 = 0.0F;
	    if (rspy1 < 0.0F)		rspy1 = 0.0F;
	    if (rspy0 > p->rowHTiF)	rspy0 = p->rowHTiF;
	    if (rspy1 > p->rowHTiF)	rspy1 = p->rowHTiF;

	    /* for each [ls] in [lsEffects] check whether [rsp]
	       extends it; if so, modify [rsp] accordingly and remove
	       [ls] from [lsEffects] */
	    ls =	p->lsEffects;
	    lslink =	NULL;
	    while (ls != NULL) {
		if (rspy1 == ls->y0 || rspy0 == ls->y1) {
		    if (rspy1 == ls->y0)	rspy1 = ls->y1;
		    else			rspy0 = ls->y0;
		    if (lslink == NULL)
			p->lsEffects = ls->next;
		    else
			lslink->next = ls->next;

		    tmp = ls->next;
		    ls->next = NULL;
		    LeftSide_releaseList(env, ls);
		    ls = tmp;
		} else {
		    lslink = ls;
		    ls = ls->next;
		}
	    }

	    /* if [rsp] still produces an effect, insert it in [lsEffect] */
	    if (rspy0 != rspy1) {
		ls = LeftSide_create(env, p->poolLeftSide); if (ls == NULL) return;
		ls->y0 = rspy0;
		ls->y1 = rspy1;
		ls->next = p->lsEffects;
		p->lsEffects = ls;
	    }
	}
    }
}

static ixx
getTileState(doeE env, dcPathFiller pf)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;
    Run			r;
    ixx			lscount;
    LeftSide		ls;

    if (!stateCheck(p, setOutputAreaDone)) {
	doeError_set(env, dcPRError, dcPRError_UNEX_getTileState);
	return -1;
    }

    /* "fast output" situations are never trivial */
    if (p->fastOutput) {
	return dcPathFiller_TILE_IS_GENERAL;
    }

    /* two conditions must be met for a tile to be trivial:
	1. the tile itself must contain no runs
	2. the list lsEffects must consist exclusively of
	   "full height" left sides
     */
    r = p->tileRuns[p->tileXI][p->tileYI];
    if (r != NULL) {
	return dcPathFiller_TILE_IS_GENERAL;
    }

    lscount = 0;
    for (ls = p->lsEffects; ls != NULL; ls = ls->next) {
	if (ls->y0 == 0.0F && ls->y1 == p->rowHTiF) {
	    lscount++;
	    continue;
	}
	if (ls->y1 == 0.0F && ls->y0 == p->rowHTiF) {
	    lscount--;
	    continue;
	}
	return dcPathFiller_TILE_IS_GENERAL;
    }

    if (p->fillmode == dcPathFiller_EOFILL)
	lscount &= 1;

    return (lscount != 0)?
		dcPathFiller_TILE_IS_ALL_1 :
		dcPathFiller_TILE_IS_ALL_0;
}

/* an auxiliary method */
static void
sendTileToLLFiller(doeE env, dcPathFiller pf, dcLLFiller ll)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;
    LeftSide		ls;
    Run			r;

    /* send lsEffects to [ll] */
    ls = p->lsEffects;
    while (ls != NULL) {
	(*ll)->processLeftRun(env, ll, dcLLFiller_tss2U(ls->y0), dcLLFiller_tss2U(ls->y1));
	ls = ls->next;
    }

    /* send the arcs in the runs of the tile */
    r = p->tileRuns[p->tileXI][p->tileYI];
    while (r != NULL) {
	i16* arcs = r->runarcs;
	ixx  i, l;
	(*ll)->beginSubpath(env, ll, arcs[0], arcs[1]);
	i = 2;
	l = r->ra1stavail;
	while (i < l) {
	    i16 arctype = arcs[i++];
	    i16 x1, y1, x2, y2, x3, y3;
	    if        (arctype == Run_TYPE_Arc1) {
		x1 = arcs[i++];
		y1 = arcs[i++];
		(*ll)->appendArc1(env, ll, x1, y1);
	    } else if (arctype == Run_TYPE_Arc2) {
		x1 = arcs[i++];
		y1 = arcs[i++];
		x2 = arcs[i++];
		y2 = arcs[i++];
		(*ll)->appendArc2(env, ll, x1, y1, x2, y2);
	    } else {
		x1 = arcs[i++];
		y1 = arcs[i++];
		x2 = arcs[i++];
		y2 = arcs[i++];
		x3 = arcs[i++];
		y3 = arcs[i++];
		(*ll)->appendArc3(env, ll, x1, y1, x2, y2, x3, y3);
	    }
	}

	r = r->next;
    }
}

static void
writeAlpha8 (	doeE env, dcPathFiller pf,
		u8 * alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;
    dcLLFiller		ll;

    if (!stateCheck(p, setOutputAreaDone)) {
	doeError_set(env, dcPRError, dcPRError_UNEX_writeAlpha);
	return;
    }
    if (alpha == NULL || xstride <= 0 || ystride <= 0 || pix0offset < 0) {
	doeError_set(env, dcPRError, dcPRError_BAD_alphadest);
	return;
    }

    ll = dcLLFiller_get(env);
    if (doeError_occurred(env))
	return;

    if (p->fastOutput) {
	FastOutputPC		fopc = p->fastOutputPC;
	dcFastPathProducer	fpp  = p->thisFPP;

	(*ll)->setParams(env, ll, p->fillmode, p->outW, p->outH);
	(*fopc)->setUpAlpha8 (	env, fopc, ll,
				-p->outLoX, -p->outLoY,
				alpha, xstride, ystride, pix0offset);
	(*fpp)->sendTo(env, fpp, (dcPathConsumer)fopc);
    } else {
	i32 tilew = MIN(p->outW -  ((p->tileXI - 1) << dcPathFiller_tileSizeL2S),
			dcPathFiller_tileSize);
	(*ll)->setParams(env, ll, p->fillmode, tilew, p->rowH);
	sendTileToLLFiller(env, pf, ll);
	(*ll)->writeAlpha8 (env, ll, alpha, xstride, ystride, pix0offset);
    }
    dcLLFiller_release(env, ll);

    nextTile(env, pf);
}

static void
writeAlpha16(	doeE env, dcPathFiller pf,
		u16* alpha, i32 xstride, i32 ystride, i32 pix0offset)
{
    dcPathFillerData*	p = (dcPathFillerData*)pf;
    dcLLFiller		ll;

    if (!stateCheck(p, setOutputAreaDone)) {
	doeError_set(env, dcPRError, dcPRError_UNEX_writeAlpha);
	return;
    }
    if (alpha == NULL || xstride <= 0 || ystride <= 0 || pix0offset < 0) {
	doeError_set(env, dcPRError, dcPRError_BAD_alphadest);
	return;
    }

    ll = dcLLFiller_get(env);
    if (doeError_occurred(env))
	return;

    if (p->fastOutput) {
	FastOutputPC		fopc = p->fastOutputPC;
	dcFastPathProducer	fpp  = p->thisFPP;

	(*ll)->setParams(env, ll, p->fillmode, p->outW, p->outH);
	(*fopc)->setUpAlpha16(	env, fopc, ll,
				-p->outLoX, -p->outLoY,
				alpha, xstride, ystride, pix0offset);
	(*fpp)->sendTo(env, fpp, (dcPathConsumer)fopc);
    } else {
	i32 tilew = MIN(p->outW -  ((p->tileXI - 1) << dcPathFiller_tileSizeL2S),
			dcPathFiller_tileSize);
	(*ll)->setParams(env, ll, p->fillmode, tilew, p->rowH);
	sendTileToLLFiller(env, pf, ll);
	(*ll)->writeAlpha16(env, ll, alpha, xstride, ystride, pix0offset);
    }
    dcLLFiller_release(env, ll);

    nextTile(env,pf);
}

static void
reset(doeE env, dcPathFiller pf)
{
    dcPathFillerData*	p  = (dcPathFillerData*)pf;
    dcPathStore		ps = (dcPathStore) pf;
    i32			tilex, tiley;

    if (p->redundantReset)	return;
    p->redundantReset = TRUE;

    (*ps)->reset(env, ps);	/* reset the PathStore */
    stateResetAll(p);
    LeftSide_releaseList(env, p->lsEffects);
    p->lsEffects = NULL;
    if (p->tileRuns != NULL) {
	for (tiley = 0; tiley < p->outHTi; tiley++) {
	    for (tilex = 0; tilex <= p->outWTi; tilex++) {
		Run_releaseList(env, p->tileRuns[tilex][tiley]);
	    }
	}
	doeMem_free(env, p->tileRuns);
	p->tileRuns = NULL;
    }

    dcPool_endCycle(env, p->poolRun);
    dcPool_endCycle(env, p->poolLeftSide);
}


static dcPathFillerFace dcPathFillerClass = {
    {
	{
	    {
		sizeof(dcPathFillerData),
		
		className,
		copy,
		_cleanup,
		_enumCoObs,
		doeObject_uproot
	    },
	    
	    dcPathStore_beginPath,
	    dcPathStore_beginSubpath,
	    dcPathStore_appendLine,
	    dcPathStore_appendQuadratic,
	    dcPathStore_appendCubic,
	    dcPathStore_closedSubpath,
	    endPath,
	    useProxy
	},

	dcPathStore_getFastPathProducer,
	dcPathStore_reset
    },

    setFillMode,
    getAlphaBox,
    setOutputArea,

    getTileState,
    writeAlpha8,
    writeAlpha16,

    nextTile,
    reset
};

void
dcPathFiller_init(doeE env, dcPathFiller pf)
{
    dcPathFillerData*	p  = (dcPathFillerData*)pf;
    dcPathStore		ps = (dcPathStore) pf;

    p->redundantReset = FALSE;

    p->tileRuns = NULL;
    p->lsEffects = NULL;
    p->runsBuilder = NULL;
    p->fastOutputPC = NULL;

    BASE_init(env, pf);

    *pf = &dcPathFillerClass;

    p->poolRun =	dcPool_create(env,	"Run's pool",
						sizeof(RunData),	0, 1.0);
    p->poolLeftSide =	dcPool_create(env,	"LeftSide's pool",
						sizeof(LeftSideData),	0, 1.0);
    p->xtsize = 40;
    p->xt = doeMem_malloc(env, sizeof(f32)*p->xtsize);
    p->ytsize = 40;
    p->yt = doeMem_malloc(env, sizeof(f32)*p->ytsize);
    if (p->xt == NULL || p->yt == NULL) {
	doeError_setNoMemory(env);
    }
    p->thisFPP = (*ps)->getFastPathProducer(env, ps);
    p->runsBuilder = RunsBuilder_create(env, p);
    p->fastOutputPC = (FastOutputPC)FastOutputPC_create(env);

    if (doeError_occurred(env))	return;

    reset(env, pf);
}

dcPathFiller
dcPathFiller_create(doeE env)
{
    dcPathFiller p = (dcPathFiller)doeMem_malloc(env, (i32)sizeof(dcPathFillerData));

    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }

    dcPathFiller_init(env, p);
    if (doeError_occurred(env)) {
	_cleanup(env, (doeObject)p);
	doeMem_free(env, p);
	p = NULL;
    }

    return p;
}

static ixx clients = 0;

void
dcPathFiller_staticInitialize(doeE env)
{
    if (clients++ > 0) return;

    dcPool_staticInitialize(env);
    dcPathStore_staticInitialize(env);
    dcLLFiller_staticInitialize(env);
    FastOutputPC_staticInitialize(env);

    if (doeError_occurred(env)) {
	doeError_setNoMemory(env);
	return;
    }

    dcPathFiller_tileSizeL2S = dcLLFiller_tileSizeL2S;
    dcPathFiller_tileSize    = 1 << dcPathFiller_tileSizeL2S;
    dcPathFiller_tileSizeF   = (f32)dcPathFiller_tileSize;

    runCheckCost =	 77.0F;

    KArc2 =		158.0F;
    LArc2 =		runCheckCost + 46.0F;
    MArc2 =		(f32)dcLLFiller_ticsSetupArc2;
    NArc2 =		(f32)dcLLFiller_ticsStepArc2;
    DIV2Arc2 =		2.00 *	(2 * KArc2 + LArc2 + MArc2) /
				(2 * NArc2 * dcPathFiller_tileSizeF);
    DIV4Arc2 =		0.67 *	(2 * KArc2 + .5F * LArc2 + .5F * MArc2) /
				(2 * NArc2 * dcPathFiller_tileSizeF);

    KArc3 =		206.0F;
    LArc3 =		runCheckCost + 67.0F;
    MArc3 =		(f32)dcLLFiller_ticsSetupArc3;
    NArc3 =		(f32)dcLLFiller_ticsStepArc3;
    DIV2Arc3 =		2.00 *	(2 * KArc3 + LArc3 + MArc3) /
				(2 * NArc3 * dcPathFiller_tileSizeF);
    DIV4Arc3 =		0.67 *	(2 * KArc3 + .5F * LArc3 + .5F * MArc3) /
				(2 * NArc3 * dcPathFiller_tileSizeF);
  }

void
dcPathFiller_staticFinalize(doeE env)
{
    if (--clients > 0) return;

    dcPool_staticFinalize(env);
    dcPathStore_staticFinalize(env);
    dcLLFiller_staticFinalize(env);
    FastOutputPC_staticFinalize(env);
}

