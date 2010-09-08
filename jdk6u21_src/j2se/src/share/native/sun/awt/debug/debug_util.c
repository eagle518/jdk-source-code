/*
 * @(#)debug_util.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "debug_util.h"

#if defined(DEBUG)

dmutex_t DMutex_Create() {
    dmutex_t	mutex;
    mutex = (dmutex_t)JVM_RawMonitorCreate();
    DASSERT(mutex != NULL);

    return mutex;
}

void DMutex_Destroy(dmutex_t mutex) {
    if (mutex != NULL) {
	JVM_RawMonitorDestroy(mutex);
    }
}

void DMutex_Enter(dmutex_t mutex) {
    if (mutex != NULL) {
	JVM_RawMonitorEnter(mutex);
    }
}

void DMutex_Exit(dmutex_t mutex) {
    if (mutex != NULL) {
	JVM_RawMonitorExit(mutex);
    }
}

#endif
