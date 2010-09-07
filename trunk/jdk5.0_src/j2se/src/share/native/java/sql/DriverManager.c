/*
 * @(#)DriverManager.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
