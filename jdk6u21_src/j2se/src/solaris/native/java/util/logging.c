/*
 * @(#)logging.c	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Solaris/Linux specific code to support logging.
 */

#include <unistd.h>
#include "jni_util.h"


JNIEXPORT jboolean JNICALL
Java_java_util_logging_FileHandler_isSetUID(JNIEnv *env, jclass thisclass) {

    /* Return true if we are in a set UID or set GID process. */
    if (getuid() != geteuid() || getgid() != getegid()) {
	return JNI_TRUE;
    }
    return JNI_FALSE;
}
