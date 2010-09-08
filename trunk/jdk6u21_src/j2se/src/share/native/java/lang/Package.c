/*
 * @(#)Package.c	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include "jvm.h"
#include "jni_util.h"
#include "java_lang_Package.h"

JNIEXPORT jstring JNICALL
Java_java_lang_Package_getSystemPackage0(JNIEnv *env, jclass cls, jstring str)
{
    return JVM_GetSystemPackage(env, str);
}

JNIEXPORT jobject JNICALL
Java_java_lang_Package_getSystemPackages0(JNIEnv *env, jclass cls)
{
    return JVM_GetSystemPackages(env);
}
