/*
 * @(#)sysSocket.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _JAVASOFT_WIN32_SOCKET_MD_H

#include <jni.h>
#include <sys/types.h>
#include "sys.h"
#include "socket_md.h"

#ifdef _LP64
typedef unsigned int    UINT32;
typedef int             INT32;
#else /* _LP64 */
typedef unsigned long   UINT32;
typedef long            INT32;
#endif /* _LP64 */

#define DBG_POLLIN		1
#define DBG_POLLOUT		2

#define DBG_EINPROGRESS		-150
#define DBG_ETIMEOUT		-200

int dbgsysSocketClose(int fd);
INT32 dbgsysSocketAvailable(int fd, INT32 *pbytes);
int dbgsysConnect(int fd, struct sockaddr *him, int len);
int dbgsysFinishConnect(int fd, long timeout);
int dbgsysAccept(int fd, struct sockaddr *him, int *len);
int dbgsysSendTo(int fd, char *buf, int len, int flags, struct sockaddr *to,
	      int tolen);
int dbgsysRecvFrom(int fd, char *buf, int nbytes, int flags,
                struct sockaddr *from, int *fromlen);
int dbgsysListen(int fd, INT32 count);
int dbgsysRecv(int fd, char *buf, int nBytes, int flags);
int dbgsysSend(int fd, char *buf, int nBytes, int flags);
int dbgsysTimeout(int fd, INT32 timeout); 
struct hostent *dbgsysGetHostByName(char *hostname);
int dbgsysSocket(int domain, int type, int protocol);
int dbgsysBind(int fd, struct sockaddr *name, int namelen);
int dbgsysSetSocketOption(int fd, jint cmd, jboolean on, jvalue value);
UINT32 dbgsysInetAddr(const char* cp);
UINT32 dbgsysHostToNetworkLong(UINT32 hostlong);
unsigned short dbgsysHostToNetworkShort(unsigned short hostshort);
UINT32 dbgsysNetworkToHostLong(UINT32 netlong);
unsigned short dbgsysNetworkToHostShort(unsigned short netshort);
int dbgsysGetSocketName(int fd, struct sockaddr *him, int *len);
int dbgsysConfigureBlocking(int fd, jboolean blocking);
int dbgsysPoll(int fd, jboolean rd, jboolean wr, long timeout);
int dbgsysGetLastIOError(char *buf, jint size);

/*
 * TLS support
 */
int dbgsysTlsAlloc();
void dbgsysTlsFree(int index);
void dbgsysTlsPut(int index, void *value);
void* dbgsysTlsGet(int index);

#endif


