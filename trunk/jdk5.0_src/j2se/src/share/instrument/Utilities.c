/*
 * @(#)Utilities.c	1.3 03/08/23
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <stdlib.h>
#include    <stdio.h>

#include    "JPLISAssert.h"
#include    "Utilities.h"
#include    "JavaExceptions.h"

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  This module provides various simple JNI and JVMTI utility functionality.
 */

void *
allocate(jvmtiEnv * jvmtienv, size_t bytecount) {
    void *          resultBuffer    = NULL;
    jvmtiError      error           = JVMTI_ERROR_NONE;

    error = (*jvmtienv)->Allocate(jvmtienv, 
                                  bytecount, 
                                  (unsigned char**) &resultBuffer);
    jplis_assert(error == JVMTI_ERROR_NONE);
    if ( error != JVMTI_ERROR_NONE ) {
        resultBuffer = NULL;
    }
    return resultBuffer;
}

/**
 * Convenience method that deallocates memory.
 * Throws assert on error.
 * JVMTI Deallocate can only fail due to internal error, that is, this
 * agent has done something wrong or JVMTI has done something wrong.  These
 * errors aren't interesting to a JPLIS agent and so are not returned.
 */
void
deallocate(jvmtiEnv * jvmtienv, void * buffer) {
    jvmtiError  error = JVMTI_ERROR_NONE;
    
    error = (*jvmtienv)->Deallocate(jvmtienv,
                                    (unsigned char*)buffer);
    jplis_assert_msg(error == JVMTI_ERROR_NONE, "Can't deallocate memory");
    return;
}

/**
 *  Returns whether the passed exception is an instance of the given classname
 *  Clears any JNI exceptions before returning
 */
jboolean
isInstanceofClassName(  JNIEnv *        jnienv,
                        jobject         instance,
                        const char *    className) {
    jboolean    isInstanceof        = JNI_FALSE;
    jboolean    errorOutstanding    = JNI_FALSE;
    jclass      classHandle         = NULL;
    
    jplis_assert(isSafeForJNICalls(jnienv));
    
    /* get an instance of unchecked exception for instanceof comparison */
    classHandle = (*jnienv)->FindClass(jnienv, className);
    errorOutstanding = checkForAndClearThrowable(jnienv);
    jplis_assert(!errorOutstanding);

    if (!errorOutstanding) {
        isInstanceof = (*jnienv)->IsInstanceOf(jnienv, instance, classHandle);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        jplis_assert(!errorOutstanding);
    }
    
    jplis_assert(isSafeForJNICalls(jnienv));
    return isInstanceof;
}

/* We don't come back from this
*/
void
abortJVM(   JNIEnv *        jnienv,
            const char *    message) {
    (*jnienv)->FatalError(jnienv, message);
}

