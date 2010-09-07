/*
 * @(#)SecurityManager.c	1.49 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

#include "java_lang_SecurityManager.h"
#include "java_lang_ClassLoader.h"
#include "java_util_ResourceBundle.h"

/*
 * Make sure a security manager instance is initialized.
 * TRUE means OK, FALSE means not.
 */
static jboolean
check(JNIEnv *env, jobject this)
{
    static jfieldID initField = 0;
    jboolean initialized = JNI_FALSE;

    if (initField == 0) {
        jclass clazz = (*env)->FindClass(env, "java/lang/SecurityManager");
	if (clazz == 0) {
	    (*env)->ExceptionClear(env);
	    return JNI_FALSE;
	}
	initField = (*env)->GetFieldID(env, clazz, "initialized", "Z");
	if (initField == 0) {
	    (*env)->ExceptionClear(env);
	    return JNI_FALSE;
	}	    
    }
    initialized = (*env)->GetBooleanField(env, this, initField);
    
    if (initialized == JNI_TRUE) {
	return JNI_TRUE;
    } else {
	jclass securityException =
	    (*env)->FindClass(env, "java/lang/SecurityException");
	if (securityException != 0) {
	    (*env)->ThrowNew(env, securityException, 
			     "security manager not initialized.");
	}
	return JNI_FALSE;
    }
}

JNIEXPORT jobjectArray JNICALL
Java_java_lang_SecurityManager_getClassContext(JNIEnv *env, jobject this)
{
    if (!check(env, this)) {
	return NULL;		/* exception */
    }

    return JVM_GetClassContext(env);
}

JNIEXPORT jclass JNICALL
Java_java_lang_SecurityManager_currentLoadedClass0(JNIEnv *env, jobject this)
{
    /* Make sure the security manager instance is initialized */
    if (!check(env, this)) {
	return NULL;		/* exception */
    }

    return JVM_CurrentLoadedClass(env);
}

JNIEXPORT jobject JNICALL
Java_java_lang_SecurityManager_currentClassLoader0(JNIEnv *env, jobject this)
{
    /* Make sure the security manager instance is initialized */
    if (!check(env, this)) {
	return NULL;		/* exception */
    }

    return JVM_CurrentClassLoader(env);
}

JNIEXPORT jint JNICALL
Java_java_lang_SecurityManager_classDepth(JNIEnv *env, jobject this,
					  jstring name)
{
    /* Make sure the security manager instance is initialized */
    if (!check(env, this)) {
	return -1;		/* exception */
    }

    if (name == NULL) {
      JNU_ThrowNullPointerException(env, 0);
      return -1;
    }

    return JVM_ClassDepth(env, name);
}

JNIEXPORT jint JNICALL
Java_java_lang_SecurityManager_classLoaderDepth0(JNIEnv *env, jobject this)
{
    /* Make sure the security manager instance is initialized */
    if (!check(env, this)) {
	return -1;		/* exception */
    }

    return JVM_ClassLoaderDepth(env);
}
