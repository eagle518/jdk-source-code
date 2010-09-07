/*
 * @(#)debug_util.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(_DEBUG_UTIL_H)
#define _DEBUG_UTIL_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef int dbool_t;

#if !defined(TRUE)
#define TRUE 1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif

typedef void * dmutex_t;

#include "jvm.h"
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <limits.h>

/* keep these after the other headers */
#include "debug_mem.h"
#include "debug_assert.h"
#include "debug_trace.h"

#if defined(DEBUG)

/* Mutex object mainly for internal debug code use only */
dmutex_t DMutex_Create();
void DMutex_Destroy(dmutex_t);
void DMutex_Enter(dmutex_t);
void DMutex_Exit(dmutex_t);

#endif /* DEBUG */

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* defined(_DEBUG_UTIL_H) */
