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
#include <unistd.h>
#include <termios.h>

JNIEXPORT jboolean JNICALL
Java_java_io_Console_istty(JNIEnv *env, jclass cls)
{
    return isatty(fileno(stdin)) && isatty(fileno(stdout));
}

JNIEXPORT jstring JNICALL
Java_java_io_Console_encoding(JNIEnv *env, jclass cls)
{  
    return NULL;
}

JNIEXPORT jboolean JNICALL
Java_java_io_Console_echo(JNIEnv *env, 
			  jclass cls, 
			  jboolean on)
{
    struct termios tio;
    jboolean old;
    int tty = fileno(stdin);
    if (tcgetattr(tty, &tio) == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "tcgetattr failed");
        return !on;
    }
    old = (tio.c_lflag & ECHO);
    if (on) {
	tio.c_lflag |= ECHO;
    } else {
	tio.c_lflag &= ~ECHO;
    }
    if (tcsetattr(tty, TCSANOW, &tio) == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "tcsetattr failed");
    }
    return old;
}

