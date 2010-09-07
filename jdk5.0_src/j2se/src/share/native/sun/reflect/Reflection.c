/*
 * @(#)Reflection.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jvm.h"
#include "sun_reflect_Reflection.h"

JNIEXPORT jclass JNICALL Java_sun_reflect_Reflection_getCallerClass
(JNIEnv *env, jclass unused, jint depth)
{
    return JVM_GetCallerClass(env, depth);
}

JNIEXPORT jint JNICALL Java_sun_reflect_Reflection_getClassAccessFlags
(JNIEnv *env, jclass unused, jclass cls)
{
    return JVM_GetClassAccessFlags(env, cls);
}
