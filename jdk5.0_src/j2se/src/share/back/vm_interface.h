/*
 * @(#)vm_interface.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_VM_INTERFACE_H
#define JDWP_VM_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>
#include <jvm.h>
#include <jvmti.h>

#include "log_messages.h"

/* Macros that access interface functions */
#if !defined(lint)
    #define JVM_ENV_PTR(e,name)      (LOG_JVM(("%s()",#name)),  (e))
    #define JNI_ENV_PTR(e,name)      (LOG_JNI(("%s()",#name)),  (e))
    #define JVMTI_ENV_PTR(e,name)    (LOG_JVMTI(("%s()",#name)),(e))
#else
    #define JVM_ENV_PTR(e,name)      (e)
    #define JNI_ENV_PTR(e,name)      (e)
    #define JVMTI_ENV_PTR(e,name)    (e)
#endif

#define FUNC_PTR(e,name)       (*((*(e))->name))
#define JVM_FUNC_PTR(e,name)   FUNC_PTR(JVM_ENV_PTR  (e,name),name)
#define JNI_FUNC_PTR(e,name)   FUNC_PTR(JNI_ENV_PTR  (e,name),name)
#define JVMTI_FUNC_PTR(e,name) FUNC_PTR(JVMTI_ENV_PTR(e,name),name)

#endif

