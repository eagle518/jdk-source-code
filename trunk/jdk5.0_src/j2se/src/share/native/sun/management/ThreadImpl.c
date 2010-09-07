/*
 * @(#)ThreadImpl.c	1.9 04/03/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "jvm.h"
#include "management.h"
#include "sun_management_ThreadImpl.h"

JNIEXPORT void JNICALL
Java_sun_management_ThreadImpl_setThreadContentionMonitoringEnabled0
  (JNIEnv *env, jclass cls, jboolean flag)
{
    jmm_interface->SetBoolAttribute(env, JMM_THREAD_CONTENTION_MONITORING, flag);
}

JNIEXPORT void JNICALL
Java_sun_management_ThreadImpl_setThreadCpuTimeEnabled0
  (JNIEnv *env, jclass cls, jboolean flag)
{
    jmm_interface->SetBoolAttribute(env, JMM_THREAD_CPU_TIME, flag);
}


JNIEXPORT void JNICALL
Java_sun_management_ThreadImpl_getThreadInfo0
  (JNIEnv *env, jclass cls, jlongArray ids, jint maxDepth, 
   jobjectArray infoArray)
{
    jmm_interface->GetThreadInfo(env, ids, maxDepth, infoArray);
}

JNIEXPORT jobjectArray JNICALL
Java_sun_management_ThreadImpl_getThreads
  (JNIEnv *env, jclass cls)
{
    return JVM_GetAllThreads(env, cls);
}

JNIEXPORT jlong JNICALL
Java_sun_management_ThreadImpl_getThreadTotalCpuTime0
  (JNIEnv *env, jclass cls, jlong tid)
{
    return jmm_interface->GetThreadCpuTimeWithKind(env, tid, JNI_TRUE /* user+sys */);
}

JNIEXPORT jlong JNICALL
Java_sun_management_ThreadImpl_getThreadUserCpuTime0
  (JNIEnv *env, jclass cls, jlong tid)
{
    return jmm_interface->GetThreadCpuTimeWithKind(env, tid, JNI_FALSE /* user */);
}

JNIEXPORT jobjectArray JNICALL
Java_sun_management_ThreadImpl_findMonitorDeadlockedThreads0
  (JNIEnv *env, jclass cls)
{
    return jmm_interface->FindCircularBlockedThreads(env);
}

JNIEXPORT void JNICALL
Java_sun_management_ThreadImpl_resetPeakThreadCount0
  (JNIEnv *env, jclass cls)
{
    jvalue unused;
    unused.i = 0;
    jmm_interface->ResetStatistic(env, unused, JMM_STAT_PEAK_THREAD_COUNT);
}

JNIEXPORT void JNICALL
Java_sun_management_ThreadImpl_resetContentionTimes0
  (JNIEnv *env, jobject dummy, jlong tid)
{
    jvalue value;
    value.j = tid;
    jmm_interface->ResetStatistic(env, value, JMM_STAT_THREAD_CONTENTION_TIME);
}
