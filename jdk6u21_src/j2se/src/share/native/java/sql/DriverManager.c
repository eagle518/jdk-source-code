/*
 * @(#)DriverManager.c	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

JNIEXPORT jobject JNICALL
Java_java_sql_DriverManager_getCallerClassLoader(JNIEnv *env, jobject this)
{
    jclass caller = JVM_GetCallerClass(env, 2);
    return caller != 0 ? JVM_GetClassLoader(env, caller) : 0;
}
