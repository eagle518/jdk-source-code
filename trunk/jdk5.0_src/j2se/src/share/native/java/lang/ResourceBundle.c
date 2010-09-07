/*
 * @(#)ResourceBundle.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"

#include "java_util_ResourceBundle.h"

JNIEXPORT jobjectArray JNICALL
Java_java_util_ResourceBundle_getClassContext(JNIEnv *env, jobject this)
{
    return JVM_GetClassContext(env);
}
