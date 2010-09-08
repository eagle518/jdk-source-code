/*
 * @(#)InstrumentationImplNativeMethods.c	1.11 10/03/23
 *
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <jni.h>

#include    "JPLISAgent.h"
#include    "JPLISAssert.h"
#include    "Utilities.h"
#include    "JavaExceptions.h"
#include    "sun_instrument_InstrumentationImpl.h"

/*
 * Copyright 2003 Wily Technology, Inc.
 */
 
/**
 * This module contains the native method implementations to back the
 * sun.instrument.InstrumentationImpl class.
 * The bridge between Java and native code is built by storing a native
 * pointer to the JPLISAgent data structure in a 64 bit scalar field
 * in the InstrumentationImpl instance which is passed to each method.
 */
 

/*
 * Native methods
 */

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    isModifiableClass0
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_instrument_InstrumentationImpl_isModifiableClass0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jclass clazz) {
    return isModifiableClass(jnienv, (JPLISAgent*)agent, clazz);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    isRetransformClassesSupported0
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_instrument_InstrumentationImpl_isRetransformClassesSupported0
  (JNIEnv * jnienv, jobject implThis, jlong agent) {
    return isRetransformClassesSupported(jnienv, (JPLISAgent*)agent);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    setHasRetransformableTransformers
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL 
Java_sun_instrument_InstrumentationImpl_setHasRetransformableTransformers
  (JNIEnv * jnienv, jobject implThis, jlong agent, jboolean has) {
    setHasRetransformableTransformers(jnienv, (JPLISAgent*)agent, has);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    retransformClasses0
 * Signature: ([Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL 
Java_sun_instrument_InstrumentationImpl_retransformClasses0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jobjectArray classes) {
    retransformClasses(jnienv, (JPLISAgent*)agent, classes);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    redefineClasses0
 * Signature: ([Ljava/lang/instrument/ClassDefinition;)V
 */
JNIEXPORT void JNICALL Java_sun_instrument_InstrumentationImpl_redefineClasses0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jobjectArray classDefinitions) {
    redefineClasses(jnienv, (JPLISAgent*)agent, classDefinitions);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getAllLoadedClasses0
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_sun_instrument_InstrumentationImpl_getAllLoadedClasses0
  (JNIEnv * jnienv, jobject implThis, jlong agent) {
    return getAllLoadedClasses(jnienv, (JPLISAgent*)agent);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getInitiatedClasses0
 * Signature: (Ljava/lang/ClassLoader;)[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_sun_instrument_InstrumentationImpl_getInitiatedClasses0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jobject classLoader) {
    return getInitiatedClasses(jnienv, (JPLISAgent*)agent, classLoader);
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getObjectSize0
 * Signature: (Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_sun_instrument_InstrumentationImpl_getObjectSize0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jobject objectToSize) {
    return getObjectSize(jnienv, (JPLISAgent*)agent, objectToSize);
}


/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    appendToClassLoaderSearch0
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_sun_instrument_InstrumentationImpl_appendToClassLoaderSearch0
  (JNIEnv * jnienv, jobject implThis, jlong agent, jstring jarFile, jboolean isBootLoader) {
    appendToClassLoaderSearch(jnienv, (JPLISAgent*)agent, jarFile, isBootLoader);
}


/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    setNativeMethodPrefixes
 * Signature: ([Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_sun_instrument_InstrumentationImpl_setNativeMethodPrefixes
  (JNIEnv * jnienv, jobject implThis, jlong agent, jobjectArray prefixArray, jboolean isRetransformable) {
    setNativeMethodPrefixes(jnienv, (JPLISAgent*)agent, prefixArray, isRetransformable);
}

