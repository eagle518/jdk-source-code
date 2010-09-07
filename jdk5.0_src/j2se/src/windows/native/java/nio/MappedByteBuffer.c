/*
 * @(#)MappedByteBuffer.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "java_nio_MappedByteBuffer.h"
#include <stdlib.h>

JNIEXPORT jboolean JNICALL 
Java_java_nio_MappedByteBuffer_isLoaded0(JNIEnv *env, jobject obj,
                                        jlong address, jlong len)
{
    jboolean loaded = JNI_FALSE;
    /* Information not available?
    MEMORY_BASIC_INFORMATION info;
    void *a = (void *) jlong_to_ptr(address);
    int result = VirtualQuery(a, &info, (DWORD)len);
    */
    return loaded;
}

JNIEXPORT jint JNICALL
Java_java_nio_MappedByteBuffer_load0(JNIEnv *env, jobject obj, jlong address,
                                     jlong len, jint pageSize)
{
    int *ptr = (int *) jlong_to_ptr(address);
    int pageIncrement = pageSize / sizeof(int);
    jlong numPages = (len + pageSize - 1) / pageSize;
    int i = 0;
    int j = 0;

    /* touch every page */
    for (i=0; i<numPages; i++) {
        j += *((volatile int *)ptr);
        ptr += pageIncrement;
    }
    return j;
}

JNIEXPORT void JNICALL
Java_java_nio_MappedByteBuffer_force0(JNIEnv *env, jobject obj, jlong address,
                                      jlong len)
{
    void *a = (void *) jlong_to_ptr(address);
    int result = FlushViewOfFile(a, (DWORD)len);
    if (result == 0) {
        JNU_ThrowByName(env, "java/io/IOException", 0);
    }
}
