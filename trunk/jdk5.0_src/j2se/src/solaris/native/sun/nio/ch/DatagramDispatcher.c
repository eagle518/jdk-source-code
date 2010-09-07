/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DatagramDispatcher.c	1.7 03/12/19
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_DatagramDispatcher.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "nio_util.h"

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramDispatcher_read0(JNIEnv *env, jclass clazz,
			 jobject fdo, jlong address, jint len)
{
    jint fd = fdval(env, fdo);
    void *buf = (void *)jlong_to_ptr(address);
    int result = recv(fd, buf, len, 0);
    if (result < 0 && errno == ECONNREFUSED) {
        JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
        return -2;
    }
    return convertReturnVal(env, result, JNI_TRUE);
}


JNIEXPORT jlong JNICALL
Java_sun_nio_ch_DatagramDispatcher_readv0(JNIEnv *env, jclass clazz,
			      jobject fdo, jlong address, jint len)
{
    jint fd = fdval(env, fdo);
    ssize_t result = 0;
    struct iovec *iov = (struct iovec *)jlong_to_ptr(address);
    struct msghdr m;
    if (len > 16) {
        len = 16;
    }

    m.msg_name = NULL;
    m.msg_namelen = 0;
    m.msg_iov = iov;
    m.msg_iovlen = len;
#ifdef __solaris__
    m.msg_accrights = NULL;
    m.msg_accrightslen = 0;
#endif

#ifdef __linux__
    m.msg_control = NULL;
    m.msg_controllen = 0;
#endif

    result = recvmsg(fd, &m, 0);
    if (result < 0 && errno == ECONNREFUSED) {
        JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
        return -2;
    }
    return convertLongReturnVal(env, (jlong)result, JNI_TRUE);
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramDispatcher_write0(JNIEnv *env, jclass clazz,
			      jobject fdo, jlong address, jint len)
{
    jint fd = fdval(env, fdo);
    void *buf = (void *)jlong_to_ptr(address);
    int result = send(fd, buf, len, 0);
    if (result < 0 && errno == ECONNREFUSED) {
        JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
        return -2;
    }
    return convertReturnVal(env, result, JNI_FALSE);
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_DatagramDispatcher_writev0(JNIEnv *env, jclass clazz,
                                       jobject fdo, jlong address, jint len)
{
    jint fd = fdval(env, fdo);
    struct iovec *iov = (struct iovec *)jlong_to_ptr(address);
    struct msghdr m;
    ssize_t result = 0;
    if (len > 16) {
        len = 16;
    }

    m.msg_name = NULL;
    m.msg_namelen = 0;
    m.msg_iov = iov;
    m.msg_iovlen = len;
#ifdef __solaris__
    m.msg_accrights = NULL;
    m.msg_accrightslen = 0;
#endif

#ifdef __linux__
    m.msg_control = NULL;
    m.msg_controllen = 0;
#endif

    result = sendmsg(fd, &m, 0);
    if (result < 0 && errno == ECONNREFUSED) {
        JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
        return -2;
    }
    return convertLongReturnVal(env, (jlong)result, JNI_FALSE);
}
