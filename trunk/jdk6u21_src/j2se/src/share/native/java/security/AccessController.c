/*
 * @(#)AccessController.c	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *      Implementation of class java.security.AccessController
 *
 */

#include <string.h>

#include "jni.h"
#include "jvm.h"
#include "java_security_AccessController.h"

/*
 * Class:     java_security_AccessController
 * Method:    doPrivileged
 * Signature: (Ljava/security/PrivilegedAction;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_security_AccessController_doPrivileged__Ljava_security_PrivilegedAction_2
  (JNIEnv *env, jclass cls, jobject action)
{
    return JVM_DoPrivileged(env, cls, action, NULL, JNI_FALSE);
}

/*
 * Class:     java_security_AccessController
 * Method:    doPrivileged
 * Signature: (Ljava/security/PrivilegedAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_security_AccessController_doPrivileged__Ljava_security_PrivilegedAction_2Ljava_security_AccessControlContext_2
  (JNIEnv *env, jclass cls, jobject action, jobject context)
{
    return JVM_DoPrivileged(env, cls, action, context, JNI_FALSE);
}

/*
 * Class:     java_security_AccessController
 * Method:    doPrivileged
 * Signature: (Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_security_AccessController_doPrivileged__Ljava_security_PrivilegedExceptionAction_2
  (JNIEnv *env, jclass cls, jobject action)
{
    return JVM_DoPrivileged(env, cls, action, NULL, JNI_TRUE);
}

/*
 * Class:     java_security_AccessController
 * Method:    doPrivileged
 * Signature: (Ljava/security/PrivilegedExceptionAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_security_AccessController_doPrivileged__Ljava_security_PrivilegedExceptionAction_2Ljava_security_AccessControlContext_2
  (JNIEnv *env, jclass cls, jobject action, jobject context)
{
    return JVM_DoPrivileged(env, cls, action, context, JNI_TRUE);
}

JNIEXPORT jobject JNICALL
Java_java_security_AccessController_getStackAccessControlContext(
							      JNIEnv *env,
							      jobject this)
{
    return JVM_GetStackAccessControlContext(env, this);
}


JNIEXPORT jobject JNICALL
Java_java_security_AccessController_getInheritedAccessControlContext(
							      JNIEnv *env,
							      jobject this)
{
    return JVM_GetInheritedAccessControlContext(env, this);
}
