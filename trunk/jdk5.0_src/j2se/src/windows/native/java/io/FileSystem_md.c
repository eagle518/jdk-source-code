/*
 * @(#)FileSystem_md.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
