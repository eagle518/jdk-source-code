/*
 * @(#)RandomAccessFile_md.c	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"

#include "io_util.h"
#include "io_util_md.h"

#include "java_io_RandomAccessFile.h"

extern jfieldID raf_fd;	/* id for jobject 'fd' in java.io.RandomAccessFile */

/*********************************************************************
 * Platform specific implementation of input stream native methods
 */

JNIEXPORT void JNICALL
Java_java_io_RandomAccessFile_close0(JNIEnv *env, jobject this) {
    handleClose(env, this, raf_fd);
}

