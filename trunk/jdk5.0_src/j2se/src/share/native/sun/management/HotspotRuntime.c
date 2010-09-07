/*
 * @(#)HotspotRuntime.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include "management.h"
#include "sun_management_HotspotRuntime.h"

static jobject vm_default_source = NULL;
static jobject vm_option_source = NULL;
static jobject ergo_source = NULL;
static jobject other_source = NULL;

JNIEXPORT jint JNICALL
Java_sun_management_HotspotRuntime_getInternalFlagCount
  (JNIEnv *env, jobject dummy)
{
    jlong count = jmm_interface->GetLongAttribute(env, NULL,
                                                  JMM_VM_GLOBAL_COUNT);
    return (jint) count;
}

JNIEXPORT jobjectArray JNICALL
  Java_sun_management_HotspotRuntime_getAllFlagNames
(JNIEnv *env, jobject dummy)
{
  return jmm_interface->GetVMGlobalNames(env);
}

JNIEXPORT void JNICALL
Java_sun_management_HotspotRuntime_initialize
  (JNIEnv *env, jclass cls)
{
    jvalue field;

    field = JNU_GetStaticFieldByName(env,
                                     NULL,
                                     "sun/management/Flag$FlagSource",
                                     "vmDefault",
                                     "Lsun/management/Flag$FlagSource;");
    vm_default_source = (*env)->NewGlobalRef(env, field.l);

    field = JNU_GetStaticFieldByName(env,
                                     NULL,
                                     "sun/management/Flag$FlagSource",
                                     "other",
                                     "Lsun/management/Flag$FlagSource;");
    other_source = (*env)->NewGlobalRef(env, field.l);
}

JNIEXPORT jint JNICALL
Java_sun_management_HotspotRuntime_getFlags
  (JNIEnv *env, jobject dummy, jobjectArray names, jobjectArray flags, jint count)
{
    char errmsg[128];

    jint num_flags, i, index;
    jmmVMGlobal* globals;
    const char* class_name;
    const char* signature;
    jobject source;
    jobject valueObj;
    jobject flag;
  
    if (flags == NULL) {
        JNU_ThrowNullPointerException(env, 0);
        return 0;
    }
  
    if (count == 0) {
        JNU_ThrowIllegalArgumentException(env, 0);
        return 0;
    }
  
    globals = (jmmVMGlobal*) malloc(count * sizeof(jmmVMGlobal));  
    if (globals == NULL) {
        JNU_ThrowOutOfMemoryError(env, 0);
        return 0;
    }

    num_flags = jmm_interface->GetVMGlobals(env, names, globals, count);
    if (num_flags == 0) {
        free(globals);
        return 0;
    }
  
    index = 0;
    for (i = 0; i < count; i++) {
        if (globals[i].name == NULL) {
            continue;
        }
        switch (globals[i].type) {
        case JMM_VMGLOBAL_TYPE_JBOOLEAN:
            class_name = "sun/management/BooleanFlag";
            signature = "(Ljava/lang/String;Ljava/lang/Boolean;ZLsun/management/Flag$FlagSource;)V";
            valueObj = JNU_NewObjectByName(env, "java/lang/Boolean", "(Z)V", 
                                           globals[i].value.z);
            break;
        case JMM_VMGLOBAL_TYPE_JSTRING:
            class_name = "sun/management/StringFlag";
            signature = "(Ljava/lang/String;Ljava/lang/String;ZLsun/management/Flag$FlagSource;)V";
            valueObj = globals[i].value.l;
            break;
        case JMM_VMGLOBAL_TYPE_JLONG:
            class_name = "sun/management/LongFlag";
            signature = "(Ljava/lang/String;Ljava/lang/Long;ZLsun/management/Flag$FlagSource;)V";
            valueObj = JNU_NewObjectByName(env, "java/lang/Long", "(J)V",
                                           globals[i].value.j);
            break;
        default:
            // unsupported type
            sprintf(errmsg, "Unsupported VMGlobal Type %d", globals[i].type);
            JNU_ThrowInternalError(env, errmsg); 
            free(globals);
            return 0;
        }
        switch (globals[i].source) {
        case JMM_VMGLOBAL_SOURCE_DEFAULT:
            source = vm_default_source;
            break;
        case JMM_VMGLOBAL_SOURCE_WASSET:
            source = other_source;
            break;
        default:
            // unknown source
            source = other_source;
            break;
        }
        flag = JNU_NewObjectByName(env, class_name, signature, globals[i].name, 
                                   valueObj, globals[i].writeable, source);
        if (flag == NULL) {
            free(globals);
            JNU_ThrowOutOfMemoryError(env, 0);
            return 0;
        }
        (*env)->SetObjectArrayElement(env, flags, index, flag);
        index++;
    }

    if (index != num_flags) {
        JNU_ThrowInternalError(env, "Number of Flag objects created unmatched");
        free(globals);
        return 0;
    }

    free(globals);

    /* return the number of Flag objects created */
    return num_flags;
}
