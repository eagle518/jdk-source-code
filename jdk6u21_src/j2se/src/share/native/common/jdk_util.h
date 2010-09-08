/*
 * @(#)jdk_util.h	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDK_UTIL_H
#define JDK_UTIL_H

#include "jni.h"
#include "jvm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------
 * Exported interfaces for both JDK and JVM to use
 *-------------------------------------------------------
 */

/*
 *
 */
JNIEXPORT void 
JDK_GetVersionInfo0(jdk_version_info* info, size_t info_size);


/*-------------------------------------------------------
 * Internal interface for JDK to use
 *-------------------------------------------------------
 */

/* Init JVM handle for symbol lookup;
 * Return 0 if JVM handle not found.
 */
int JDK_InitJvmHandle();

/* Find the named JVM entry; returns NULL if not found. */
void* JDK_FindJvmEntry(const char* name);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* JDK_UTIL_H */

