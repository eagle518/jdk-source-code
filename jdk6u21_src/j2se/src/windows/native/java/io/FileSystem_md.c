/*
 * @(#)FileSystem_md.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "jni.h"
#include "jni_util.h"

extern jboolean onNT;
extern void initializeWindowsVersion();

JNIEXPORT jobject JNICALL  
Java_java_io_FileSystem_getFileSystem(JNIEnv *env, jclass ignored)
{
    initializeWindowsVersion();
    if (onNT) {
        return JNU_NewObjectByName(env, "java/io/WinNTFileSystem", "()V");
    } else {
        return JNU_NewObjectByName(env, "java/io/Win32FileSystem", "()V");
    }
}
