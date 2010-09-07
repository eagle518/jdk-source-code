/*
 * @(#)Reentrancy.h	1.1 03/08/16
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#ifndef _REENTRANCY_H_
#define _REENTRANCY_H_

#include    <jni.h>

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  This module provides some utility functions to support the "same thread" re-entrancy management.
 *  Uses JVMTI TLS to store a single bit per thread.
 *  Non-zero means the thread is already inside; zero means the thread is not inside.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* returns true if the token is acquired by this call,
 * false if we already hold it and do not have to acquire it
 */
extern jboolean
tryToAcquireReentrancyToken(    jvmtiEnv *  jvmtienv,
                                jthread     thread);

/* release the token; assumes we already hold it */                         
extern void
releaseReentrancyToken(         jvmtiEnv *  jvmtienv,
                                jthread     thread);    


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif

