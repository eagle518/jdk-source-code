/*
 * @(#)dcPathConsumer.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathConsumer.c 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcPathError.h"
#include "dcPathConsumer-p.h"

#define BASE doeObject

static char*
className(doeE env, doeObject o) { return ("PathConsumer"); }

static doeObject
copy(doeE env, doeObject o)
{ 
    dcPathConsumerData* st = 
        (dcPathConsumerData*)doeMem_malloc(env, sizeof(dcPathConsumerData));
    if (st == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcPathConsumer_copyinit(env, (dcPathConsumer)st, (dcPathConsumer)o);
    return (doeObject)st;
}

static void
_cleanup(doeE env, doeObject o)
{
    BASE__cleanup(env, o);
}

static void
_enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    BASE__enumCoObs(env, o, cb);
}


/*----------------------
 * The implementation of the dcPathConsumer interface
 */

static void beginPath(doeE env, dcPathConsumer pc) {}
static void beginSubpath(doeE env, dcPathConsumer pc, f32 x0, f32 y0) {}
static void appendLine(doeE env, dcPathConsumer pc, f32 x1, f32 y1) {}
static void appendQuadratic(doeE env, dcPathConsumer pc, f32 x1, f32 y1, f32 x2, f32 y2) {}
static void appendCubic(doeE env, dcPathConsumer pc, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3) {}
static void closedSubpath(doeE env, dcPathConsumer pc) {}
static void endPath(doeE env, dcPathConsumer pc) {}
static void useProxy(doeE env, dcPathConsumer pc, dcFastPathProducer fpp) {}

dcPathConsumerFace dcPathConsumerClass = {    
    {
	sizeof( dcPathConsumerData ),
	className,        /* Object interface */
	copy,
	_cleanup,
	_enumCoObs,
	doeObject_uproot
    },

    beginPath,        /* PathConsumer interface */
    beginSubpath,
    appendLine,
    appendQuadratic,
    appendCubic,
    closedSubpath,
    endPath,
    useProxy
};


/*----------------------
 * The class-related global functions
 */

dcPathConsumer
dcPathConsumer_create(doeE env) {
    dcPathConsumerData* st = 
        (dcPathConsumerData*)doeMem_malloc(env, sizeof(dcPathConsumerData));
    if (st == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcPathConsumer_init(env, (dcPathConsumer)st);
    return (dcPathConsumer)st;
}

void
dcPathConsumer_init(doeE env, dcPathConsumer o)
{
    BASE_init(env, o);
    *o = &dcPathConsumerClass;
}

void
dcPathConsumer_copyinit(doeE env, dcPathConsumer o, dcPathConsumer src)
{
    BASE_copyinit(env, o, src);
    *o = &dcPathConsumerClass;
}

#undef BASE
