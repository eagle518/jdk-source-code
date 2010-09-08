/*
 * @(#)MappedByteBuffer.c	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "java_nio_MappedByteBuffer.h"
#include <sys/mman.h>
#include <stddef.h>
#include <stdlib.h>

JNIEXPORT jboolean JNICALL
Java_java_nio_MappedByteBuffer_isLoaded0(JNIEnv *env, jobject obj, jlong address,
                                         jlong len, jint pageSize)
{
    jboolean loaded = JNI_TRUE;
    jint numPages = (len + pageSize - 1) / pageSize;
    int result = 0;
    int i = 0;
    void *a = (void *) jlong_to_ptr(address);
    char * vec = (char *)malloc(numPages * sizeof(char));

    if (vec == NULL) {
	JNU_ThrowOutOfMemoryError(env, NULL);
	return JNI_FALSE;
    }

    result = mincore(a, (size_t)len, vec);
    if (result == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "mincore failed");
        free(vec);
        return JNI_FALSE;
    }
    
    for (i=0; i<numPages; i++) {
        if (vec[i] == 0) {
            loaded = JNI_FALSE;
            break;
        }
    }
    free(vec);
    return loaded;
}


JNIEXPORT jint JNICALL
Java_java_nio_MappedByteBuffer_load0(JNIEnv *env, jobject obj, jlong address,
                                     jlong len, jint pageSize)
{
    int numPages = (len + pageSize - 1) / pageSize;
    char *a = (char *)jlong_to_ptr(address);
    int i = 0;
    int j = 0;
    int result = madvise((caddr_t)a, (size_t)len, MADV_WILLNEED);
    if (result == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "madvise failed");
        return j;
    }

    /* touch every page */
    for (i=0; i<numPages; i++) {
        j += a[0];
        a += pageSize;
    }
    return j;
}


JNIEXPORT void JNICALL
Java_java_nio_MappedByteBuffer_force0(JNIEnv *env, jobject obj, jlong address,
                                      jlong len)
{
    void* a = (void *)jlong_to_ptr(address);
    int result = msync(a, (size_t)len, MS_SYNC);
    if (result == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "msync failed");
    }
}
