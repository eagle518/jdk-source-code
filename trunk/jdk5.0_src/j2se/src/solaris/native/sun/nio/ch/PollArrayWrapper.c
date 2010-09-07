/*
 * @(#)PollArrayWrapper.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_PollArrayWrapper.h"
#include <poll.h>


JNIEXPORT jint JNICALL 
Java_sun_nio_ch_PollArrayWrapper_poll0(JNIEnv *env, jobject this,
                                       jlong address, jint numfds,
                                       jlong timeout)
{
    struct pollfd *a;
    int err = 0;

    a = (struct pollfd *) jlong_to_ptr(address);
    err = poll(a, numfds, timeout);

    if (err < 0)
        JNU_ThrowIOExceptionWithLastError(env, "Poll failed");
    return (jint)err;
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_PollArrayWrapper_interrupt(JNIEnv *env, jobject this, jint fd)
{
    int fakebuf[1];
    fakebuf[0] = 1;
    if (write(fd, fakebuf, 1) < 0) {
         JNU_ThrowIOExceptionWithLastError(env,
                                          "Write to interrupt fd failed");
    }
}
