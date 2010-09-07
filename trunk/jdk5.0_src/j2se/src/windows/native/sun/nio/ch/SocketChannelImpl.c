/*
 * @(#)SocketChannelImpl.c	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_SocketChannelImpl.h"

#include "nio.h"
#include "nio_util.h"
#include "net_util.h"


static jfieldID ia_addrID;      /* java.net.InetAddress.address */

JNIEXPORT void JNICALL
Java_sun_nio_ch_SocketChannelImpl_initIDs(JNIEnv *env, jclass cls)
{
    cls = (*env)->FindClass(env, "java/net/InetAddress");
    ia_addrID = (*env)->GetFieldID(env, cls, "address", "I");
}

jint
handleSocketError(JNIEnv *env, int errorValue)
{
    NET_ThrowNew(env, errorValue, NULL);
    return IOS_THROWN;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_SocketChannelImpl_checkConnect(JNIEnv *env, jobject this,
                                               jobject fdo, jboolean block,
                                               jboolean ready)
{
    int optError = 0;
    int lastError = 0;
    int result = 0;
    int retry = 0;
    int n = sizeof(int);
    jint fd = fdval(env, fdo);
    fd_set wr, ex;
    struct timeval t = { 0, 0 };

    FD_ZERO(&wr);
    FD_ZERO(&ex);
    FD_SET((u_int)fd, &wr);
    FD_SET((u_int)fd, &ex);

    result = select(fd+1, 0, &wr, &ex, block ? NULL : &t);

    /* save last winsock error */
    if (result == SOCKET_ERROR) {
        lastError = WSAGetLastError();
    }

    if (block) { /* must configure socket back to blocking state */
        u_long argp = 0;
        int r = ioctlsocket(fd, FIONBIO, &argp);
        if (r == SOCKET_ERROR) {
            handleSocketError(env, WSAGetLastError());
        }
    }

    if (result == 0) {  /* timeout */
        return block ? 0 : IOS_UNAVAILABLE;
    } else {
        if (result == SOCKET_ERROR)	{ /* select failed */
            handleSocketError(env, lastError);
            return IOS_THROWN;
        }
    }

    /*  
     * Socket is writable or error occured. On some Windows editions  
     * the socket will appear writable when the connect fails so we 
     * check for error rather than writable. 
     */ 
    if (!FD_ISSET(fd, &ex)) { 
        return 1;		/* connection established */
    }

    /* 
     * A getsockopt( SO_ERROR ) may indicate success on NT4 even
     * though the connection has failed. The workaround is to allow 
     * winsock to be scheduled and this is done via by yielding.
     * As the yield approach is problematic in heavy load situations
     * we attempt up to 3 times to get the failure reason.
     */
    for (retry=0; retry<3; retry++) {
        result = getsockopt((SOCKET)fd,
                            SOL_SOCKET,
                            SO_ERROR,
                            (char *)&optError,
                            &n);
        if (result == SOCKET_ERROR) {
            int lastError = WSAGetLastError();
            if (lastError == WSAEINPROGRESS) {
                return IOS_UNAVAILABLE;
            }
            NET_ThrowNew(env, lastError, "getsockopt");
            return IOS_THROWN;
        }
        if (optError) {
            break;
        }
        Sleep(0);
    }

    if (optError != NO_ERROR) {
        handleSocketError(env, optError);
        return IOS_THROWN;
    }

    return 0;
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_SocketChannelImpl_shutdown(JNIEnv *env, jclass cl,
					   jobject fdo, jint how)
{
    if (shutdown(fdval(env, fdo), how) == SOCKET_ERROR) {
	NET_ThrowNew(env, WSAGetLastError(), "shutdown");
    }
}

