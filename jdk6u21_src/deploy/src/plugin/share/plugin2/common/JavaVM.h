/*
 * @(#)JavaVM.h	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __JAVAVM_H_
#define __JAVAVM_H_

#include <jni.h>

// Helper routines for setting up the embedded, headless JVM which
// this version of the plugin uses in the browser process to mediate
// communications with the subordinate JVMs it launches.

#ifdef __cplusplus
extern "C"  {
#endif

// Gets the JNIEnv* associated with the Java VM, creating the JVM
// instance if necessary. Note that the implementation of this routine
// must be prepared for it to be called from more than one thread.
JNIEnv* JavaVM_GetJNIEnv();

// Detaches the current thread from the JVM. Note that it is still
// legal to call JavaVM_GetJNIEnv() again after calling this function,
// but doing so will re-allocate JVM-side resources.
void JavaVM_DetachCurrentThread();

#ifdef __cplusplus
}
#endif
#endif //__JAVAVM_H_
