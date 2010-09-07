/*
 * @(#)Signal.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <signal.h>
#include <stdlib.h>

#include <jni.h>
#include <jvm.h>
#include <jni_util.h>
#include <jlong.h>
#include "sun_misc_Signal.h"

JNIEXPORT jint JNICALL 
Java_sun_misc_Signal_findSignal(JNIEnv *env, jclass cls, jstring name)
{
    jint res;
    const char *cname = (*env)->GetStringUTFChars(env, name, 0);
    if (cname == NULL) {
        /* out of memory thrown */
        return 0;
    }
    res = JVM_FindSignal(cname);
    (*env)->ReleaseStringUTFChars(env, name, cname);
    return res;
}

JNIEXPORT jlong JNICALL 
Java_sun_misc_Signal_handle0(JNIEnv *env, jclass cls, jint sig, jlong handler)
{
    return ptr_to_jlong(JVM_RegisterSignal(sig, jlong_to_ptr(handler)));
}

JNIEXPORT void JNICALL 
Java_sun_misc_Signal_raise0(JNIEnv *env, jclass cls, jint sig)
{
    JVM_RaiseSignal(sig);
}
