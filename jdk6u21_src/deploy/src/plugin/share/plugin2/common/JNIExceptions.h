/*
 * @(#)JNIExceptions.h	1.2 10/03/24 12:03:38
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"

// Helper macros to make it easier and more standard to check for and clear JNI exceptions

// Clear any pending JNI exception without affecting control flow
#define CLEAR_EXCEPTION(env)                                                 \
    do { if (env->ExceptionOccurred() != NULL) {                             \
             env->ExceptionDescribe(); env->ExceptionClear();                \
    } } while (0)

// Check for a JNI exception; if one occurred, clear it and return
#define CHECK_EXCEPTION(env)                                                 \
    do { if (env->ExceptionOccurred() != NULL) {                             \
             env->ExceptionDescribe(); env->ExceptionClear(); return;        \
    } } while (0) 

// Check for a JNI exception; if one occurred, clear it and return the given value
#define CHECK_EXCEPTION_VAL(env, val)                                            \
    do { if (env->ExceptionOccurred() != NULL)                                   \
            { env->ExceptionDescribe(); env->ExceptionClear(); return val; } }   \
    while (0)
