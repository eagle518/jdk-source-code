/*
 * @(#)socket_md.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#ifdef __solaris__
#include <thread.h>
#endif
#ifdef __linux__
#include <pthread.h>
#include <sys/poll.h>
#endif

#include "socket_md.h"
#include "sysSocket.h"

int
dbgsysListen(int fd, INT32 count) {
    return listen(fd, count);
}

int
dbgsysConnect(int fd, struct sockaddr *name, int namelen) {
    int rv = connect(fd, name, namelen);
    if (rv < 0 && errno == EINPROGRESS) {
	return DBG_EINPROGRESS;
    } else {
	return rv;
    }
}

int
dbgsysFinishConnect(int fd, long timeout) {
    int rv = dbgsysPoll(fd, 0, 1, timeout);
    if (rv == 0) {
	return DBG_ETIMEOUT;
    }
    if (rv > 0) {
	return 0;
    }
    return rv;
}

int
dbgsysAccept(int fd, struct sockaddr *name, int *namelen) {
    int rv; 
    for (;;) {
	rv = accept(fd, name, namelen);
	if (rv >= 0) {
	    return rv;
	}
	if (errno != ECONNABORTED) {
	    return rv;
	}
    }
}

int
dbgsysRecvFrom(int fd, char *buf, int nBytes,
                  int flags, struct sockaddr *from, int *fromlen) {
    return recvfrom(fd, buf, nBytes, flags, from, fromlen);
}

int
dbgsysSendTo(int fd, char *buf, int len,
                int flags, struct sockaddr *to, int tolen) {
    return sendto(fd, buf, len, flags, to, tolen);
}

int
dbgsysRecv(int fd, char *buf, int nBytes, int flags) {
    return recv(fd, buf, nBytes, flags);
}

int
dbgsysSend(int fd, char *buf, int nBytes, int flags) {
    return send(fd, buf, nBytes, flags);
}

struct hostent *
dbgsysGetHostByName(char *hostname) {
    return gethostbyname(hostname);
}

unsigned short
dbgsysHostToNetworkShort(unsigned short hostshort) {
    return htons(hostshort);
}

int
dbgsysSocket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

int dbgsysSocketClose(int fd) {
    return close(fd);
}

int
dbgsysBind(int fd, struct sockaddr *name, int namelen) {
    return bind(fd, name, namelen);
}

UINT32
dbgsysInetAddr(const char* cp) {
    return (UINT32)inet_addr(cp);
}

UINT32
dbgsysHostToNetworkLong(UINT32 hostlong) {
    return htonl(hostlong);
}

unsigned short
dbgsysNetworkToHostShort(unsigned short netshort) {
    return ntohs(netshort);
}

int
dbgsysGetSocketName(int fd, struct sockaddr *name, int *namelen) {
    return getsockname(fd, name, namelen);
}

UINT32
dbgsysNetworkToHostLong(UINT32 netlong) {
    return ntohl(netlong);
}


int
dbgsysSetSocketOption(int fd, jint cmd, jboolean on, jvalue value) 
{
    if (cmd == TCP_NODELAY) {
        struct protoent *proto = getprotobyname("TCP");
        int tcp_level = (proto == 0 ? IPPROTO_TCP: proto->p_proto);
        INT32 onl = (INT32)on;

        if (setsockopt(fd, tcp_level, TCP_NODELAY,
                       (char *)&onl, sizeof(INT32)) < 0) {
                return SYS_ERR;
        }
    } else if (cmd == SO_LINGER) {
        struct linger arg;
        arg.l_onoff = on;

        if(on) {
            arg.l_linger = (unsigned short)value.i;
            if(setsockopt(fd, SOL_SOCKET, SO_LINGER,
                          (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        } else {
            if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
                           (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        }
    } else if (cmd == SO_SNDBUF) {
        jint buflen = value.i;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                       (char *)&buflen, sizeof(buflen)) < 0) {
            return SYS_ERR;
        }
    } else if (cmd == SO_REUSEADDR) {
        int oni = (int)on;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                       (char *)&oni, sizeof(oni)) < 0) {
            return SYS_ERR;

        }
    } else {
        return SYS_ERR;
    }
    return SYS_OK;
}

int
dbgsysConfigureBlocking(int fd, jboolean blocking) {
    int flags = fcntl(fd, F_GETFL);

    if ((blocking == JNI_FALSE) && !(flags & O_NONBLOCK)) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    if ((blocking == JNI_TRUE) && (flags & O_NONBLOCK)) {
        return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    return 0;
}

int
dbgsysPoll(int fd, jboolean rd, jboolean wr, long timeout) {
    struct pollfd fds[1];
    int rv;

    fds[0].fd = fd;
    fds[0].events = 0;
    if (rd) {
        fds[0].events |= POLLIN;
    }
    if (wr) {
        fds[0].events |= POLLOUT;
    }
    fds[0].revents = 0;

    rv = poll(&fds[0], 1, timeout);
    if (rv >= 0) {
        rv = 0;
        if (fds[0].revents & POLLIN) {
            rv |= DBG_POLLIN;
        }
        if (fds[0].revents & POLLOUT) {
	    rv |= DBG_POLLOUT;
        }
    }
    return rv;
}

int
dbgsysGetLastIOError(char *buf, jint size) {
    char *msg = strerror(errno);
    strncpy(buf, msg, size-1);
    buf[size-1] = '\0';  
    return 0;
}

#ifdef __solaris__
int
dbgsysTlsAlloc() {
    thread_key_t tk;
    if (thr_keycreate(&tk, NULL)) {
  	perror("thr_keycreate");
	exit(-1);
    }
    return (int)tk;
}

void
dbgsysTlsFree(int index) {
   /* no-op */
}

void
dbgsysTlsPut(int index, void *value) {
    thr_setspecific((thread_key_t)index, value) ;
}

void *
dbgsysTlsGet(int index) {
    void* r = NULL;
    thr_getspecific((thread_key_t)index, &r);
    return r;
}

#endif

#ifdef __linux__
int
dbgsysTlsAlloc() {
    pthread_key_t key;
    if (pthread_key_create(&key, NULL)) {
        perror("pthread_key_create");
        exit(-1);
    }
    return (int)key;
}

void
dbgsysTlsFree(int index) {
    pthread_key_delete((pthread_key_t)index);
}

void
dbgsysTlsPut(int index, void *value) {
    pthread_setspecific((pthread_key_t)index, value) ;
}

void *
dbgsysTlsGet(int index) {
    return pthread_getspecific((pthread_key_t)index);
}

#endif



