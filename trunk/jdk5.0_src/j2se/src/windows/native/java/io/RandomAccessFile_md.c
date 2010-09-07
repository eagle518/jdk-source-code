/*
 * @(#)RandomAccessFile_md.c	1.4 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

