/*
 * @(#)ResourceBundle.c	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"

#include "java_util_ResourceBundle.h"

JNIEXPORT jobjectArray JNICALL
Java_java_util_ResourceBundle_getClassContext(JNIEnv *env, jobject this)
{
    return JVM_GetClassContext(env);
}
