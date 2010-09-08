/*
 * @(#)MemoryManagerImpl.c	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_MemoryManagerImpl.h"

JNIEXPORT jobject JNICALL Java_sun_management_MemoryManagerImpl_getMemoryPools0
  (JNIEnv *env, jobject mgr) {
    jobject pools = jmm_interface->GetMemoryPools(env, mgr);
    if (pools == NULL) {
        JNU_ThrowInternalError(env, "Memory Manager not found");
    }
    return pools;
}
