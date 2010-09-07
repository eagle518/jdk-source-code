/*
 * @(#)NativeThread.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/types.h>
#include <string.h>
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_NativeThread.h"
#include "nio_util.h"


#ifdef __linux__
#include <pthread.h>
#include <sys/signal.h>

/* Also defined in src/solaris/native/java/net/linux_close.c */
#define INTERRUPT_SIGNAL (__SIGRTMAX - 2)

static void
nullHandler(int sig)
{
}

#endif


JNIEXPORT void JNICALL
Java_sun_nio_ch_NativeThread_init(JNIEnv *env, jclass cl)
{
#ifdef __linux__

    /* Install the null handler for INTERRUPT_SIGNAL.  This might overwrite the
     * handler previously installed by java/net/linux_close.c, but that's okay
     * since neither handler actually does anything.  We install our own
     * handler here simply out of paranoia; ultimately the two mechanisms
     * should somehow be unified, perhaps within the VM.
     */

    sigset_t ss;
    struct sigaction sa, osa;
    sa.sa_handler = nullHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(INTERRUPT_SIGNAL, &sa, &osa) < 0)
	JNU_ThrowIOExceptionWithLastError(env, "sigaction");

#endif
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_NativeThread_current(JNIEnv *env, jclass cl)
{
#ifdef __linux__
    return (long)pthread_self();
#else
    return 0;
#endif
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_NativeThread_signal(JNIEnv *env, jclass cl, jlong thread)
{
#ifdef __linux__
    if (pthread_kill((pthread_t)thread, INTERRUPT_SIGNAL))
	JNU_ThrowIOExceptionWithLastError(env, "Thread signal failed");
#endif
}
