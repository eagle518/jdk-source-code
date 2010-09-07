/*
 * @(#)FileOutputStream_md.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

#include "io_util.h"
#include "io_util_md.h"
#include "java_io_FileOutputStream.h"

#include <fcntl.h>

/*******************************************************************/
/*  BEGIN JNI ********* BEGIN JNI *********** BEGIN JNI ************/
/*******************************************************************/

jfieldID fos_fd; /* id for jobject 'fd' in java.io.FileOutputStream */

/**************************************************************
 * static methods to store field ID's in initializers
 */

JNIEXPORT void JNICALL 
Java_java_io_FileOutputStream_initIDs(JNIEnv *env, jclass fdClass) {
    fos_fd = (*env)->GetFieldID(env, fdClass, "fd", "Ljava/io/FileDescriptor;");
}

/**************************************************************
 * Output stream
 */

JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_open(JNIEnv *env, jobject this, jstring path) {
    fileOpen(env, this, path, fos_fd, O_WRONLY | O_CREAT | O_TRUNC);
}

JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_openAppend(JNIEnv *env, jobject this, jstring path) {
    fileOpen(env, this, path, fos_fd, O_WRONLY | O_CREAT | O_APPEND);
}

JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_write(JNIEnv *env, jobject this, jint byte) {
    writeSingle(env, this, byte, fos_fd);
}

JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_writeBytes(JNIEnv *env,
    jobject this, jbyteArray bytes, jint off, jint len) {
    writeBytes(env, this, bytes, off, len, fos_fd);
}
    
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_close0(JNIEnv *env, jobject this) {
    fileClose(env, this, fos_fd);
}
