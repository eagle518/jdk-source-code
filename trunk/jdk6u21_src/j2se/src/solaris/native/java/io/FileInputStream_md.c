/*
 * @(#)FileInputStream_md.c	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "io_util_md.h"

#include "java_io_FileInputStream.h"

extern jfieldID fis_fd; /* id for jobject 'fd' in java.io.FileInputStream */

/*********************************************************************
 * Platform specific implementation of input stream native methods
 */

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_close0(JNIEnv *env, jobject this) {
    fileClose(env, this, fis_fd);
}

