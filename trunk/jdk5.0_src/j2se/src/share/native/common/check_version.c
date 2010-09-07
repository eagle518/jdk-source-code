/*
 * @(#)check_version.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint vm_version = JVM_GetInterfaceVersion();
    if (vm_version != JVM_INTERFACE_VERSION) {
        JNIEnv *env;
	char buf[128];
	sprintf(buf, "JVM interface version mismatch: expecting %d, got %d.",
		JVM_INTERFACE_VERSION, (int)vm_version);
	(*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_2);
	if (env) {
	    (*env)->FatalError(env, buf);
	}
    }
    return JNI_VERSION_1_2;
}
