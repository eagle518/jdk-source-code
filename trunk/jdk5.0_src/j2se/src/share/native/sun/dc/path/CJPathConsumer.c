/*
 * @(#)CJPathConsumer.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CJPathConsumer.c 3.2 97/11/19
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include <jni.h>
#include "doe.h"
#include "doeObject-p.h"
#include "dcPathError.h"

#include "dcPathConsumer-p.h"
#include "CJPathConsumer.h"


typedef struct CJPCData_ {
    doeObjectData	obj;
    jobject		jpc;
} CJPCData;

static jmethodID	beginPathMID;
static jmethodID	beginSubpathMID;
static jmethodID	appendLineMID;
static jmethodID	appendQuadraticMID;
static jmethodID	appendCubicMID;
static jmethodID	closedSubpathMID;
static jmethodID	endPathMID;
static jmethodID	useProxyMID;


/* _____________________________________________________________________________
 *
 *	CJ PathConsumer
 * _____________________________________________________________________________
 */

#define	BASE	dcPathConsumer

static char*
PC_className(doeE env, doeObject o)
{
    return "CJPathConsumer";
}

static doeObject
PC_copy(doeE env, doeObject o)
{
    return o;
}

static void
PC__cleanup(doeE env, doeObject o)
{
    CJPCData*	p    = (CJPCData*)o;
    JNIEnv*	jenv = (JNIEnv*)doeE_getPCtxt(env);

    BASE__cleanup(env, o);
    (*jenv)->DeleteGlobalRef(jenv, p->jpc); 
}

static void
PC__enumCoObs(doeE env, doeObject o, doeObjectEnumCb cb)
{
    BASE__enumCoObs(env, o, cb);
}

#undef	BASE

/* -----------------------------------------------------------------------------
 * The implementation of the dcPathConsumer interface
 */
static void
PC_beginPath(doeE env, dcPathConsumer pc)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, beginPathMID);
}
static void
PC_beginSubpath(doeE env, dcPathConsumer pc, f32 x0, f32 y0)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, beginSubpathMID, x0, y0);
}
static void
PC_appendLine(doeE env, dcPathConsumer pc, f32 x1, f32 y1)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, appendLineMID, x1, y1);
}
static void
PC_appendQuad(doeE env, dcPathConsumer pc, f32 xm, f32 ym, f32 x1, f32 y1)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, appendQuadraticMID, xm, ym, x1, y1);
}
static void
PC_appendCubic(doeE env, dcPathConsumer pc,
		f32 xm, f32 ym, f32 xn, f32 yn, f32 x1, f32 y1)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, appendCubicMID,
					  xm, ym, xn, yn, x1, y1);
}
static void
PC_closedSubpath(doeE env, dcPathConsumer pc)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, closedSubpathMID);
}
static void
PC_endPath(doeE env, dcPathConsumer pc)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->CallVoidMethod(jenv, p->jpc, endPathMID);
}
static void
PC_useProxy(doeE env, dcPathConsumer pc, dcFastPathProducer proxy)
{
    doeError_set(env, dcPathError, dcPathError_UNEX_useProxy);
}

static void
setJPathConsumer(doeE env, CJPathConsumer pc, jobject jpc)
{
    CJPCData*	p    = (CJPCData*)pc;
    JNIEnv*	jenv = doeE_getPCtxt(env);

    (*jenv)->DeleteGlobalRef(jenv, p->jpc);
    p->jpc = (*jenv)->NewGlobalRef(jenv, jpc);
}

CJPathConsumerFace CJPCClass = {
    {
	{ 
	    sizeof(CJPCData),	/* Object i/f */
	    PC_className,
	    PC_copy,
	    PC__cleanup,
	    PC__enumCoObs,
	    doeObject_uproot,
	},

	PC_beginPath,		/* PathConsumer i/f */
	PC_beginSubpath,
	PC_appendLine,
	PC_appendQuad,
	PC_appendCubic,
	PC_closedSubpath,
	PC_endPath,
	PC_useProxy
    },
    setJPathConsumer	/* CJPathConsumer i/f */
};

/* -----------------------------------------------------------------------------
 * The class-related global functions
 */

static void
CJPathConsumer_init(doeE env, CJPathConsumer target)
{
    CJPCData*	p   = (CJPCData*)target;

    doeObject_init(env, (doeObject)target);
    p->obj.face	= (doeObjectFace*)&CJPCClass;
}

CJPathConsumer
CJPathConsumer_create(doeE env, jobject obj)
{
    CJPCData*	p;
    JNIEnv*	jenv = doeE_getPCtxt(env);

    p = (CJPCData*)doeMem_malloc(env, sizeof(CJPCData));
    if (p == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    CJPathConsumer_init(env, (CJPathConsumer)p);
    if (doeError_occurred(env)) {
	doeMem_free(env, p);
	return NULL;
    }

    /* _________________________________
     * the Java PathConsumer counterpart
     */
    p->jpc = (*jenv)->NewGlobalRef(jenv, obj);

    return (CJPathConsumer)p;
}

void
CJPathConsumer_staticInitialize(doeE env)
{
    jclass	cls;
    JNIEnv*	jenv = doeE_getPCtxt(env);

    cls = (*jenv)->FindClass(jenv, "sun/dc/path/PathConsumer");
    beginPathMID	= (*jenv)->GetMethodID(jenv, cls, "beginPath",	"()V");
    beginSubpathMID	= (*jenv)->GetMethodID(jenv, cls, "beginSubpath", "(FF)V");

    appendLineMID	= (*jenv)->GetMethodID(jenv, cls, "appendLine",	"(FF)V");
    appendQuadraticMID	=(*jenv)->GetMethodID(jenv, cls, "appendQuadratic",
							  "(FFFF)V");
    appendCubicMID	= (*jenv)->GetMethodID(jenv, cls, "appendCubic",
							  "(FFFFFF)V");

    closedSubpathMID	= (*jenv)->GetMethodID(jenv, cls, "closedSubpath","()V");
    endPathMID		= (*jenv)->GetMethodID(jenv, cls, "endPath",	"()V");
    useProxyMID		= (*jenv)->GetMethodID(jenv, cls, "useProxy",
					"(Lsun/dc/path/FastPathProducer;)V");
}

void
CJPathConsumer_staticFinalize(doeE env) {
}
