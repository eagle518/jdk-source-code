/*
 * @(#)dcPathStore.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStore.c 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcPathError.h"
#include "dcPathStore-p.h"

/*- internal static classes Item - a common base class -
    beginSubpath, appendLine, appendQuadratic, appendCubic
    and closedSubpath - all extending Item ----------------*/

#define BASE doeObject
typedef struct dcPathStoreItemFace_ {
    doeObjectFace       mu;
    void                (*describeTo)(doeE, dcPathStoreItem, dcPathConsumer);
    dcPathStoreItem     (*dismissed) (doeE, dcPathStoreItem);
} dcPathStoreItemFace;

typedef struct dcPathStoreItemData_ {
    doeObjectData       mu;
    dcPathStoreItem     next;
} dcPathStoreItemData;

/* the class cannot be instantiated; thus, no need for
        className
        copy
        describeTo
        release
*/

static void
dcPathStoreItem__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

static void
dcPathStoreItem__enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    dcPathStoreItemData* p = (dcPathStoreItemData *)o;
    cb->enumerate(env, cb, (doeObject)(p->next));
    BASE__enumCoObs(env, o, cb);
}

static dcPathStoreItem
dcPathStoreItem_dismissed(doeE env, dcPathStoreItem obj)
{
    dcPathStoreItemData*	p = (dcPathStoreItemData*)obj;
    dcPathStoreItem
		next = p->next;
    p->next = NULL;

    dcPool_staticReleaseItem(env, obj);

    return next;
}

dcPathStoreItemFace dcPathStoreItemClass = {
    {
	0,

	NULL,
	NULL,
	dcPathStoreItem__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    0,
    dcPathStoreItem_dismissed
};

static void
dcPathStoreItem_init(doeE env, dcPathStoreItem target)
{
    dcPathStoreItemData* p = (dcPathStoreItemData *)target;

    BASE_init(env, target);
    p->next = NULL;
}

#undef BASE

#define BASE	dcPathStoreItem

/*	beginSubpath item	*/

typedef struct beginSubpathData_ {
    dcPathStoreItemData	mu;
    f32			x0, y0;
} beginSubpathData;

static char*
beginSubpath_className(doeE env, doeObject o)	{ return "PathStoreItem_beginSubpath"; }

static doeObject
beginSubpath_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
beginSubpath__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

/*		    _enumCoObs is    dcPathStoreItem__enumCoObs */

static void
beginSubpath_describeTo(doeE env, dcPathStoreItem obj, dcPathConsumer dest)
{
    beginSubpathData* p = (beginSubpathData *)obj;
    (*dest)->beginSubpath(env, dest, p->x0, p->y0);
}

/*		    dismissed  is    dcPathStoreItem_dismissed */

dcPathStoreItemFace beginSubpathClass = {
    {
	sizeof(beginSubpathData),
	
	beginSubpath_className,
	beginSubpath_copy,
	beginSubpath__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    beginSubpath_describeTo,
    dcPathStoreItem_dismissed
};

static void
beginSubpath_init(doeE env, dcPathStoreItem target)
{
    beginSubpathData* p = (beginSubpathData *)target;
    BASE_init(env, target);
    *target = &beginSubpathClass;
}

static dcPathStoreItem
beginSubpath_create(doeE env, dcPool pool, f32 x0, f32 y0)
{
    beginSubpathData* p = dcPool_getItem(env, pool);
    if (p == NULL) {
	return NULL;
    }
    beginSubpath_init(env, (dcPathStoreItem)p);

    p->x0 = x0;
    p->y0 = y0;

    return (dcPathStoreItem)p;
}

/*	appendLine item	*/

typedef struct appendLineData_ {
    dcPathStoreItemData	mu;
    f32			x1, y1;
} appendLineData;

static char*
appendLine_className(doeE env, doeObject o) { return "PathStoreItem_appendLine"; }

static doeObject
appendLine_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
appendLine__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}
/*		    _enumCoObs is    dcPathStoreItem__enumCoObs */

static void
appendLine_describeTo(doeE env, dcPathStoreItem obj, dcPathConsumer dest)
{
    appendLineData* p = (appendLineData *)obj;
    (*dest)->appendLine(env, dest, p->x1, p->y1);
}

/*		    dismissed  is    dcPathStoreItem_dismissed */

dcPathStoreItemFace appendLineClass = {
    {
	sizeof(appendLineData),
	
	appendLine_className,
	appendLine_copy,
	appendLine__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    appendLine_describeTo,
    dcPathStoreItem_dismissed
};

static void
appendLine_init(doeE env, dcPathStoreItem target)
{
    appendLineData* p = (appendLineData *)target;
    BASE_init(env, target);
    *target = &appendLineClass;
}

static dcPathStoreItem
appendLine_create(doeE env, dcPool pool, f32 x1, f32 y1)
{
    appendLineData* p = dcPool_getItem(env, pool);
    if (p == NULL) {
	return NULL;
    }
    appendLine_init(env, (dcPathStoreItem)p);

    p->x1 = x1;
    p->y1 = y1;

    return (dcPathStoreItem)p;
}

/*	appendQuadratic item	*/

typedef struct appendQuadraticData_ {
    dcPathStoreItemData	mu;
    f32			x1, y1, x2, y2;
} appendQuadraticData;

static char*
appendQuadratic_className(doeE env, doeObject o) { return "PathStoreItem_appendQuadratic"; }

static doeObject
appendQuadratic_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
appendQuadratic__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}
/*		    _enumCoObs is    dcPathStoreItem__enumCoObs */

static void
appendQuadratic_describeTo(doeE env, dcPathStoreItem obj, dcPathConsumer dest)
{
    appendQuadraticData* p = (appendQuadraticData *)obj;
    (*dest)->appendQuadratic(env, dest, p->x1, p->y1, p->x2, p->y2);
}

/*		    dismissed  is    dcPathStoreItem_dismissed */

dcPathStoreItemFace appendQuadraticClass = {
    {
	sizeof(appendQuadraticData),
	
	appendQuadratic_className,
	appendQuadratic_copy,
	appendQuadratic__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    appendQuadratic_describeTo,
    dcPathStoreItem_dismissed
};

static void
appendQuadratic_init(doeE env, dcPathStoreItem target)
{
    appendQuadraticData* p = (appendQuadraticData *)target;
    BASE_init(env, target);
    *target = &appendQuadraticClass;
}

static dcPathStoreItem
appendQuadratic_create(doeE env, dcPool pool, f32 x1, f32 y1, f32 x2, f32 y2)
{
    appendQuadraticData* p = dcPool_getItem(env, pool);
    if (p == NULL) {
	return NULL;
    }
    appendQuadratic_init(env, (dcPathStoreItem)p);

    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x2;
    p->y2 = y2;

    return (dcPathStoreItem)p;
}

/*	appendCubic item	*/

typedef struct appendCubicData_ {
    dcPathStoreItemData	mu;
    f32			x1, y1, x2, y2, x3, y3;
} appendCubicData;

static char*
appendCubic_className(doeE env, doeObject o) { return "PathStoreItem_appendCubic"; }

static doeObject
appendCubic_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
appendCubic__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}
/*		    _enumCoObs is    dcPathStoreItem__enumCoObs */

static void
appendCubic_describeTo(doeE env, dcPathStoreItem obj, dcPathConsumer dest)
{
    appendCubicData* p = (appendCubicData *)obj;
    (*dest)->appendCubic(env, dest, p->x1, p->y1, p->x2, p->y2, p->x3, p->y3);
}

/*		    dismissed  is    dcPathStoreItem_dismissed */

dcPathStoreItemFace appendCubicClass = {
    {
	sizeof(appendCubicData),
	
	appendCubic_className,
	appendCubic_copy,
	appendCubic__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    appendCubic_describeTo,
    dcPathStoreItem_dismissed
};

static void
appendCubic_init(doeE env, dcPathStoreItem target)
{
    appendCubicData* p = (appendCubicData *)target;
    BASE_init(env, target);
    *target = &appendCubicClass;
}

static dcPathStoreItem
appendCubic_create(doeE env, dcPool pool, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    appendCubicData* p = dcPool_getItem(env, pool);
    if (p == NULL) {
	return NULL;
    }
    appendCubic_init(env, (dcPathStoreItem)p);

    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x2;
    p->y2 = y2;
    p->x3 = x3;
    p->y3 = y3;

    return (dcPathStoreItem)p;
}

/*	closedSubpath item	*/

typedef struct closedSubpathData_ {
    dcPathStoreItemData	mu;
} closedSubpathData;

static char*
closedSubpath_className(doeE env, doeObject o) { return "PathStoreItem_closedSubpath"; }

static doeObject
closedSubpath_copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
closedSubpath__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}
/*		    _enumCoObs is    dcPathStoreItem__enumCoObs */

static void
closedSubpath_describeTo(doeE env, dcPathStoreItem obj, dcPathConsumer dest)
{
    (*dest)->closedSubpath(env, dest);
}

/*		    dismissed  is    dcPathStoreItem_dismissed */

dcPathStoreItemFace closedSubpathClass = {
    {
	sizeof(closedSubpathData),
	
	closedSubpath_className,
	closedSubpath_copy,
	closedSubpath__cleanup,
	dcPathStoreItem__enumCoObs,
	doeObject_uproot
    },

    closedSubpath_describeTo,
    dcPathStoreItem_dismissed
};

static void
closedSubpath_init(doeE env, dcPathStoreItem target)
{
    BASE_init(env, target);
    *target = &closedSubpathClass;
}

static dcPathStoreItem
closedSubpath_create(doeE env, dcPool pool)
{
    closedSubpathData* p = dcPool_getItem(env, pool);
    if (p == NULL) {
	return NULL;
    }
    closedSubpath_init(env, (dcPathStoreItem)p);

    return (dcPathStoreItem)p;
}

#undef BASE

/*- internal (nonstatic) class FPP (a FastPathConsumer) ----*/

#define BASE doeObject

typedef struct FPPData_ {
    doeObjectData	mu;
    dcPathStoreData*	psp; /* this.this */
} FPPData;

static char*
FPP_className(doeE env, doeObject o)	{ return "PathStore_FPP"; }

static doeObject
FPP_copy(doeE env, doeObject t)	{ return t; /*!!*/ }

static void
FPP__cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

static void
FPP__enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)	{}

static void
getBox(doeE env, dcFastPathProducer fpp, f32* box)
{
    dcPathStoreData*	p = ((FPPData*)fpp)->psp;

    if (!p->pathDefined) {
	doeError_set(env, dcPathError, dcPathError_UNEX_getBox);
	return;
    }
    if (p->proxy != NULL) {
	(*(p->proxy))->getBox(env, p->proxy, box);
	return;
    }

    box[0] = p->loX;
    box[1] = p->loY;
    box[2] = p->hiX;
    box[3] = p->hiY;
}

static void
sendTo(doeE env, dcFastPathProducer fpp, dcPathConsumer dest)
{
    dcPathStoreData*	p = ((FPPData*)fpp)->psp;
    dcFastPathProducer
		proxy = p->proxy;
    dcPathStoreItem
		item;

    if (!p->pathDefined) {
	doeError_set(env, dcPathError, dcPathError_UNEX_sendTo);
	return;
    }
    if (dest == NULL) {
	doeError_set(env, dcPathError, dcPathError_BAD_pathconsumer);
	return;
    }

    if (proxy != NULL) { /* unusual */
	(*proxy)->sendTo(env, proxy, dest);
	return;
    }

    /* send the stored path */
    (*dest)->beginPath(env, dest);
    item = p->itemFirst;
    while (item != NULL) {
	(*item)->describeTo(env, item, dest);
	item = ((dcPathStoreItemData *)item)->next;
    }
    (*dest)->endPath(env, dest);
}

static dcFastPathProducerFace FPPClass = {
    {
	sizeof(FPPData),

	FPP_className,
	FPP_copy,
	FPP__cleanup,
	FPP__enumCoObs,
	doeObject_uproot,
    },

    getBox,
    sendTo
};

static void
FPP_init(doeE env, dcFastPathProducer target, dcPathStoreData* psp)
{
    FPPData*	p = (FPPData*)target;

    BASE_init(env, target);

    *target = &FPPClass;

    p->psp = psp;
}

static dcFastPathProducer
FPP_create(doeE env, dcPathStoreData* psp)
{
    dcFastPathProducer	p = doeMem_malloc(env, (i32)sizeof(FPPData));
    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    FPP_init(env, p, psp);

    return p;
}

#undef BASE
/*--------------------------------------------------------*/
/*	PathStore methods		*/

#define BASE dcPathConsumer

/*	Object interface		*/

static char*
className(doeE env, doeObject o) { return "PathStore"; }

static doeObject
copy(doeE env, doeObject t) { return t; /*!!*/ }

static void
_cleanup(doeE env, doeObject o)
{
    dcPathStore	     ps = (dcPathStore)o;
    dcPathStoreData* p =  (dcPathStoreData *)o;
    doeObject	     po; /* private object */

    (*ps)->reset(env, ps);
    po = (doeObject)(p->stored);
    if (po != NULL) {
	(*po)->_cleanup(env, po);
	doeMem_free(env, po);
    }

    dcPool_destroy(env, p->beginSubpathPool);
    dcPool_destroy(env, p->appendLinePool);
    dcPool_destroy(env, p->appendQuadraticPool);
    dcPool_destroy(env, p->appendCubicPool);
    dcPool_destroy(env, p->closedSubpathPool);

    BASE__cleanup(env, o);
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    dcPathStoreData* p = (dcPathStoreData *)o;
    if (p->itemFirst != NULL)	cb->enumerate(env, cb, (doeObject)(p->itemFirst));
    /* no need to enumerate [itemLast]; it is always
       accessible from [itemFirst] */
    if (p->proxy != NULL)	cb->enumerate(env, cb, (doeObject)(p->proxy));
    if (p->stored != NULL)	cb->enumerate(env, cb, (doeObject)(p->stored));

    BASE__enumCoObs(env, o, cb);
}

/*	PathConsumer interface		*/

void
dcPathStore_beginPath(doeE env, dcPathConsumer pc)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;

    if (p->inPath || p->pathDefined) {
	doeError_set(env, dcPathError, dcPathError_UNEX_beginPath);
	return;
    }
    p->inPath = TRUE;
}

void
dcPathStore_beginSubpath(doeE env, dcPathConsumer pc, f32 x0, f32 y0)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;
    dcPathStoreItem	item;

    if (!p->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_beginSubpath);
	return;
    }
    p->inSubpath = TRUE;

    item = beginSubpath_create(env, p->beginSubpathPool, x0, y0);
    if (doeError_occurred(env))
	return;
    if (p->itemLast != NULL)	((dcPathStoreItemData *)(p->itemLast))->next = item;
    else			p->itemFirst = item;
    p->itemLast = item;

    if (x0 < p->loX)	p->loX = x0;
    if (y0 < p->loY)	p->loY = y0;
    if (x0 > p->hiX)	p->hiX = x0;
    if (y0 > p->hiY)	p->hiY = y0;
}

void
dcPathStore_appendLine(doeE env, dcPathConsumer pc, f32 x1, f32 y1)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;
    dcPathStoreItem	item;

    if (!p->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendLine);
	return;
    }

    item = appendLine_create(env, p->appendLinePool, x1, y1);
    if (doeError_occurred(env))
	return;

    ((dcPathStoreItemData *)(p->itemLast))->next = item;
    p->itemLast = item;

    if (x1 < p->loX)	p->loX = x1;
    if (y1 < p->loY)	p->loY = y1;
    if (x1 > p->hiX)	p->hiX = x1;
    if (y1 > p->hiY)	p->hiY = y1;
}

void
dcPathStore_appendQuadratic(	doeE env, dcPathConsumer pc,	f32 x1, f32 y1,
								f32 x2, f32 y2)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;
    dcPathStoreItem	item;

    if (!p->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendQuadratic);
	return;
    }

    item = appendQuadratic_create(env, p->appendQuadraticPool, x1, y1, x2, y2);
    if (doeError_occurred(env))
	return;

    ((dcPathStoreItemData *)(p->itemLast))->next = item;
    p->itemLast = item;

    if (x1 < p->loX)	p->loX = x1;
    if (y1 < p->loY)	p->loY = y1;
    if (x1 > p->hiX)	p->hiX = x1;
    if (y1 > p->hiY)	p->hiY = y1;

    if (x2 < p->loX)	p->loX = x2;
    if (y2 < p->loY)	p->loY = y2;
    if (x2 > p->hiX)	p->hiX = x2;
    if (y2 > p->hiY)	p->hiY = y2;
}

void
dcPathStore_appendCubic(	doeE env, dcPathConsumer pc,	f32 x1, f32 y1,
								f32 x2, f32 y2,
								f32 x3, f32 y3)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;
    dcPathStoreItem	item;

    if (!p->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_appendCubic);
	return;
    }

    item = appendCubic_create(env, p->appendCubicPool, x1, y1, x2, y2, x3, y3);
    if (doeError_occurred(env))
	return;

    ((dcPathStoreItemData *)(p->itemLast))->next = item;
    p->itemLast = item;

    if (x1 < p->loX)	p->loX = x1;
    if (y1 < p->loY)	p->loY = y1;
    if (x1 > p->hiX)	p->hiX = x1;
    if (y1 > p->hiY)	p->hiY = y1;

    if (x2 < p->loX)	p->loX = x2;
    if (y2 < p->loY)	p->loY = y2;
    if (x2 > p->hiX)	p->hiX = x2;
    if (y2 > p->hiY)	p->hiY = y2;

    if (x3 < p->loX)	p->loX = x3;
    if (y3 < p->loY)	p->loY = y3;
    if (x3 > p->hiX)	p->hiX = x3;
    if (y3 > p->hiY)	p->hiY = y3;
}

void
dcPathStore_closedSubpath(doeE env, dcPathConsumer pc)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;
    dcPathStoreItem	item;

    if (!p->inSubpath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_closedSubpath);
	return;
    }

    item = closedSubpath_create(env, p->closedSubpathPool);
    if (doeError_occurred(env))
	return;

    ((dcPathStoreItemData *)(p->itemLast))->next = item;
    p->itemLast = item;

}

void
dcPathStore_endPath(doeE env, dcPathConsumer pc)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;

    if (!p->inPath) {
	doeError_set(env, dcPathError, dcPathError_UNEX_endPath);
	return;
    }

    p->inPath = p->inSubpath = FALSE;
    p->pathDefined = TRUE;
}

void
dcPathStore_useProxy(doeE env, dcPathConsumer pc, dcFastPathProducer proxy)
{
    dcPathStoreData*	p = (dcPathStoreData *)pc;

    if (p->inPath || p->pathDefined) {
	doeError_set(env, dcPathError, dcPathError_UNEX_useProxy);
	return;
    }
    p->proxy = proxy;
    p->pathDefined = TRUE;
}

/*	PathStore interface		*/

dcFastPathProducer
dcPathStore_getFastPathProducer(doeE env, dcPathStore target)
{
    dcPathStoreData*	p = (dcPathStoreData *)target;

    return p->stored;
}

void
dcPathStore_reset(doeE env, dcPathStore target)
{
    dcPathStoreData*	p = (dcPathStoreData *)target;
    dcPathStoreItem	item;

    p->inPath = p->inSubpath = p->pathDefined = FALSE;
    p->loX = p->loY =	1.0e20;
    p->hiX = p->hiY =  -1.0e20;
    item = p->itemFirst;
    while (item != NULL)
	item = (*item)->dismissed(env, item);
    p->itemFirst = p->itemLast = NULL;
    p->proxy = NULL;

    dcPool_endCycle(env, p->beginSubpathPool);
    dcPool_endCycle(env, p->appendLinePool);
    dcPool_endCycle(env, p->appendQuadraticPool);
    dcPool_endCycle(env, p->appendCubicPool);
    dcPool_endCycle(env, p->closedSubpathPool);
}

dcPathStoreFace dcPathStoreClass = {
    {
	{
	    sizeof(dcPathStoreData),
	    
	    className,
	    copy,
	    _cleanup,
	    _enumCoObs,
	    doeObject_uproot,
	},
	
	dcPathStore_PARENT_METHODS, 
    },
    dcPathStore_SELF_METHODS
};

void
dcPathStore_init(doeE env, dcPathStore target)
{
    dcPathStoreData*	p = (dcPathStoreData *)target;

    BASE_init(env, target);
    p->stored = FPP_create(env, (dcPathStoreData *)target); 

    p->beginSubpathPool =	dcPool_create(env, "beginSubpath pool",
						    sizeof(beginSubpathData),	  0, 1.0);
    p->appendLinePool =		dcPool_create(env, "appendLine pool",
						    sizeof(appendLineData),	  0, 1.0);
    p->appendQuadraticPool =	dcPool_create(env, "appendQuadratic pool",
						    sizeof(appendQuadraticData),  0, 1.0);
    p->appendCubicPool =	dcPool_create(env, "appendCubic pool",
						    sizeof(appendCubicData),	  0, 1.0);
    p->closedSubpathPool =	dcPool_create(env, "closedSubpath pool",
						    sizeof(closedSubpathData),	  0, 1.0);
    p->itemFirst = p->itemLast = NULL;
    dcPathStore_reset(env, target);
}

void
dcPathStore_copyinit(doeE env, dcPathStore target, dcPathStore src)
{
    BASE_copyinit(env, target, src);
}

dcPathStore
dcPathStore_create(doeE env)
{
    dcPathStore		p;

    p   = doeMem_malloc(env, (i32)sizeof(dcPathStoreData));
    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcPathStore_init(env, p);
    if (doeError_occurred(env)) {
	_cleanup(env, (doeObject)p);
	doeMem_free(env, p);
	return NULL;
    }
    return p;
}



static ixx clients = 0;

void
dcPathStore_staticInitialize(doeE env)
{
    if (clients++ > 0)	return;

    dcPool_staticInitialize(env);
}

void

dcPathStore_staticFinalize  (doeE env)
{
    if (--clients > 0)	return;

    dcPool_staticFinalize(env);
}

#undef BASE




