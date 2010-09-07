/*
 * @(#)doeSun.c	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)doeSun.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include "doe.h"

/* The only difference between this version and the one under /c/doe
   is the implementation of Mutex; this version uses JNI Monitors.
 */

#define	doeMutex_create_FAILED	1
#define	doeMutex_lock_FAILED	2
#define	doeMutex_unlock_FAILED	3

char*	doeErrors[] = {
	"java/lang/RuntimeException",
	"doeMutex_create failed",
	"doeMutex_lock failed",
	"doeMutex_unlock failed",
	""
};

/*
 *  ------------------------------------
 *	ENVIRONMENT
 *  ------------------------------------
 */

static void reporterNot(doeE env, char* msg) {}
static void _doeError_set(doeE env, char** msgs, ixx index);
static void _doeError_setNoMemory(doeE env);

doeE
doeE_make()
{
    doeE e = (doeE)malloc(sizeof(doeEData));

    if (e != NULL) {
	e->msgtable = NULL;
	e->msgindex = 0;
	e->errorfunc = _doeError_set;
	e->nomemfunc = _doeError_setNoMemory;
	e->reporter = reporterNot;
	e->thname = NULL;
	e->pctxt = NULL;
    }
    return e;
}
void
doeE_destroy(doeE e)
{
    if (e != NULL) {
	free(e);
    }
}

/*
 *  ------------------------------------
 *	DYNAMIC MEMORY ALLOCATION
 *  ------------------------------------
 */

typedef struct	doeMemHead_*	doeMem;
typedef struct	doeMemHead_ {
    i32		size;		/* in bytes */
#if defined(_LP64) || defined(_WIN64)
    i32         pad;
#endif
} doeMemHead;


void*
doeMem_malloc(doeE env, i32 bytes)
{
    doeMem	m = (doeMem)malloc(sizeof(doeMemHead) + bytes);

    if (m != 0) {
	m->size = bytes;
	return (void*)(m + 1);
    }
    return NULL;
    /* do garbage collection */
}

void*
doeMem_realloc(doeE env, void* mem, i32 bytes)
{
    doeMem	old = (doeMem)mem - 1;
    doeMem	new;

    if (mem == NULL)
	return doeMem_malloc(env, bytes);

    new = (doeMem)realloc((void*)old, sizeof(doeMemHead) + bytes);
    if (new != 0) {
	new->size = bytes;
	return (void*)(new + 1);
    }
    return NULL;
    /* do garbage collection */
}

i32
doeMem_size(doeE env, void* mem)
{
    doeMem	m = (doeMem)mem - 1;
    return m->size;
}

void
doeMem_free(doeE env, void* mem)
{
    doeMem	m = (doeMem)mem - 1;
    free((char*)m);
}

/*
 *  ------------------------------------
 *	FILE ACCESS
 *  ------------------------------------
 */

/*
 *  ------------------------------------
 *	MUTUAL EXCLUSION
 *  ------------------------------------
 */

jobject
JObject_createGlobal(doeE env)
{
    JNIEnv*	jenv = doeE_getPCtxt(env);
    jclass	oclass;
    jmethodID	cid;
    jobject	jo;

    oclass =	(*jenv)->FindClass	(jenv, "java/lang/Object");
    cid =	(*jenv)->GetMethodID(jenv, oclass, "<init>", "()V");
    jo =	(*jenv)->NewObject	(jenv, oclass, cid);
    if (jo != NULL)
	jo = (*jenv)->NewGlobalRef(jenv, jo);

    return jo;
}

void
JObject_destroyGlobal(doeE env, jobject jo)
{
    JNIEnv*	jenv = doeE_getPCtxt(env);
    (*jenv)->DeleteGlobalRef(jenv, jo);
}

doeMutex
doeMutex_create(doeE env)
{
    doeMutex m = (doeMutex)JObject_createGlobal(env);
    if (m == NULL)
	doeError_set(env, doeErrors, doeMutex_create_FAILED);
    return m;
}

void
doeMutex_destroy(doeE env, doeMutex m)
{
    if (m != NULL)
	JObject_destroyGlobal(env, (jobject)m);
}

void
doeMutex_lock(doeE env, doeMutex m)
{
    JNIEnv*	jenv = doeE_getPCtxt(env);

    if (0 != (*jenv)->MonitorEnter(jenv, (jobject)m))
	doeError_set(env, doeErrors, doeMutex_lock_FAILED);
}

void
doeMutex_unlock(doeE env, doeMutex m)
{
    JNIEnv*	jenv = doeE_getPCtxt(env);

    if (0 != (*jenv)->MonitorExit(jenv, (jobject)m))
	doeError_set(env, doeErrors, doeMutex_unlock_FAILED);
}

/*
 *  ------------------------------------
 *	ERROR HANDLING
 *  ------------------------------------
 */

static void _doeError_set(doeE env, char** msgs, ixx index)
{
    env->msgtable = msgs;
    env->msgindex = index;
}

static char*	NoMemoryError[] = { "java/lang/OutOfMemoryError", "" };

static void _doeError_setNoMemory(doeE env)
{
    env->msgtable = NoMemoryError;
    env->msgindex = 1;
}
