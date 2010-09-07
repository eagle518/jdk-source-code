/*
 * @(#)Shutdown.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

#include "java_lang_Shutdown.h"


JNIEXPORT void JNICALL
Java_java_lang_Shutdown_halt0(JNIEnv *env, jclass ignored, jint code)
{
    JVM_Halt(code);
}


JNIEXPORT void JNICALL
Java_java_lang_Shutdown_runAllFinalizers(JNIEnv *env, jclass ignored)
{
    jclass cl;
    jmethodID mid;

    if ((cl = (*env)->FindClass(env, "java/lang/ref/Finalizer"))
	&& (mid = (*env)->GetStaticMethodID(env, cl,
					    "runAllFinalizers", "()V"))) {
	(*env)->CallStaticVoidMethod(env, cl, mid);
    }
}
