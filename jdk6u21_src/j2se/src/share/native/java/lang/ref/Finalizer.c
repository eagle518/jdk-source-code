/*
 * @(#)Finalizer.c	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "java_lang_ref_Finalizer.h"


JNIEXPORT void JNICALL
Java_java_lang_ref_Finalizer_invokeFinalizeMethod(JNIEnv *env, jclass clazz,
						  jobject ob)
{
    jclass cls;
    jmethodID mid;

    cls = (*env)->GetObjectClass(env, ob);
    if (cls == NULL) return;
    mid = (*env)->GetMethodID(env, cls, "finalize", "()V");
    if (mid == NULL) return;
    (*env)->CallVoidMethod(env, ob, mid);
}
