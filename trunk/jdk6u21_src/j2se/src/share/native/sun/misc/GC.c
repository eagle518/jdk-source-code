/*
 * @(#)GC.c	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <jvm.h>
#include "sun_misc_GC.h"


JNIEXPORT jlong JNICALL 
Java_sun_misc_GC_maxObjectInspectionAge(JNIEnv *env, jclass cls)
{
    return JVM_MaxObjectInspectionAge();
}
