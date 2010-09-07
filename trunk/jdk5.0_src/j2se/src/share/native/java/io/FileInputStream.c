/*
 * @(#)FileInputStream.c	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "io_util.h"

#include "jvm.h"

#include "java_io_FileInputStream.h"

#include <fcntl.h>
#include <limits.h>

#include "io_util_md.h"

/*******************************************************************/
/*  BEGIN JNI ********* BEGIN JNI *********** BEGIN JNI ************/
/*******************************************************************/

jfieldID fis_fd; /* id for jobject 'fd' in java.io.FileInputStream */

/**************************************************************
 * static methods to store field ID's in initializers
 */

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_initIDs(JNIEnv *env, jclass fdClass) {
    fis_fd = (*env)->GetFieldID(env, fdClass, "fd", "Ljava/io/FileDescriptor;");
}

/**************************************************************
 * Input stream
 */

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_open(JNIEnv *env, jobject this, jstring path) {
    fileOpen(env, this, path, fis_fd, O_RDONLY);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_read(JNIEnv *env, jobject this) {
    return readSingle(env, this, fis_fd);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_readBytes(JNIEnv *env, jobject this,
	jbyteArray bytes, jint off, jint len) {
    return readBytes(env, this, bytes, off, len, fis_fd);
}

JNIEXPORT jlong JNICALL
Java_java_io_FileInputStream_skip(JNIEnv *env, jobject this, jlong toSkip) {
    jlong cur = jlong_zero;
    jlong end = jlong_zero;
    FD fd = GET_FD(this, fis_fd);
    if ((cur = IO_Lseek(fd, (jlong)0, (jint)SEEK_CUR)) == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "Seek error");
    } else if ((end = IO_Lseek(fd, toSkip, (jint)SEEK_CUR)) == -1) {
	JNU_ThrowIOExceptionWithLastError(env, "Seek error");
    }
    return (end - cur);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_available(JNIEnv *env, jobject this) {
    jlong ret;
    FD fd = GET_FD(this, fis_fd);

    if (IO_Available(fd, &ret)) {
        if (ret > INT_MAX) {
	    ret = (jlong) INT_MAX;
	}
	return jlong_to_jint(ret);
    }
    JNU_ThrowIOExceptionWithLastError(env, NULL);
    return 0;
}
