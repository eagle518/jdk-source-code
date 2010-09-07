/*
 * @(#)Double.c	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "jvm.h"

#include "java_lang_Double.h"

/* 
 * Find the double float corresponding to a given bit pattern
 */
JNIEXPORT jdouble JNICALL
Java_java_lang_Double_longBitsToDouble(JNIEnv *env, jclass unused, jlong v)
{
    union {
	jlong l;
	double d;
    } u;
    jlong_to_jdouble_bits(&v);
    u.l = v;
    return (jdouble)u.d;
}

/*
 * Find the bit pattern corresponding to a given double float, collapsing NaNs
 */
JNIEXPORT jlong JNICALL
Java_java_lang_Double_doubleToLongBits(JNIEnv *env, jclass unused, jdouble v)
{
    union {
	jlong l;
	double d;
    } u;
    if (JVM_IsNaN((double)v)) {
        u.l = jint_to_jlong(0x7ff80000);
	return jlong_shl(u.l, 32);
    }
    jdouble_to_jlong_bits(&v);
    u.d = (double)v;
    return u.l;
}

/*
 * Find the bit pattern corresponding to a given double float, NOT collapsing NaNs
 */
JNIEXPORT jlong JNICALL
Java_java_lang_Double_doubleToRawLongBits(JNIEnv *env, jclass unused, jdouble v)
{
    union {
	jlong l;
	double d;
    } u;
    jdouble_to_jlong_bits(&v);
    u.d = (double)v;
    return u.l;
}
