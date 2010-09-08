/*
 * @(#)jdk_util_md.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <dlfcn.h>
#include "jdk_util.h"

int JDK_InitJvmHandle() {
    /* nop */
    return 1;
}

void* JDK_FindJvmEntry(const char* name) {
    return dlsym(RTLD_DEFAULT, name);
}
