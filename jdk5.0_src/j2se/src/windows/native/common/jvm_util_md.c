/*
 * @(#)jvm_util_md.c	1.2 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "jvm_util.h"

#ifdef DEBUG
#define JVM_DLL "jvm_g.dll"
#else
#define JVM_DLL "jvm.dll"
#endif

static HMODULE jvm_handle = NULL;

int initJvmHandle() {
    jvm_handle = GetModuleHandle(JVM_DLL);
    return (jvm_handle != NULL);
}

void* findJvmEntry(const char* name) {
    return (void*) GetProcAddress(jvm_handle, name);
}
