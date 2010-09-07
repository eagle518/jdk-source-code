/*
 * @(#)Package.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
