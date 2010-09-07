/*
 * @(#)Float.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
 * Find the bit pattern corresponding to a given float, collapsing NaNs
 */
JNIEXPORT jint JNICALL
Java_java_lang_Float_floatToIntBits(JNIEnv *env, jclass unused, jfloat v)
{
    union {
	int i;
	float f;
    } u;
    if (JVM_IsNaN((float)v)) {
        return 0x7fc00000;
    }
    u.f = (float)v;
    return (jint)u.i;
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
