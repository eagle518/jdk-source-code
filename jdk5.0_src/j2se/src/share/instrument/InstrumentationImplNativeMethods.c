/*
 * @(#)InstrumentationImplNativeMethods.c	1.3 04/06/08
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
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
 * in the InstrumentationImpl instance.
 */
 
 
/*
 * local forward definitions
 */

/*
 *  Local helper to fetch the stored native pointer out of the java object.
 *  Returns null if anything goes wrong.
 */
JPLISAgent *
getJPLISAgentFromJavaImpl(JNIEnv * jnienv, jobject implThis);


/*
 * internal implementations
 */

JPLISAgent *
getJPLISAgentFromJavaImpl(JNIEnv * jnienv, jobject implThis) {
    JPLISAgent *    resultAgent             = NULL;
    jboolean        errorOutstanding        = JNI_FALSE;
    jclass          classHandle             = NULL;
    jmethodID       fetchMethodID           = NULL;
    
    classHandle = (*jnienv)->GetObjectClass(    jnienv,
                                                implThis);
    errorOutstanding = checkForAndClearThrowable(jnienv);
    jplis_assert_msg(!errorOutstanding, "can't get InstrumentationImpl class");
    
    if ( !errorOutstanding ) {
        fetchMethodID = (*jnienv)->GetMethodID( jnienv,
                                                classHandle,
                                                JPLIS_INSTRUMENTIMPL_GETNATIVEAGENT_METHODNAME,
                                                JPLIS_INSTRUMENTIMPL_GETNATIVEAGENT_METHODSIGNATURE);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        jplis_assert_msg(!errorOutstanding, "can't get native agent accessor methodID");
    }
    
    if ( !errorOutstanding ) {
        jlong peerReferenceAsScalar = (*jnienv)->CallLongMethod(jnienv,
                                                                implThis,
                                                                fetchMethodID);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        jplis_assert_msg(!errorOutstanding, "failed in call to native agent accessor");
        
        if ( !errorOutstanding ) {
            resultAgent = (JPLISAgent *) peerReferenceAsScalar;
        }
        
    }

    return resultAgent;
}

/*
 * Native methods
 */

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    redefineClasses0
 * Signature: ([Ljava/lang/instrument/ClassDefinition;)V
 */
JNIEXPORT void JNICALL Java_sun_instrument_InstrumentationImpl_redefineClasses0
  (JNIEnv * jnienv, jobject implThis, jobjectArray classDefinitions) {
    JPLISAgent *    agent = getJPLISAgentFromJavaImpl(jnienv, implThis);
    if ( agent == NULL ) {
        createAndThrowInternalError(jnienv);
    }
    else {
        redefineClasses(jnienv, agent, classDefinitions);
    }
    return;
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getAllLoadedClasses0
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_sun_instrument_InstrumentationImpl_getAllLoadedClasses0
  (JNIEnv * jnienv, jobject implThis) {
    jobjectArray    result = NULL;
    JPLISAgent *    agent = getJPLISAgentFromJavaImpl(jnienv, implThis);
    if ( agent == NULL ) {
        createAndThrowInternalError(jnienv);
    }
    else {
        result = getAllLoadedClasses(jnienv, agent);
    }
    return result;
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getInitiatedClasses0
 * Signature: (Ljava/lang/ClassLoader;)[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_sun_instrument_InstrumentationImpl_getInitiatedClasses0
  (JNIEnv * jnienv, jobject implThis, jobject classLoader) {
    jobjectArray    result = NULL;
    JPLISAgent *    agent = getJPLISAgentFromJavaImpl(jnienv, implThis);
    if ( agent == NULL ) {
        createAndThrowInternalError(jnienv);
    }
    else {
        result = getInitiatedClasses(jnienv, agent, classLoader);
    }
    return result;
}

/*
 * Class:     sun_instrument_InstrumentationImpl
 * Method:    getObjectSize0
 * Signature: (Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_sun_instrument_InstrumentationImpl_getObjectSize0
  (JNIEnv * jnienv, jobject implThis, jobject objectToSize) {
    jlong           result = 0;
    JPLISAgent *    agent = getJPLISAgentFromJavaImpl(jnienv, implThis);
    if ( agent == NULL ) {
        createAndThrowInternalError(jnienv);
    }
    else {
        result = getObjectSize(jnienv, agent, objectToSize);
    }
    return result;
}
