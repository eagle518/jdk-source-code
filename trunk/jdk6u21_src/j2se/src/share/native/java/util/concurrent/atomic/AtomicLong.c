/*
 * @(#)AtomicLong.c	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <jvm.h>
#include "java_util_concurrent_atomic_AtomicLong.h"

JNIEXPORT jboolean JNICALL
Java_java_util_concurrent_atomic_AtomicLong_VMSupportsCS8(JNIEnv *env, jclass cls)
{
    return JVM_SupportsCX8();
}
