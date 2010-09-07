/*
 * @(#)Utilities.h	1.2 03/08/23
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include    <jni.h>
#include    <jvmti.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/*
 *  This module provides various simple JNI and JVMTI utility functionality.
 */



/*
 *  This allocate must be paired with this deallocate. Used for our own working buffers.
 *  Implementation may vary.
 */
extern void *
allocate(jvmtiEnv * jvmtienv, size_t bytecount);

extern void
deallocate(jvmtiEnv * jvmtienv, void * buffer);


/*
 *  Misc. JNI support
 */
/* convenience wrapper around JNI instanceOf */
extern jboolean
isInstanceofClassName(  JNIEnv*     jnienv,
                        jobject     instance,
                        const char* className);


/* calling this stops the JVM and does not return */                
extern void
abortJVM(   JNIEnv *        jnienv,
            const char *    message);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif

