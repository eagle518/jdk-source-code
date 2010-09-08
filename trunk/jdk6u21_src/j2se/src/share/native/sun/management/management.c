/*
 * @(#)management.c	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <jni.h>
#include "jvm.h"
#include "management.h"

#define ERR_MSG_SIZE 128

const JmmInterface* jmm_interface = NULL;
JavaVM* jvm = NULL;
jint jmm_version = 0;

JNIEXPORT jint JNICALL 
   JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv* env;

    jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_2) != JNI_OK) {
        return JNI_ERR;
    }

    jmm_interface = (JmmInterface*) JVM_GetManagement(JMM_VERSION_1_0);
    if (jmm_interface == NULL) {
        JNU_ThrowInternalError(env, "Unsupported Management version");
        return JNI_ERR;
    }

    jmm_version = jmm_interface->GetVersion(env);
    return (*env)->GetVersion(env);
}

void throw_internal_error(JNIEnv* env, const char* msg) {
    char errmsg[128];

    sprintf(errmsg, "errno: %d error: %s\n", errno, msg);
    JNU_ThrowInternalError(env, errmsg);
}

