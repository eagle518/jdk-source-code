/*
 * @(#)SocketInputStream.c	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "jvm.h"
#include "jni_util.h"
#include "net_util.h"

#include "java_net_SocketInputStream.h"


/************************************************************************
 * SocketInputStream
 */

static jfieldID IO_fd_fdID;

/*
 * Class:     java_net_SocketInputStream
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_java_net_SocketInputStream_init(JNIEnv *env, jclass cls) {
    IO_fd_fdID = NET_GetFileDescriptorID(env);
}

/*
 * Class:     java_net_SocketInputStream
 * Method:    socketRead0
 * Signature: (Ljava/io/FileDescriptor;[BIII)I
 */
JNIEXPORT jint JNICALL
Java_java_net_SocketInputStream_socketRead0(JNIEnv *env, jobject this, 
					    jobject fdObj, jbyteArray data,
					    jint off, jint len, jint timeout)
{
    char BUF[MAX_BUFFER_LEN];
    char *bufP;
    jint fd, nread;
    jint n;

    if (IS_NULL(fdObj)) {
	/* should't this be a NullPointerException? -br */
        JNU_ThrowByName(env, JNU_JAVANETPKG "SocketException", 
			"Socket closed");
	return -1;
    } else {
        fd = (*env)->GetIntField(env, fdObj, IO_fd_fdID);
        /* Bug 4086704 - If the Socket associated with this file descriptor
         * was closed (sysCloseFD), the the file descriptor is set to -1.
         */
        if (fd == -1) {
            JNU_ThrowByName(env, "java/net/SocketException", "Socket closed");
            return -1;
        }
    }

    /*
     * If the read is greater than our stack allocated buffer then
     * we allocate from the heap (up to a limit)
     */
    if (len > MAX_BUFFER_LEN) {
	if (len > MAX_HEAP_BUFFER_LEN) {
	    len = MAX_HEAP_BUFFER_LEN;
	}
	bufP = (char *)malloc((size_t)len);
	if (bufP == NULL) {
	    bufP = BUF;
	    len = MAX_BUFFER_LEN;
	}
    } else {
	bufP = BUF;
    }

    if (timeout) {
	nread = NET_Timeout(fd, timeout);
	if (nread <= 0) {
            if (nread == 0) {
                JNU_ThrowByName(env, JNU_JAVANETPKG "SocketTimeoutException",
			    "Read timed out");
            } else if (nread == JVM_IO_ERR) {
	        if (errno == EBADF) { 
                     JNU_ThrowByName(env, JNU_JAVANETPKG "SocketException", "Socket closed");
                 } else {
                     NET_ThrowByNameWithLastError(env, JNU_JAVANETPKG "SocketException",
                                                  "select/poll failed");
                 }
	    } else if (nread == JVM_IO_INTR) {
	        JNU_ThrowByName(env, JNU_JAVAIOPKG "InterruptedIOException",
			    "Operation interrupted");
	    }
	    if (bufP != BUF) {
	        free(bufP);
	    }
	    return -1;
	}
    }

    nread = NET_Read(fd, bufP, len);

    if (nread <= 0) {
	if (nread < 0) {

	    switch (errno) {
		case ECONNRESET:
		case EPIPE:
		    JNU_ThrowByName(env, "sun/net/ConnectionResetException", 	
			"Connection reset");
		    break;

		case EBADF:
		    JNU_ThrowByName(env, JNU_JAVANETPKG "SocketException", 
			"Socket closed");
		    break;

		case EINTR:
                     JNU_ThrowByName(env, JNU_JAVAIOPKG "InterruptedIOException",
                           "Operation interrupted");
		     break;

		default:
	            NET_ThrowByNameWithLastError(env, 
			JNU_JAVANETPKG "SocketException", "Read failed");
	    }
	}
    } else {
        (*env)->SetByteArrayRegion(env, data, off, nread, (jbyte *)bufP);
    }

    if (bufP != BUF) {
	free(bufP);
    }
    return nread;
}					   
