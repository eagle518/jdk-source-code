/*
 * @(#)CJError.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CJError.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include <jni.h>
#include "CJError.h"

void
CJError_throw(doeE env)
{
    JNIEnv*	jenv = doeE_getPCtxt(env);
    jclass	cls  = (*jenv)->FindClass(jenv, env->msgtable[0]);

    if (cls == NULL) {
	cls  = (*jenv)->FindClass(jenv, "java/lang/ClassNotFoundException");
	(*jenv)->ThrowNew(jenv, cls, env->msgtable[0]);
	return;
    }
    (*jenv)->ThrowNew(jenv, cls, env->msgtable[env->msgindex]);
}
#ifdef OLD
{
    doeErrorData*	e = (doeErrorData*)err;
    jclass		cls;

    if (e->msgtable  == dcPRError) {
	if (e->msgindex == dcPRError_NO_MEMORY) {
	    cls = (*env)->FindClass(env, "java/lang/OutOfMemoryException");
	    (*env)->ThrowNew(env, cls, dcPRError[dcPRError_NO_MEMORY]);
	    return;
	}
	cls = (*env)->FindClass(env, "sun/dc/pr/PRError");
	(*env)->ThrowNew(env, cls, dcPRError[e->msgindex]);
	return;
    }
    if (e->msgtable  == dcPRException) {
	cls = (*env)->FindClass(env, "sun/dc/pr/PRException");
	(*env)->ThrowNew(env, cls, dcPRException[e->msgindex]);
	return;
    }

    if (e->msgtable  == dcPathError) {
	if (e->msgindex == dcPathError_NO_MEMORY) {
	    cls = (*env)->FindClass(env, "java/lang/OutOfMemoryException");
	    (*env)->ThrowNew(env, cls, dcPathError[dcPathError_NO_MEMORY]);
	    return;
	}
	cls = (*env)->FindClass(env, "sun/dc/pr/PathError");
	(*env)->ThrowNew(env, cls, dcPathError[e->msgindex]);
	return;
    }
    if (e->msgtable  == dcPathException) {
	cls = (*env)->FindClass(env, "sun/dc/pr/PathException");
	(*env)->ThrowNew(env, cls, dcPathException[e->msgindex]);
	return;
    }
}
#endif
