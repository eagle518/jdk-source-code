/*
 * @(#)nio_util.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include <sys/types.h>


/* NIO utility procedures */


/* Defined in IOUtil.c */

jint fdval(JNIEnv *env, jobject fdo);

jint convertReturnVal(JNIEnv *env, jint n, jboolean reading);
jlong convertLongReturnVal(JNIEnv *env, jlong n, jboolean reading);


/* Defined in Net.c */

jint handleSocketError(JNIEnv *env, jint errorValue);
