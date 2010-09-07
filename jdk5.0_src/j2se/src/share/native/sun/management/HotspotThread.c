/*
 * @(#)HotspotThread.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_HotspotThread.h"

JNIEXPORT jint JNICALL
Java_sun_management_HotspotThread_getInternalThreadCount
  (JNIEnv *env, jobject dummy)
{
    jlong count = jmm_interface->GetLongAttribute(env, NULL,
                                                  JMM_VM_THREAD_COUNT);
    return (jint) count;
}

JNIEXPORT jint JNICALL
Java_sun_management_HotspotThread_getInternalThreadTimes0
  (JNIEnv *env, jobject dummy, jobjectArray names, jobjectArray times)
{
    return jmm_interface->GetInternalThreadTimes(env, names, times); 
}
