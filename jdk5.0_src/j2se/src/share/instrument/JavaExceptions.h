/*
 * @(#)JavaExceptions.h	1.1 03/08/16
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#ifndef _JAVAEXCEPTIONS_H_
#define _JAVAEXCEPTIONS_H_

#include    <jni.h>
#include    <jvmti.h>

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/**
 * This module contains utility routines for manipulating Java throwables
 * and JNIEnv throwable state from native code.
 */ 


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set up static state. Needs java, must be called at or after VMInit.
 * Returns true if it succeeds, false if it fails.
 */
extern jboolean
initializeFallbackError(JNIEnv* jnienv);
 
/*
 *  Mapping support. Allows different clients to map checked exceptions in different ways.
 */
typedef jthrowable (*CheckedExceptionMapper)
    (   JNIEnv *    jnienv,
        jthrowable  throwableToMap);

/* Default mapper. Map everything checked to InternalError; can return null if error */ 
extern jthrowable
mapAllCheckedToInternalErrorMapper( JNIEnv *    jnienv,
                                    jthrowable  throwableToMap);



/*
 *  Exception-helper routines that do not modify the JNIEnv.
 *  They require a clean JNIEnv on entry, and they guarantee a clean JNIEnv on exit.
 */

/* creates a throwable from the supplied parameters; can return null if error */
extern jthrowable
createThrowable(    JNIEnv*     jnienv,
                    const char* className,
                    jstring     message);

/* creates a java.lang.InternalError; can return null if error */
extern jthrowable
createInternalError(JNIEnv * jnienv, jstring message);

/* creates the appropriate java Throwable based on the error code; can return null if error */      
extern jthrowable
createThrowableFromJVMTIErrorCode(JNIEnv * jnienv, jvmtiError errorCode);

/* fetches the message string out of the supplied throwable, null if there is none, null if error   */      
extern jstring
getMessageFromThrowable(    JNIEnv*     jnienv,
                            jthrowable  exception);

/* true if the supplied throwable is unchecked. null will return true.  */      
extern jboolean
isUnchecked(    JNIEnv*     jnienv,
                jthrowable  exception);

/* true if the env contains a thrown exception */       
extern jboolean
checkForThrowable(  JNIEnv*     jnienv);

/* true if the env is clean for JNI calls */        
extern jboolean
isSafeForJNICalls(  JNIEnv * jnienv);

/* 
 * Logs the outstanding throwable, if one exists.
 * This call assumes an outstanding exception, but does not
 * modify the JNIEnv outstanding Throwable state.
 */
extern void
logThrowable(   JNIEnv * jnienv);


/*
 *  These routines do modify the JNIEnv outstanding Throwable state.
 */

/* Throws the supplied throwable. always sets the JNIEnv throwable */
extern void
throwThrowable(     JNIEnv *    jnienv,
                    jthrowable  exception);

/* returns current throwable. always clears the JNIEnv exception */
extern jthrowable
preserveThrowable(JNIEnv * jnienv);

/* undoes preserveThrowable (Throws the supplied throwable). always sets the JNIEnv throwable */
extern void
restoreThrowable(   JNIEnv *    jnienv,
                    jthrowable  preservedException);
                    
/* always clears the JNIEnv throwable. returns true if an exception was pending on entry. */
extern jboolean
checkForAndClearThrowable(  JNIEnv *    jnienv);

/* creates the appropriate java Throwable based on the error code   
 * does the very best it can to make sure an exception ends up installed; uses fallback if necessary            
 * always sets the JNIEnv exception
 */     
extern void
createAndThrowThrowableFromJVMTIErrorCode(JNIEnv * jnienv, jvmtiError errorCode);

/* creates a java.lang.InternalError and installs it into the JNIEnv.
 * does the very best it can to make sure an exception ends up installed; uses fallback if necessary            
 * always sets the JNIEnv exception
 */
extern void
createAndThrowInternalError(JNIEnv * jnienv);

/* If no throwable is outstanding, do nothing.
 * If a throwable is outstanding, make sure it is of a legal type according to the supplied
 * mapping function.
 * Leaves the "thrown" state the same (none on exit if none on entry, thrown on exit if 
 * thrown on entry); may change the type of the thrown exception.
 */
extern void
mapThrownThrowableIfNecessary(JNIEnv * jnienv, CheckedExceptionMapper mapper);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif

