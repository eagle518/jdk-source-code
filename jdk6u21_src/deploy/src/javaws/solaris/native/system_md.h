/*
 * @(#)system_md.h	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef SYSTEM_MD_H
#define SYSTEM_MD_H
/* Defines a common interface to a set of OS specific methods 
*/

/*
 *  Unix specific includes
 */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pwd.h>

#define BREAKPOINT        { *((int*)NULL) = 0; }


#define SOCKET             int
#define INVALID_SOCKET     -1
#define SOCKET_ERROR       -1
#define SOCKADDR           struct sockaddr
#define SOCKADDR_IN        struct sockaddr_in

#define FALSE 0
#define TRUE  1
#define FILE_SEPARATOR '/'
#define PATH_SEPARATOR ':'

#ifdef LINUX
#define PLATFORM "Linux"
#define ARCH     "i386"
#else /* LINUX */
#define PLATFORM "SunOS"
#ifdef __i386
#define ARCH     "x86"
#else
#define ARCH     "sparc"
#endif /* __i386 */
#endif /* LINUX */

typedef short twchar_t;

#endif

