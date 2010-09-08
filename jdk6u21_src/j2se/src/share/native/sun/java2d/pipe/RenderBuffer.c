/*
 * @(#)RenderBuffer.c	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include <string.h>

#include "sun_java2d_pipe_RenderBuffer.h"

/**
 * Note: The code in this file is nearly identical to that in
 *       java/nio/Bits.c...
 */

#define MBYTE 1048576

JNIEXPORT void JNICALL
Java_sun_java2d_pipe_RenderBuffer_copyFromArray
    (JNIEnv *env, jclass rb,
     jobject srcArray, jlong srcPos, jlong dstAddr, jlong length)
{
    jbyte *bytes;
    size_t size;

    while (length > 0) {
        /*
         * Copy no more than one megabyte at a time, to allow for GC.
         * (Probably not an issue for STR, since our buffer size is likely
         * much smaller than a megabyte, but just in case...)
         */
	size = (size_t)(length > MBYTE ? MBYTE : length);

        bytes = (*env)->GetPrimitiveArrayCritical(env, srcArray, NULL);
        if (bytes == NULL) {
            JNU_ThrowInternalError(env, "Unable to get array");
            return;
        }

 	memcpy(jlong_to_ptr(dstAddr), bytes + srcPos, size);

        (*env)->ReleasePrimitiveArrayCritical(env, srcArray,
                                              bytes, JNI_ABORT);

	length -= size;
	dstAddr += size;
	srcPos += size;
    }
}
