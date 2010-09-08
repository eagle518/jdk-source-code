/*
 * @(#)Float.c	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"

#include "java_lang_Float.h"

/*
 * Find the float corresponding to a given bit pattern
 */
JNIEXPORT jfloat JNICALL
Java_java_lang_Float_intBitsToFloat(JNIEnv *env, jclass unused, jint v)
{
    union {
	int i;
	float f;
    } u;
    u.i = (long)v;
    return (jfloat)u.f;
}

/*
 * Find the bit pattern corresponding to a given float, NOT collapsing NaNs
 */
JNIEXPORT jint JNICALL
Java_java_lang_Float_floatToRawIntBits(JNIEnv *env, jclass unused, jfloat v)
{
    union {
	int i;
	float f;
    } u;
    u.f = (float)v;
    return (jint)u.i;
}
