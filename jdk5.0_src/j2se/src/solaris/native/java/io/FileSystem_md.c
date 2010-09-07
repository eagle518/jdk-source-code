/*
 * @(#)FileSystem_md.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "java_io_FileSystem.h"


JNIEXPORT jobject JNICALL  
Java_java_io_FileSystem_getFileSystem(JNIEnv *env, jclass ignored)
{
    return JNU_NewObjectByName(env, "java/io/UnixFileSystem", "()V");
}
