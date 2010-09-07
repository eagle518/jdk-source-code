/*
 * @(#)FileDescriptor_md.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

#include "java_io_FileDescriptor.h"

/*******************************************************************/
/*  BEGIN JNI ********* BEGIN JNI *********** BEGIN JNI ************/
/*******************************************************************/

/* field id for jint 'fd' in java.io.FileDescriptor */
jfieldID IO_fd_fdID; 

/**************************************************************
 * static methods to store field ID's in initializers
 */

JNIEXPORT void JNICALL 
Java_java_io_FileDescriptor_initIDs(JNIEnv *env, jclass fdClass) {
    IO_fd_fdID = (*env)->GetFieldID(env, fdClass, "fd", "I");
}

/**************************************************************
 * File Descriptor
 */

JNIEXPORT void JNICALL 
Java_java_io_FileDescriptor_sync(JNIEnv *env, jobject this) {
    int fd = (*env)->GetIntField(env, this, IO_fd_fdID);
    if (JVM_Sync(fd) == -1) {
	JNU_ThrowByName(env, "java/io/SyncFailedException", "sync failed");
    }	
}
