/*
 * @(#)ObjectStreamClass.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"

#include "java_io_ObjectStreamClass.h"

static jclass noSuchMethodErrCl;

/*
 * Class:     java_io_ObjectStreamClass
 * Method:    initNative
 * Signature: ()V
 * 
 * Native code initialization hook.
 */
JNIEXPORT void JNICALL 
Java_java_io_ObjectStreamClass_initNative(JNIEnv *env, jclass this)
{
    jclass cl = (*env)->FindClass(env, "java/lang/NoSuchMethodError");
    if (cl == NULL) {		/* exception thrown */
	return;
    }
    noSuchMethodErrCl = (*env)->NewGlobalRef(env, cl);
}

/*
 * Class:     java_io_ObjectStreamClass
 * Method:    hasStaticInitializer
 * Signature: (Ljava/lang/Class;)Z
 * 
 * Returns true if the given class defines a <clinit>()V method; returns false
 * otherwise.
 */
JNIEXPORT jboolean JNICALL 
Java_java_io_ObjectStreamClass_hasStaticInitializer(JNIEnv *env, jclass this, 
						    jclass clazz)
{
    jclass superCl = NULL;
    jmethodID superClinitId = NULL;
    jmethodID clinitId =
	(*env)->GetStaticMethodID(env, clazz, "<clinit>", "()V");
    if (clinitId == NULL) {	/* error thrown */
	jthrowable th = (*env)->ExceptionOccurred(env);
	(*env)->ExceptionClear(env);	/* normal return */
	if (!(*env)->IsInstanceOf(env, th, noSuchMethodErrCl)) {
	    (*env)->Throw(env, th);
	}
	return JNI_FALSE;
    }
    
    /*
     * Check superclass for static initializer as well--if the same method ID
     * is returned, then the static initializer is from a superclass.
     * Empirically, this step appears to be unnecessary in 1.4; however, the
     * JNI spec makes no guarantee that GetStaticMethodID will not return the
     * ID for a superclass initializer.
     */
    
    if ((superCl = (*env)->GetSuperclass(env, clazz)) == NULL) {
	return JNI_TRUE;
    }
    superClinitId =
	(*env)->GetStaticMethodID(env, superCl, "<clinit>", "()V");
    if (superClinitId == NULL) {	/* error thrown */
	jthrowable th = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env);	/* normal return */
	if (!(*env)->IsInstanceOf(env, th, noSuchMethodErrCl)) {
	    (*env)->Throw(env, th);
	}
	return JNI_TRUE;
    }
    
    return (clinitId != superClinitId);
}
