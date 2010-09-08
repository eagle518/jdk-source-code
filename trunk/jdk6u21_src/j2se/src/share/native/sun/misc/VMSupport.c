/*
 * @(#)VMSupport.c	1.2 10/03/23
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "jvm.h"
#include "jdk_util.h"

#include "sun_misc_VMSupport.h"

typedef jobject (JNICALL *INIT_AGENT_PROPERTIES_FN)(JNIEnv *, jobject);

static INIT_AGENT_PROPERTIES_FN InitAgentProperties_fp = NULL;

JNIEXPORT jobject JNICALL
Java_sun_misc_VMSupport_initAgentProperties(JNIEnv *env, jclass cls, jobject props)
{
    char errmsg[128];

    if (InitAgentProperties_fp == NULL) {
        if (!JDK_InitJvmHandle()) {
            JNU_ThrowInternalError(env,
                 "Handle for JVM not found for symbol lookup");
        }
        InitAgentProperties_fp = (INIT_AGENT_PROPERTIES_FN) 
            JDK_FindJvmEntry("JVM_InitAgentProperties");
        if (InitAgentProperties_fp == NULL) {
            JNU_ThrowInternalError(env,
                 "Mismatched VM version: JVM_InitAgentProperties not found");
            return NULL;
        }
    }
    return (*InitAgentProperties_fp)(env, props);
}

