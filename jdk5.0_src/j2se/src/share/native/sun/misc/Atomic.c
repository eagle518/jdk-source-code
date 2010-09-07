/*
 * @(#)Atomic.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>

#include <jni.h>
#include <jvm.h>
#include <jni_util.h>
#include <jlong.h>
#include "sun_misc_AtomicLong.h"

JNIEXPORT jboolean JNICALL 
Java_sun_misc_AtomicLong_VMSupportsCS8(JNIEnv *env, jclass cls)
{
  return JVM_SupportsCX8();
}

JNIEXPORT jboolean JNICALL 
Java_sun_misc_AtomicLongCSImpl_attemptUpdate(JNIEnv *env, jobject this, jlong oldValue, jlong newValue)
{
  static jfieldID valueField = 0;
  jboolean rv;

  if (valueField == 0) {
    jclass clazz = (*env)->FindClass(env, "sun/misc/AtomicLongCSImpl");
    valueField = (*env)->GetFieldID(env, clazz, "value", "J");
  }

  rv = JVM_CX8Field(env, this, valueField, oldValue, newValue);
  return rv;
}

