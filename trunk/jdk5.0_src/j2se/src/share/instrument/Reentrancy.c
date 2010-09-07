/*
 * @(#)Reentrancy.c	1.2 03/08/19
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <jni.h>
#include    <jvmti.h>

#include    "JPLISAssert.h"
#include    "Reentrancy.h"
#include    "JPLISAgent.h"

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  This module provides some utility functions to support the "same thread" re-entrancy management.
 *  Uses JVMTI TLS to store a single bit per thread.
 *  Non-zero means the thread is already inside; zero means the thread is not inside.
 */

/*
 *  Local prototypes
 */

/* Wrapper around set that does the set then re-fetches to make sure it worked.
 * Degenerates to a simple set when assertions are disabled.
 * This routine is only here because of a bug in the JVMTI where set to 0 fails.
 */
jvmtiError
confirmingTLSSet(   jvmtiEnv *      jvmtienv,
                    jthread         thread,
                    const void *    newValue);

/* Confirmation routine only; used to assure that the TLS slot holds the value we expect it to. */
void
assertTLSValue( jvmtiEnv *      jvmtienv,
                jthread         thread,
                const void *    expected);


#define JPLIS_CURRENTLY_INSIDE_TOKEN                ((void *) 0x7EFFC0BB)
#define JPLIS_CURRENTLY_OUTSIDE_TOKEN               ((void *) 0)


jvmtiError
confirmingTLSSet(   jvmtiEnv *      jvmtienv,
                    jthread         thread,
                    const void *    newValue) {
    jvmtiError  error;

    error = (*jvmtienv)->SetThreadLocalStorage(
                                    jvmtienv,
                                    thread,
                                    newValue);

#if JPLISASSERT_ENABLEASSERTIONS
    assertTLSValue( jvmtienv,
                    thread,
                    newValue);
#endif

    return error;
}

void
assertTLSValue( jvmtiEnv *      jvmtienv,
                jthread         thread,
                const void *    expected) {
    jvmtiError  error;
    void *      test = (void *) 0x99999999;

    /* now check if we do a fetch we get what we wrote */
    error = (*jvmtienv)->GetThreadLocalStorage(
                                jvmtienv,
                                thread,
                                &test);
    jplis_assert(error == JVMTI_ERROR_NONE);
    jplis_assert(test == expected);
}

jboolean
tryToAcquireReentrancyToken(    jvmtiEnv *  jvmtienv,
                                jthread     thread) {
    jboolean    result      = JNI_FALSE;
    jvmtiError  error       = JVMTI_ERROR_NONE;
    void *      storedValue = NULL;
    
    error = (*jvmtienv)->GetThreadLocalStorage(
                                jvmtienv,
                                thread,
                                &storedValue);
    jplis_assert(error == JVMTI_ERROR_NONE);
    if ( error == JVMTI_ERROR_NONE ) {
        /* if this thread is already inside, just return false and short-circuit */
        if ( storedValue == JPLIS_CURRENTLY_INSIDE_TOKEN ) {
            result = JNI_FALSE;
        }
        else {
            /* stuff in the sentinel and return true */
#if JPLISASSERT_ENABLEASSERTIONS
            assertTLSValue( jvmtienv,
                            thread,
                            JPLIS_CURRENTLY_OUTSIDE_TOKEN);
#endif
            error = confirmingTLSSet (  jvmtienv,
                                        thread,
                                        JPLIS_CURRENTLY_INSIDE_TOKEN);
            jplis_assert(error == JVMTI_ERROR_NONE);
            if ( error != JVMTI_ERROR_NONE ) {
                result = JNI_FALSE;
            }
            else {
                result = JNI_TRUE;
            }
        }
    }
    return result;
}


void
releaseReentrancyToken(         jvmtiEnv *  jvmtienv,
                                jthread     thread)  {
    jvmtiError  error       = JVMTI_ERROR_NONE;

/* assert we hold the token */
#if JPLISASSERT_ENABLEASSERTIONS
    assertTLSValue( jvmtienv,
                    thread,
                    JPLIS_CURRENTLY_INSIDE_TOKEN);
#endif
    
    error = confirmingTLSSet(   jvmtienv,
                                thread,
                                JPLIS_CURRENTLY_OUTSIDE_TOKEN);
    jplis_assert(error == JVMTI_ERROR_NONE);

}

