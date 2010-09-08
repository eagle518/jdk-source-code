/*
 * @(#)FileKey.c	1.2 10/03/23
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "nio.h"
#include "nio_util.h"
#include "sun_nio_ch_FileKey.h"

static jfieldID key_st_dev;    /* id for FileKey.st_dev */
static jfieldID key_st_ino;    /* id for FileKey.st_ino */

#define RESTARTABLE(_cmd, _result) do { \
  do { \
    _result = _cmd; \
  } while ((_result == -1) && (errno == EINTR)); \
} while(0)


JNIEXPORT void JNICALL 
Java_sun_nio_ch_FileKey_initIDs(JNIEnv *env, jclass clazz)
{
    key_st_dev = (*env)->GetFieldID(env, clazz, "st_dev", "J");
    key_st_ino = (*env)->GetFieldID(env, clazz, "st_ino", "J");
}


JNIEXPORT void JNICALL 
Java_sun_nio_ch_FileKey_init(JNIEnv *env, jobject this, jobject fdo)
{
    struct stat64 fbuf;
    int res;

    RESTARTABLE(fstat64(fdval(env, fdo), &fbuf), res);
    if (res < 0) {
        JNU_ThrowIOExceptionWithLastError(env, "fstat64 failed");
    } else {
        (*env)->SetLongField(env, this, key_st_dev, (jlong)fbuf.st_dev);
        (*env)->SetLongField(env, this, key_st_ino, (jlong)fbuf.st_ino);
    }
}

