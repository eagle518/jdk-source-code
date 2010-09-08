/*
 * @(#)CRC32.c	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Native method support for java.util.zip.CRC32
 */

#include "jni.h"
#include "jni_util.h"
#include "zlib.h"

#include "java_util_zip_CRC32.h"

JNIEXPORT jint JNICALL
Java_java_util_zip_CRC32_update(JNIEnv *env, jclass cls, jint crc, jint b)
{
    Bytef buf[1];

    buf[0] = (Bytef)b;
    return crc32(crc, buf, 1);
}

JNIEXPORT jint JNICALL
Java_java_util_zip_CRC32_updateBytes(JNIEnv *env, jclass cls, jint crc,
				     jarray b, jint off, jint len)
{
    Bytef *buf = (*env)->GetPrimitiveArrayCritical(env, b, 0);
    if (buf) {
        crc = crc32(crc, buf + off, len);
	(*env)->ReleasePrimitiveArrayCritical(env, b, buf, 0);
    }
    return crc;
}

JNIEXPORT jint ZIP_CRC32(jint crc, const jbyte *buf, jint len)
{
    return crc32(crc, (Bytef*)buf, len);
}
