/*
 * @(#)ClassLoadingImpl.c	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_ClassLoadingImpl.h"

JNIEXPORT void JNICALL Java_sun_management_ClassLoadingImpl_setVerboseClass
  (JNIEnv *env, jclass cls, jboolean flag) {
    jmm_interface->SetBoolAttribute(env, JMM_VERBOSE_CLASS, flag);
}

