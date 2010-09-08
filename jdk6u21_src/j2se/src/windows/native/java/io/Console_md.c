/*
 * @(#)Console_md.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "java_io_Console.h"

#include <stdlib.h>
#include <Wincon.h>

static HANDLE hStdOut = INVALID_HANDLE_VALUE;
static HANDLE hStdIn = INVALID_HANDLE_VALUE;
JNIEXPORT jboolean JNICALL
Java_java_io_Console_istty(JNIEnv *env, jclass cls) 
{
    if (hStdIn == INVALID_HANDLE_VALUE &&
        (hStdIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
     	JNU_ThrowIOExceptionWithLastError(env, "Open Console input failed");
        return JNI_FALSE;
    } 
    if (hStdOut == INVALID_HANDLE_VALUE &&
        (hStdOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
     	JNU_ThrowIOExceptionWithLastError(env, "Open Console output failed");
        return JNI_FALSE;
    } 
    if (GetFileType(hStdIn) != FILE_TYPE_CHAR ||
        GetFileType(hStdOut) != FILE_TYPE_CHAR)
        return JNI_FALSE;
    return JNI_TRUE;
}

JNIEXPORT jstring JNICALL
Java_java_io_Console_encoding(JNIEnv *env, jclass cls)
{  
    char buf[64];
    int cp = GetConsoleCP();
    if (cp >= 874 && cp <= 950)
        sprintf(buf, "ms%d", cp);
    else
        sprintf(buf, "cp%d", cp);
    return JNU_NewStringPlatform(env, buf);
}

JNIEXPORT jboolean JNICALL
Java_java_io_Console_echo(JNIEnv *env, jclass cls, jboolean on)
{
    DWORD fdwMode;
    jboolean old;
    if (! GetConsoleMode(hStdIn, &fdwMode)) {
     	JNU_ThrowIOExceptionWithLastError(env, "GetConsoleMode failed");
        return !on;
    }
    old = (fdwMode & ENABLE_ECHO_INPUT) != 0;
    if (on) {
        fdwMode |= ENABLE_ECHO_INPUT;
    } else {
        fdwMode &= ~ENABLE_ECHO_INPUT;
    }
    if (! SetConsoleMode(hStdIn, fdwMode)) {
     	JNU_ThrowIOExceptionWithLastError(env, "SetConsoleMode failed");
    }
    return old;
}
