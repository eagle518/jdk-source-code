/*
 * @(#)DevPollArrayWrapper.c	1.13 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_DevPollArrayWrapper.h"
#include <sys/poll.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef	uint32_t	caddr32_t;

/* /dev/poll ioctl */
#define		DPIOC	(0xD0 << 8)
#define	DP_POLL		(DPIOC | 1)	/* poll on fds in cached in /dev/poll */
#define	DP_ISPOLLED	(DPIOC | 2)	/* is this fd cached in /dev/poll */
#define	DEVPOLLSIZE	1000		/* /dev/poll table size increment */
#define POLLREMOVE      0x0800          /* Removes fd from monitored set */

/*
 * /dev/poll DP_POLL ioctl format
 */
typedef struct dvpoll {
	pollfd_t	*dp_fds;	/* pollfd array */
	nfds_t		dp_nfds;	/* num of pollfd's in dp_fds[] */
	int		dp_timeout;	/* time out in millisec */
} dvpoll_t;

typedef struct dvpoll32 {
	caddr32_t	dp_fds;		/* pollfd array */
	uint32_t	dp_nfds;	/* num of pollfd's in dp_fds[] */
	int32_t		dp_timeout;	/* time out in millisec */
} dvpoll32_t;

#ifdef	__cplusplus
}
#endif

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DevPollArrayWrapper_init(JNIEnv *env, jobject this)
{
    int wfd = open("/dev/poll", O_RDWR);
    if (wfd < 0) {
       JNU_ThrowIOExceptionWithLastError(env, "Error opening driver");
       return -1;
    }
    return wfd;
}

JNIEXPORT void JNICALL 
Java_sun_nio_ch_DevPollArrayWrapper_register(JNIEnv *env, jobject this,
                                             jint wfd, jint fd, jint mask)
{
    struct pollfd a[2];
    unsigned char *pollBytes = (unsigned char *)&a[0];
    unsigned char *pollEnd = pollBytes + sizeof(struct pollfd) * 2;

    /* We clear it first, otherwise any entries between poll invocations
       get OR'd together */
    a[0].fd = fd;             
    a[0].events = POLLREMOVE;
    a[0].revents = 0;
     
    a[1].fd = fd;
    a[1].events = mask;
    a[1].revents = 0;

    while (pollBytes < pollEnd) {
        int bytesWritten = write(wfd, pollBytes, (int)(pollEnd - pollBytes));
        if (bytesWritten < 0) {
            JNU_ThrowIOExceptionWithLastError(env, "Error writing pollfds");
            return;
        }
        pollBytes += bytesWritten;
    }
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_DevPollArrayWrapper_registerMultiple(JNIEnv *env, jobject this,
                                                     jint wfd, jlong address,
                                                     jint len)
{
    unsigned char *pollBytes = (unsigned char *)jlong_to_ptr(address);
    unsigned char *pollEnd = pollBytes + sizeof(struct pollfd) * len;
    while (pollBytes < pollEnd) {
        int bytesWritten = write(wfd, pollBytes, (int)(pollEnd - pollBytes));
        if (bytesWritten < 0) {
            JNU_ThrowIOExceptionWithLastError(env, "Error writing pollfds");
            return;
        }
        pollBytes += bytesWritten;
    }
}

JNIEXPORT jint JNICALL 
Java_sun_nio_ch_DevPollArrayWrapper_poll0(JNIEnv *env, jobject this,
                                       jlong address, jint numfds,
                                       jlong timeout, jint wfd)
{
    struct dvpoll a;
    void *pfd = (void *) jlong_to_ptr(address);
    int result = 0;

    a.dp_fds = pfd;
    a.dp_nfds = numfds;
    a.dp_timeout = (int)timeout;

    result = ioctl(wfd, DP_POLL, &a);
    if (result < 0) {
        JNU_ThrowIOExceptionWithLastError(env, "Error reading driver");
        return -1;
    }
    return result;
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_DevPollArrayWrapper_interrupt(JNIEnv *env, jobject this, jint fd)
{
    int fakebuf[1];
    fakebuf[0] = 1;
    if (write(fd, fakebuf, 1) < 0) {
        JNU_ThrowIOExceptionWithLastError(env,
                                          "Write to interrupt fd failed");
    }
}


