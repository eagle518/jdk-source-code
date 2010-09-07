/*
 * @(#)monitor_md.h	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Win32 implementation of Java monitors
 */

#ifndef _JAVASOFT_WIN32_MONITOR_MD_H_
#define _JAVASOFT_WIN32_MONITOR_MD_H_

#include <windows.h>

#include "threads_md.h"
#include "mutex_md.h"

#define SYS_MID_NULL ((sys_mon_t *) 0)

typedef struct sys_mon {
    long            atomic_count;   /* Variable for atomic compare swap */
    HANDLE          semaphore;      /* Semaphore used for the contention */
    sys_thread_t   *monitor_owner;  /* Current owner of this monitor */
    long            entry_count;    /* Recursion depth */
    sys_thread_t   *monitor_waiter; /* Monitor waiting queue head */
    long            waiter_count;   /* For debugging purpose */
} sys_mon_t;

#endif /* !_JAVASOFT_WIN32_MONITOR_MD_H_ */
