/*
 * @(#)ServerSocketChannelImpl.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"

#include "nio.h"
#include "nio_util.h"
#include "net_util.h"

#include "sun_nio_ch_ServerSocketChannelImpl.h"


static jfieldID fd_fdID;	/* java.io.FileDescriptor.fd */
static jclass isa_class;	/* java.net.InetSocketAddress */
static jmethodID isa_ctorID;	/* InetSocketAddress(InetAddress, int) */
static jclass ia_class;	        /* java.net.InetAddress */
static jmethodID ia_ctorID;     /* InetAddress() */
static jfieldID ia_addrID;      /* java.net.InetAddress.address */
static jfieldID ia_famID;       /* java.net.InetAddress.family */


/**************************************************************
 * static method to store field IDs in initializers
 */

JNIEXPORT void JNICALL 
Java_sun_nio_ch_ServerSocketChannelImpl_initIDs(JNIEnv *env, jclass cls)
{
    cls = (*env)->FindClass(env, "java/io/FileDescriptor");
    fd_fdID = (*env)->GetFieldID(env, cls, "fd", "I");

    cls = (*env)->FindClass(env, "java/net/InetSocketAddress");
    isa_class = (*env)->NewGlobalRef(env, cls);
    isa_ctorID = (*env)->GetMethodID(env, cls, "<init>",
				     "(Ljava/net/InetAddress;I)V");

    cls = (*env)->FindClass(env, "java/net/Inet4Address");
    ia_class = (*env)->NewGlobalRef(env, cls);
    ia_ctorID = (*env)->GetMethodID(env, cls, "<init>","()V");
    ia_addrID = (*env)->GetFieldID(env, cls, "address", "I");
    ia_famID = (*env)->GetFieldID(env, cls, "family", "I");
}

JNIEXPORT void JNICALL 
Java_sun_nio_ch_ServerSocketChannelImpl_listen(JNIEnv *env, jclass cl,
					       jobject fdo, jint backlog)
{
    if (listen(fdval(env,fdo), backlog) == SOCKET_ERROR) {
	NET_ThrowNew(env, WSAGetLastError(), "listen");
    }
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_ServerSocketChannelImpl_accept0(JNIEnv *env, jobject this,
                                                jobject ssfdo, jobject newfdo,
						jobjectArray isaa)
{
    jint ssfd = (*env)->GetIntField(env, ssfdo, fd_fdID);
    jint newfd;
    struct sockaddr_in sa;
    jobject remote_ia = 0;
    jobject isa;
    jobject ia;
    int addrlen = sizeof(sa);

    memset((char *)&sa, 0, sizeof(sa));
    newfd = (jint)accept(ssfd, (struct sockaddr *)&sa, &addrlen);
    if (newfd == INVALID_SOCKET) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return IOS_UNAVAILABLE;
        }
	JNU_ThrowIOExceptionWithLastError(env, "Accept failed");
	return IOS_THROWN;
    }
    (*env)->SetIntField(env, newfdo, fd_fdID, newfd);

    ia = (*env)->NewObject(env, ia_class, ia_ctorID);
    (*env)->SetIntField(env, ia, ia_addrID, ntohl(sa.sin_addr.s_addr));
    (*env)->SetIntField(env, ia, ia_famID, sa.sin_family);

    isa = (*env)->NewObject(env, isa_class, isa_ctorID, ia,
                            ntohs(sa.sin_port));
    (*env)->SetObjectArrayElement(env, isaa, 0, isa);
    return 1;
}
