/*
 * @(#)JDNsprData.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  JDNsprData.h  by X.Lu
 * 
 *
 * Definetion of various NSPR data type and macros 
 */

#ifndef _JDNSPRDATA_H_
#define _JDNSPRDATA_H_

#include "JDData.h"
/* NSPR Portal */
typedef enum JDThreadType {
    JD_USER_THREAD,
    JD_SYSTEM_THREAD
} JDThreadType;

typedef enum JDThreadPriority {
    JD_PRIORITY_FIRST = 0,
    JD_PRIORITY_LOW = 0,
    JD_PRIORITY_NORMAL = 1,
    JD_PRIORITY_HIGH = 2,
    JD_PRIORITY_URGENT = 3,
    JD_PRIORITY_LAST = 3
} JDThreadPriority;

typedef enum JDThreadScope {
    JD_LOCAL_THREAD,
    JD_GLOBAL_THREAD,
    JD_GLOBAL_BOUND_THREAD
} JDThreadScope;

typedef enum JDThreadState {
    JD_JOINABLE_THREAD,
    JD_UNJOINABLE_THREAD
} JDThreadState;

typedef struct JDIPv6Addr {
  union {
         JDUint8  _S6_u8[16];
         JDUint16 _S6_u16[8];
         JDUint32 _S6_u32[4];
         JDUint64 _S6_u64[2];
  } _S6_un;
} JDIPv6Addr;

typedef union JDNetAddr {
    struct {
         JDUint16 family;                /* address family (0x00ff maskable) */
#ifdef XP_BEOS
         char data[10];                  /* Be has a smaller structure */
#else
         char data[14];                  /* raw address data */
#endif
    } raw;
    struct {
        JDUint16 family;                /* address family (AF_INET) */
        JDUint16 port;                  /* port number */
        JDUint32 ip;                    /* The actual 32 bits of address */
#ifdef XP_BEOS
        char pad[4];                    /* Be has a smaller structure */
#else
        char pad[8];
#endif
    } inet;
    struct {
        JDUint16 family;                /* address family (AF_INET6) */
        JDUint16 port;                  /* port number */
        JDUint32 flowinfo;              /* routing information */
        JDIPv6Addr ip;                  /* the actual 128 bits of address */
        JDUint32 scope_id;              /* set of interfaces for a scope */
    } ipv6;
#if defined(XP_UNIX)
    struct {                            /* Unix domain socket address */
        JDUint16 family;                /* address family (AF_UNIX) */
        char path[104];                 /* null-terminated pathname */
    } local;
#endif
} JDNetAddr;

#ifdef XP_UNIX
typedef struct JDPollDesc {
    void *fd;
    JDint16 in_flags;
    JDint16 out_flags;
} JDPollDesc;

typedef JDUint32 JDIntervalTime;
#include <poll.h>

#define JD_INTERVAL_NO_TIMEOUT 0xffffffffUL
#define JD_POLL_READ    POLLIN
#define JD_POLL_WRITE   POLLOUT
#define JD_POLL_EXCEPT  POLLPRI
#define JD_POLL_ERR     POLLERR         /* only in out_flags */
#define JD_POLL_NVAL    POLLNVAL        /* only in out_flags when fd is bad */
#define JD_POLL_HUP     POLLHUP         /* only in out_flags */
#endif

#endif //_JDNSPRDATA_H_
