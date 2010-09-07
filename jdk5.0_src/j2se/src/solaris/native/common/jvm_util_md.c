/*
 * @(#)jvm_util_md.c	1.2 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <dlfcn.h>
#include "jvm_util.h"

int initJvmHandle() {
    /* nop */
    return 1;
}

void* findJvmEntry(const char* name) {
    return dlsym(RTLD_DEFAULT, name);
}
