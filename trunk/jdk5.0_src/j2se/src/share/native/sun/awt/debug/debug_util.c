/*
 * @(#)debug_util.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
