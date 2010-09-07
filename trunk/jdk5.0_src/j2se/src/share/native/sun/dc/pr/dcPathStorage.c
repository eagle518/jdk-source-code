/*
 * @(#)dcPathStorage.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStorage.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcPRError.h"
#include "dcPathError.h"

#include "dcPathConsumer-p.h"
#include "dcPathStorage.h"


/*----------------------
 * The data of its instances
 */
typedef struct dcPathStorageData_ {
    dcPathConsumerData  paco;
    /* an "eternal" property of the storage object */
    ixx       tangentInfo;  /* does it contain tangent info? */
    /* state of the interaction with the path feeder */
    ixx       frozen;
    ixx       inPath;
    ixx       inSubpath;
    ixx       expectingTangentInfo;
    /* the stored path */
    ixx       isClosed;     /* is the current subpath open/closed? */
    u8*       arcs;         /* types of the arcs */
    ixx       arcCnt;       /* number of arcs stored */
    ixx       maxArcs;      /* how many arcs can be stored currently */
    f32*      pnts;         /* points -- laid out (x,y)(x,y)... */
    ixx       pntCnt;       /* number of points stored (each point is 2 coordinates) */
    ixx       maxPnts;      /* how many points can be stored currently */
    i32*      tans;         /* tangent info: 1 per line, 2 for other arcs */
    ixx       tanCnt;       /* number of tangents stored */
    ixx       maxTans;      /* how many tangents can be stored currently */
} dcPathStorageData;



/*----------------------
 * The real work
 */

static void*
reallocate(doeE env, void* where, ixx newsize) { 
    void* tmp = (void*)doeMem_realloc(env, where, newsize);
    if (tmp == NULL) {
        doeError_setNoMemory(env);
	return NULL;
    }
    return tmp;
}

static void
clearAndResizePath(doeE env, dcPathStorageData* st, ixx arccnt, ixx pntcnt,
						     ixx tancnt) {
    st->arcs = (u8*)reallocate(env, (void*)(st->arcs), arccnt * sizeof(u8));
    if (doeError_occurred(env)) return;
    st->maxArcs = arccnt; st->arcCnt = 0;

    st->pnts = (f32*)reallocate(env, (void*)(st->pnts), pntcnt * sizeof(f32));
    if (doeError_occurred(env)) return;
    st->maxPnts = pntcnt; st->pntCnt = 0;

    if (st->tangentInfo) {
        st->tans = (i32*)reallocate(env, (void*)(st->tans), tancnt * sizeof(i32));
	/* if (doeError_occurred(env)) return;	last one, could skip */
        st->maxTans = tancnt; st->tanCnt = 0;
    }
}

#define INITIAL_MAX_ARCS      32
#define INITIAL_MAX_PNTS      128
#define INITIAL_MAX_TANS      64

#define newStorage(a,b)       (((a)<(b))?((a)+(a)):((a)+(b)))

#define LIMIT_MAX_ARCS        128
#define LIMIT_MAX_PNTS        512
#define LIMIT_MAX_TANS        256

static void
guaranteeStorage(doeE env, dcPathStorageData* st, ixx a, ixx p, ixx t)
{
    if (st->maxArcs < (st->arcCnt + a)) {
        ixx    newmaxarcs = newStorage(st->maxArcs, LIMIT_MAX_ARCS);
        st->arcs = (u8*)reallocate(env, (void*)(st->arcs), newmaxarcs*sizeof(u8));
	if (doeError_occurred(env)) return;
        st->maxArcs = newmaxarcs;
    }
    if (st->maxPnts < (st->pntCnt + p)) {
        ixx    newmaxpnts = newStorage(st->maxPnts, LIMIT_MAX_PNTS);
        st->pnts = (f32*)reallocate(env, (void*)(st->pnts),
					  newmaxpnts * sizeof(f32));
	if (doeError_occurred(env)) return;
        st->maxPnts = newmaxpnts;
    }
    if (st->tangentInfo) {
        if (st->maxTans < (st->tanCnt + t)) {
            ixx    newmaxtans = newStorage(st->maxTans, LIMIT_MAX_TANS);
            st->tans = (i32*)reallocate(env, (void*)(st->tans),
					     newmaxtans * sizeof(i32));
	    /* if (doeError_occurred(env)) return;	last one, could skip */
            st->maxTans = newmaxtans;
        }
    }
}


/*----------------------
 * The overriden doeObject methods
 */

static char*
className(doeE env, doeObject o)
{
    return "dcPathStorage";
}

static void    dcPathStorage_copyinit(doeE, dcPathStorage, dcPathStorage src);

static doeObject
copy(doeE env, doeObject o)
{
    dcPathStorageData* st =
        (dcPathStorageData*)doeMem_malloc(env, sizeof(dcPathStorageData));
    if (st == NULL) {
        doeError_setNoMemory(env);
	return NULL;
    }
    dcPathStorage_copyinit(env, (dcPathStorage)st, (dcPathStorage)o);
    return  (doeObject)st;
}

static void
_cleanup(doeE env, doeObject o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    doeMem_free(env, (void*)st->arcs); st->arcs = NULL;
    doeMem_free(env, (void*)st->pnts); st->pnts = NULL;
    if (st->tangentInfo) {
        doeMem_free(env, (void*)st->tans);
	st->tans = NULL;
    }
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
}


/*----------------------
 * The implementation of the dcPathConsumer interface
 */

static void
endOfSubpath(doeE env, dcPathStorageData* st)
{
    guaranteeStorage(env, st, 1, 0, 0);
    if (doeError_occurred(env)) return;
    if (st->isClosed) {
        st->arcs[st->arcCnt++] = dcPathStorageEND_CLOSED_SUBPATH;
    } else {
        st->arcs[st->arcCnt++] = dcPathStorageEND_OPEN_SUBPATH;
    }
}

static void
beginPath(doeE env, dcPathConsumer o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    st->inPath = 1;

    guaranteeStorage(env, st, 1, 0, 0);
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageBEGIN_PATH_NO_BOX;
}

static void
beginSubpath(doeE env, dcPathConsumer o, f32 x0, f32 y0)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    f32*               dst;
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
    } else {
        st->inSubpath = 1;
    }
    guaranteeStorage(env, st, 1, 2, 0);
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageBEGIN_SUBPATH;
    dst = st->pnts + st->pntCnt;
    *dst++ = x0; *dst = y0; 
    st->pntCnt += 2;
    st->isClosed = 0;
}

static void
appendLine(doeE env, dcPathConsumer o, f32 x1, f32 y1)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    f32*               dst;
    if (st->tangentInfo) {
        guaranteeStorage(env, st, 1, 2, 1);
        st->expectingTangentInfo = 1;
    } else {
        guaranteeStorage(env, st, 1, 2, 0);
    }
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageLINE;
    dst = st->pnts + st->pntCnt;
    *dst++ = x1; *dst = y1; 
    st->pntCnt += 2;
}

static void
appendQuadratic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    f32*               dst;
    if (st->tangentInfo) {
        guaranteeStorage(env, st, 1, 4, 2);
        st->expectingTangentInfo = 2;
    } else {
        guaranteeStorage(env, st, 1, 4, 0);
    }
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageQUADRATIC;
    dst = st->pnts + st->pntCnt;
    *dst++ = x1; *dst++ = y1; 
    *dst++ = x2; *dst = y2; 
    st->pntCnt += 4;
}

static void
appendCubic(doeE env, dcPathConsumer o, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3) {
    dcPathStorageData* st = (dcPathStorageData*)o;
    f32*               dst;
    if (st->tangentInfo) {
        guaranteeStorage(env, st, 1, 6, 2);
        st->expectingTangentInfo = 2;
    } else {
        guaranteeStorage(env, st, 1, 6, 0);
    }
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageCUBIC;
    dst = st->pnts + st->pntCnt;
    *dst++ = x1; *dst++ = y1; 
    *dst++ = x2; *dst++ = y2; 
    *dst++ = x3; *dst = y3; 
    st->pntCnt += 6;
}

static void
closedSubpath(doeE env, dcPathConsumer o) {
    dcPathStorageData* st = (dcPathStorageData*)o;
    st->isClosed = 1;
}

static void
endPath(doeE env, dcPathConsumer o) {
    dcPathStorageData* st = (dcPathStorageData*)o;
    if (st->inSubpath) {
        endOfSubpath(env, st);
	if (doeError_occurred(env)) return;
    }
    guaranteeStorage(env, st, 1, 0, 0);
    if (doeError_occurred(env)) return;
    st->arcs[st->arcCnt++] = dcPathStorageEND_OF_PATH;
    st->inSubpath = 0;
    st->inPath = 0;
    st->frozen = 1;
}

static void
useProxy(doeE env, dcPathConsumer o, dcFastPathProducer fpp)
{
    dcPathStorageData*  st = (dcPathStorageData*)o;
    if (st->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_useProxy);
	return;
    }
}


/*----------------------
 * The class-specific methods
 */
static void
appendTangent(doeE env, dcPathStorage o, i32 tan)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    st->tans[st->tanCnt++] = tan;
    st->expectingTangentInfo = 0;
}

static void
appendTangents(doeE env, dcPathStorage o, i32 tan1, i32 tan2)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    st->tans[st->tanCnt++] = tan1;
    st->tans[st->tanCnt++] = tan2;
    st->expectingTangentInfo = 0;
}

static void
sendToConsumer(doeE env, dcPathStorage o, dcPathConsumer where, ixx mask)
{
    dcPathStorageData* st  = (dcPathStorageData*)o;
    u8*                typ = st->arcs;
    f32*               pnt = st->pnts;
    i32*               tan = st->tans;
    while (*typ != dcPathStorageEND_OF_PATH) {
        if (*typ == dcPathStorageBEGIN_PATH_NO_BOX) {
            if (mask & dcPathStoragePATH_MASK) {
                (*where)->beginPath(env, where);
		if (doeError_occurred(env)) return;
	    }
        } else if (*typ == dcPathStorageBEGIN_PATH_WITH_BOX) {
            if (mask & dcPathStoragePATH_MASK) {
                (*where)->beginPath(env, where);
		if (doeError_occurred(env)) return;
	    }
            pnt += 4;
        } else if (*typ == dcPathStorageBEGIN_SUBPATH) {
            if (mask & dcPathStorageSUBPATH_MASK) {
                (*where)->beginSubpath(env, where, *pnt, *(pnt+1));
		if (doeError_occurred(env)) return;
	    }
            pnt += 2;
        } else if (*typ == dcPathStorageLINE) { 
            if (mask & dcPathStorageARC_MASK) {
                (*where)->appendLine(env, where, *pnt, *(pnt+1));
		if (doeError_occurred(env)) return;
	    }
            pnt += 2; tan += 1; 
        } else if (*typ == dcPathStorageQUADRATIC) { 
            if (mask & dcPathStorageARC_MASK) {
                (*where)->appendQuadratic(env, where, *pnt,     *(pnt+1),
						      *(pnt+2), *(pnt+3));
		if (doeError_occurred(env)) return;
	    }
            pnt += 4; tan += 2; 
        } else if (*typ == dcPathStorageCUBIC) { 
            if (mask & dcPathStorageARC_MASK) {
                (*where)->appendCubic(env, where, *pnt,     *(pnt+1),
						  *(pnt+2), *(pnt+3),
						  *(pnt+4), *(pnt+5));
		if (doeError_occurred(env)) return;
	    }
            pnt += 6; tan += 2; 
        } else if (*typ == dcPathStorageEND_CLOSED_SUBPATH) {
            if (mask & dcPathStorageCLOSED_MASK) {
                (*where)->closedSubpath(env, where);
		if (doeError_occurred(env)) return;
	    }
        } else if (*typ == dcPathStorageEND_OPEN_SUBPATH) {
        } else {
        }
        typ++;
    }
    if (mask & dcPathStoragePATH_MASK)
        (*where)->endPath(env, where);
}

static u8*
getElements(doeE env, dcPathStorage o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    return (st->arcs);
}

static f32*
getPoints(doeE env, dcPathStorage o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    return (st->pnts);
}

static i32*
getTangents(doeE env, dcPathStorage o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    return (st->tans);
}

static void
reset(doeE env, dcPathStorage o)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    st->frozen = 0;
    st->inPath = 0;
    st->inSubpath = 0;
    clearAndResizePath(env, st, INITIAL_MAX_ARCS,
				INITIAL_MAX_PNTS, INITIAL_MAX_TANS);
}

/*----------------------
 * The class variable
 */

dcPathStorageFace dcPathStorageClass = {    
    {
	{
	    sizeof(dcPathStorageData),
	    className,                     /* Object interface */
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

    sendToConsumer,                /* PathStorage interface */
    appendTangent,
    appendTangents,
    getElements,
    getPoints,
    getTangents,
    reset
};


/*----------------------
 * The class-related global functions
 */

static void
dcPathStorage_init(doeE env, dcPathStorage o, ixx withtangentinfo)
{
    dcPathStorageData* st = (dcPathStorageData*)o;
    dcPathConsumer_init(env, (dcPathConsumer)o);
    *o = &dcPathStorageClass;
    st->tangentInfo = withtangentinfo;
    st->frozen = 0;
    st->inPath = 0; st->inSubpath = 0;
    st->expectingTangentInfo = 0;
    st->isClosed = 0;
    st->arcs = NULL; 
    st->pnts = NULL; 
    st->tans = NULL;
    clearAndResizePath(env, st, INITIAL_MAX_ARCS, INITIAL_MAX_PNTS,
				INITIAL_MAX_TANS);
}

dcPathStorage
dcPathStorage_create(doeE env, ixx withtangentinfo)
{
    dcPathStorageData* st = 
        (dcPathStorageData*)doeMem_malloc(env, sizeof(dcPathStorageData));
    if (st == NULL) {
        doeError_setNoMemory(env);
	return NULL;
    }
    dcPathStorage_init(env, (dcPathStorage)st, withtangentinfo);
    return (dcPathStorage)st;
}


void
dcPathStorage_copyinit(doeE env, dcPathStorage o, dcPathStorage src)
{
    dcPathStorageData* st    = (dcPathStorageData*)o;
    dcPathStorageData* srcst = (dcPathStorageData*)src;

    dcPathConsumer_copyinit(env, (dcPathConsumer)o, (dcPathConsumer)src);
    st->tangentInfo = srcst->tangentInfo;
    st->frozen = srcst->frozen;
    st->inPath = srcst->inPath;
    st->inSubpath = srcst->inSubpath;
    st->expectingTangentInfo = srcst->expectingTangentInfo;
    st->isClosed = srcst->isClosed;
    st->arcs = NULL; 
    st->pnts = NULL; 
    st->tans = NULL;
    clearAndResizePath(env, st, srcst->maxArcs, srcst->maxPnts, srcst->maxTans);
    if (doeError_occurred(env)) return;
    {
        u8* src = srcst->arcs;
        u8* dst = st->arcs;
        ixx cnt = srcst->arcCnt;
        while( cnt-- ) *dst++ = *src++;
        st->arcCnt = srcst->arcCnt;
    }
    {
        f32* src = srcst->pnts;
        f32* dst = st->pnts;
        ixx  cnt = srcst->pntCnt;
        while( cnt-- ) *dst++ = *src++;
        st->pntCnt = srcst->pntCnt;
    }
    {
        i32* src = srcst->tans;
        i32* dst = st->tans;
        ixx  cnt = srcst->tanCnt;
        while( cnt-- ) *dst++ = *src++;
        st->tanCnt = srcst->tanCnt;
    }
}

void dcPathStorage_staticInitialize(doeE env) { }
void dcPathStorage_staticFinalize  (doeE env) { }
