/*
 * @(#)jvm_util.h	1.2 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JVM_UTIL_H
#define JVM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Init JVM handle for symbol lookup;
 * Return 0 if JVM handle not found.
 */
int initJvmHandle();

/* Find the named JVM entry; returns NULL if not found. */
void* findJvmEntry(const char* name);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* JVM_UTIL_H */

