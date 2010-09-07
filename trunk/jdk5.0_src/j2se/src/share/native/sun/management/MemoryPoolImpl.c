/*
 * @(#)MemoryPoolImpl.c	1.8 04/02/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "management.h"
#include "sun_management_MemoryPoolImpl.h"

JNIEXPORT jobject JNICALL
Java_sun_management_MemoryPoolImpl_getMemoryManagers0
  (JNIEnv *env, jobject pool)
{
    jobject mgrs = jmm_interface->GetMemoryManagers(env, pool);
    if (mgrs == NULL) {
        // Throw internal error since this implementation expects the
        // pool will never become invalid.
        JNU_ThrowInternalError(env, "Memory Pool not found");
    }
    return mgrs;
}

JNIEXPORT jobject JNICALL
Java_sun_management_MemoryPoolImpl_getUsage0
  (JNIEnv *env, jobject pool)
{
    jobject usage = jmm_interface->GetMemoryPoolUsage(env, pool);
    if (usage == NULL) {
        // Throw internal error since this implementation expects the
        // pool will never become invalid.
        JNU_ThrowInternalError(env, "Memory Pool not found");
    }
    return usage;
}

JNIEXPORT jobject JNICALL
Java_sun_management_MemoryPoolImpl_getPeakUsage0
  (JNIEnv *env, jobject pool)
{
    jobject usage = jmm_interface->GetPeakMemoryPoolUsage(env, pool);
    if (usage == NULL) {
        // Throw internal error since this implementation expects the
        // pool will never become invalid.
        JNU_ThrowInternalError(env, "Memory Pool not found");
    }
    return usage;
}

JNIEXPORT void JNICALL
Java_sun_management_MemoryPoolImpl_setUsageThreshold0
  (JNIEnv *env, jobject pool, jlong current, jlong newThreshold)
{
    // Set both high and low threshold to the same threshold
    if (newThreshold > current) {
        // high threshold has to be set first so that high >= low
        jmm_interface->SetPoolThreshold(env, pool, 
                                        JMM_USAGE_THRESHOLD_HIGH, newThreshold);
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_USAGE_THRESHOLD_LOW, newThreshold);
    } else {
        // low threshold has to be set first so that high >= low
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_USAGE_THRESHOLD_LOW, newThreshold);
        jmm_interface->SetPoolThreshold(env, pool, 
                                        JMM_USAGE_THRESHOLD_HIGH, newThreshold);
    }
}

JNIEXPORT void JNICALL
Java_sun_management_MemoryPoolImpl_setCollectionThreshold0
  (JNIEnv *env, jobject pool, jlong current, jlong newThreshold)
{
    // Set both high and low threshold to the same threshold
    if (newThreshold > current) {
        // high threshold has to be set first so that high >= low
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_COLLECTION_USAGE_THRESHOLD_HIGH,
                                        newThreshold);
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_COLLECTION_USAGE_THRESHOLD_LOW,
                                        newThreshold);
    } else {
        // low threshold has to be set first so that high >= low
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_COLLECTION_USAGE_THRESHOLD_LOW,
                                        newThreshold);
        jmm_interface->SetPoolThreshold(env, pool,
                                        JMM_COLLECTION_USAGE_THRESHOLD_HIGH,
                                        newThreshold);
    }
}

JNIEXPORT void JNICALL
Java_sun_management_MemoryPoolImpl_resetPeakUsage0
  (JNIEnv *env, jobject pool)
{
    jvalue value;
    value.l = pool;
    jmm_interface->ResetStatistic(env, value, JMM_STAT_PEAK_POOL_USAGE);
}

JNIEXPORT void JNICALL
Java_sun_management_MemoryPoolImpl_setPoolUsageSensor
  (JNIEnv *env, jobject pool, jobject sensor)
{
    jmm_interface->SetPoolSensor(env, pool,
                                 JMM_USAGE_THRESHOLD_HIGH, sensor);
}

JNIEXPORT void JNICALL
Java_sun_management_MemoryPoolImpl_setPoolCollectionSensor
  (JNIEnv *env, jobject pool, jobject sensor)
{
    jmm_interface->SetPoolSensor(env, pool,
                                 JMM_COLLECTION_USAGE_THRESHOLD_HIGH, sensor);
}

JNIEXPORT jobject JNICALL
Java_sun_management_MemoryPoolImpl_getCollectionUsage0
  (JNIEnv *env, jobject pool)
{
    return jmm_interface->GetPoolCollectionUsage(env, pool);
}

