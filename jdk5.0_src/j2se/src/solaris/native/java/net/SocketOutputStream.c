/*
 * @(#)SocketOutputStream.c	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "jni_util.h"
#include "jvm.h"
#include "net_util.h"

#include "java_net_SocketOutputStream.h"

#define min(a, b)       ((a) < (b) ? (a) : (b))

/*
 * SocketOutputStream
 */

static jfieldID IO_fd_fdID;

/*
 * Class:     java_net_SocketOutputStream
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_net_SocketOutputStream_init(JNIEnv *env, jclass cls) {
    IO_fd_fdID = NET_GetFileDescriptorID(env);
}

/*
 * Class:     java_net_SocketOutputStream
 * Method:    socketWrite0
 * Signature: (Ljava/io/FileDescriptor;[BII)V
 */
JNIEXPORT void JNICALL
Java_java_net_SocketOutputStream_socketWrite0(JNIEnv *env, jobject this, 
					      jobject fdObj,
					      jbyteArray data,
					      jint off, jint len) {
    char *bufP;
    char BUF[MAX_BUFFER_LEN];
    int buflen;
    int fd;
    jint n = 0;

    if (IS_NULL(fdObj)) {
	JNU_ThrowByName(env, "java/net/SocketException", "Socket closed");
	return;
    } else {
	fd = (*env)->GetIntField(env, fdObj, IO_fd_fdID);
        /* Bug 4086704 - If the Socket associated with this file descriptor
         * was closed (sysCloseFD), the the file descriptor is set to -1.
         */
        if (fd == -1) {
            JNU_ThrowByName(env, "java/net/SocketException", "Socket closed");
            return;
        }

    }

    if (len <= MAX_BUFFER_LEN) {
	bufP = BUF;
	buflen = MAX_BUFFER_LEN;
    } else {
	buflen = min(MAX_HEAP_BUFFER_LEN, len);
	bufP = (char *)malloc((size_t)buflen);

	/* if heap exhausted resort to stack buffer */
	if (bufP == NULL) {
	    bufP = BUF;
	    buflen = MAX_BUFFER_LEN;
	}
    }

    while(len > 0) {
	int loff = 0;
	int chunkLen = min(buflen, len);
	int llen = chunkLen;
	(*env)->GetByteArrayRegion(env, data, off, chunkLen, (jbyte *)bufP);
      
	while(llen > 0) {
	    int n = NET_Send(fd, bufP + loff, llen, 0);
	    if (n > 0) {
		llen -= n;
		loff += n;
		continue;
	    }
	    if (n == JVM_IO_INTR) {
		JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
	    } else {
		if (errno == ECONNRESET) {
                    JNU_ThrowByName(env, "sun/net/ConnectionResetException",
                        "Connection reset");
		} else {
		    NET_ThrowByNameWithLastError(env, "java/net/SocketException", 
			"Write failed");
		}
	    }
	    if (bufP != BUF) {
		free(bufP);
	    }
	    return;
	}
	len -= chunkLen;
	off += chunkLen;
    }

    if (bufP != BUF) {
	free(bufP);
    }
}




