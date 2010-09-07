/*
 * @(#)ClassLoadingImpl.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_ClassLoadingImpl.h"

JNIEXPORT void JNICALL Java_sun_management_ClassLoadingImpl_setVerboseClass
  (JNIEnv *env, jclass cls, jboolean flag) {
    jmm_interface->SetBoolAttribute(env, JMM_VERBOSE_CLASS, flag);
}

