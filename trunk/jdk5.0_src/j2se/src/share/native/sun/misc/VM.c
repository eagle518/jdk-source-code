/*
 * @(#)VM.c	1.2 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "jvm.h"
#include "jvm_util.h"

#include "sun_misc_VM.h"

typedef jintArray (JNICALL *GET_THREAD_STATE_VALUES_FN)(JNIEnv *, jint);
typedef jobjectArray (JNICALL *GET_THREAD_STATE_NAMES_FN)(JNIEnv *, jint, jintArray);

static GET_THREAD_STATE_VALUES_FN GetThreadStateValues_fp = NULL;
static GET_THREAD_STATE_NAMES_FN GetThreadStateNames_fp = NULL;

static void get_thread_state_info(JNIEnv *env, jint state, 
                                      jobjectArray stateValues,
                                      jobjectArray stateNames) {
    char errmsg[128];
    jintArray values;
    jobjectArray names;
    
    values = (*GetThreadStateValues_fp)(env, state);
    if (values == NULL) {
        sprintf(errmsg, "Mismatched VM version: Thread state (%d) "
                        "not supported", state);
        JNU_ThrowInternalError(env, errmsg);
        return;
    }
    /* state is also used as the index in the array */
    (*env)->SetObjectArrayElement(env, stateValues, state, values);

    names = (*GetThreadStateNames_fp)(env, state, values);
    if (names == NULL) {
        sprintf(errmsg, "Mismatched VM version: Thread state (%d) "
                        "not supported", state);
        JNU_ThrowInternalError(env, errmsg);
        return;
    }
    (*env)->SetObjectArrayElement(env, stateNames, state, names);
}

JNIEXPORT void JNICALL
Java_sun_misc_VM_getThreadStateValues(JNIEnv *env, jclass cls, 
                                      jobjectArray values,
                                      jobjectArray names)
{
    char errmsg[128];

    // check if the number of Thread.State enum constants 
    // matches the number of states defined in jvm.h 
    jsize len1 = (*env)->GetArrayLength(env, values);
    jsize len2 = (*env)->GetArrayLength(env, names);
    if (len1 != JAVA_THREAD_STATE_COUNT || len2 != JAVA_THREAD_STATE_COUNT) {
        sprintf(errmsg, "Mismatched VM version: JAVA_THREAD_STATE_COUNT = %d "
                " but JDK expects %d / %d", 
                JAVA_THREAD_STATE_COUNT, len1, len2);
        JNU_ThrowInternalError(env, errmsg);
        return;
    }

    if (GetThreadStateValues_fp == NULL) {
        GetThreadStateValues_fp = (GET_THREAD_STATE_VALUES_FN) 
            findJvmEntry("JVM_GetThreadStateValues");
        if (GetThreadStateValues_fp == NULL) {
            JNU_ThrowInternalError(env,
                 "Mismatched VM version: JVM_GetThreadStateValues not found");
            return;
        }

        GetThreadStateNames_fp = (GET_THREAD_STATE_NAMES_FN) 
            findJvmEntry("JVM_GetThreadStateNames");
        if (GetThreadStateNames_fp == NULL) {
            JNU_ThrowInternalError(env,
                 "Mismatched VM version: JVM_GetThreadStateNames not found");
            return ;
        }
    }

    get_thread_state_info(env, JAVA_THREAD_STATE_NEW, values, names);
    get_thread_state_info(env, JAVA_THREAD_STATE_RUNNABLE, values, names);
    get_thread_state_info(env, JAVA_THREAD_STATE_BLOCKED, values, names);
    get_thread_state_info(env, JAVA_THREAD_STATE_WAITING, values, names);
    get_thread_state_info(env, JAVA_THREAD_STATE_TIMED_WAITING, values, names);
    get_thread_state_info(env, JAVA_THREAD_STATE_TERMINATED, values, names);
}

JNIEXPORT void JNICALL
Java_sun_misc_VM_initialize(JNIEnv *env, jclass cls) {
    char errmsg[128];

    if (!initJvmHandle()) {
        JNU_ThrowInternalError(env, "Handle for JVM not found for symbol lookup");
    }
}

