/*
 * @(#)NativeSignalHandler.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <signal.h>
#include <stdlib.h>

#include <jni.h>
#include <jlong.h>
#include "sun_misc_NativeSignalHandler.h"

typedef void (*sig_handler_t)(jint, void *, void *);

JNIEXPORT void JNICALL
Java_sun_misc_NativeSignalHandler_handle0(JNIEnv *env, jclass cls, jint sig, jlong f)
{
    /* We've lost the siginfo and context */
    (*(sig_handler_t)jlong_to_ptr(f))(sig, NULL, NULL);
}

