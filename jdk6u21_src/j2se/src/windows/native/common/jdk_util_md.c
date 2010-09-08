/*
 * @(#)jdk_util_md.c	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "jdk_util.h"

#define JVM_DLL "jvm.dll"

static HMODULE jvm_handle = NULL;

int JDK_InitJvmHandle() {
    jvm_handle = GetModuleHandle(JVM_DLL);
    return (jvm_handle != NULL);
}

void* JDK_FindJvmEntry(const char* name) {
    return (void*) GetProcAddress(jvm_handle, name);
}
