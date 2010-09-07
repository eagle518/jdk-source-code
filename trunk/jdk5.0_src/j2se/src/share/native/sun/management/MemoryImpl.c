/*
 * @(#)MemoryImpl.c	1.5 04/02/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_MemoryImpl.h"

JNIEXPORT void JNICALL Java_sun_management_MemoryImpl_setVerboseGC
  (JNIEnv *env, jobject dummy, jboolean flag) {
    jmm_interface->SetBoolAttribute(env, JMM_VERBOSE_GC, flag);
}

JNIEXPORT jobject JNICALL Java_sun_management_MemoryImpl_getMemoryPools0
  (JNIEnv *env, jclass dummy) {
    return jmm_interface->GetMemoryPools(env, NULL);
}

JNIEXPORT jobject JNICALL Java_sun_management_MemoryImpl_getMemoryManagers0
  (JNIEnv *env, jclass dummy) {
    return jmm_interface->GetMemoryManagers(env, NULL);
}

JNIEXPORT jobject JNICALL Java_sun_management_MemoryImpl_getMemoryUsage0
  (JNIEnv *env, jobject dummy, jboolean heap) {
    return jmm_interface->GetMemoryUsage(env, heap);
}
