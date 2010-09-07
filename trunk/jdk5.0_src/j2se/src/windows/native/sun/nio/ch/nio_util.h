/*
 * @(#)nio_util.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"

jint fdval(JNIEnv *env, jobject fdo);
jlong handleval(JNIEnv *env, jobject fdo);
jboolean isNT();
jint convertReturnVal(JNIEnv *env, jint n, jboolean r);
jlong convertLongReturnVal(JNIEnv *env, jlong n, jboolean r);
jboolean purgeOutstandingICMP(JNIEnv *env, jclass clazz, jint fd);
jint handleSocketError(JNIEnv *env, int errorValue);

#ifdef _WIN64

struct iovec {
    jlong  iov_base;
    jint  iov_len;
};

#else

struct iovec {
    jint  iov_base;
    jint  iov_len;
};

#endif
