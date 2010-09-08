/*
 * @(#)FileDescriptor_md.c	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "jlong.h"
#include "io_util_md.h"

#include "java_io_FileDescriptor.h"

/*******************************************************************/
/*  BEGIN JNI ********* BEGIN JNI *********** BEGIN JNI ************/
/*******************************************************************/

/* field id for jint 'fd' in java.io.FileDescriptor */
jfieldID IO_fd_fdID; 

/* field id for jlong 'handle' in java.io.FileDescriptor */
jfieldID IO_handle_fdID;

/**************************************************************
 * static methods to store field IDs in initializers
 */

JNIEXPORT void JNICALL 
Java_java_io_FileDescriptor_initIDs(JNIEnv *env, jclass fdClass) {
    IO_fd_fdID = (*env)->GetFieldID(env, fdClass, "fd", "I");
    IO_handle_fdID = (*env)->GetFieldID(env, fdClass, "handle", "J");
}

JNIEXPORT jlong JNICALL 
Java_java_io_FileDescriptor_set(JNIEnv *env, jclass fdClass, jint fd) {
    SET_HANDLE(fd);
}

/**************************************************************
 * File Descriptor
 */

JNIEXPORT void JNICALL 
Java_java_io_FileDescriptor_sync(JNIEnv *env, jobject this) {
    FD fd = THIS_FD(this);
    if (IO_Sync(fd) == -1) {
	JNU_ThrowByName(env, "java/io/SyncFailedException", "sync failed");
    }
}
