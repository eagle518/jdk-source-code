/*
 * @(#)IUnixService.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//  JDunrt.h  by X.Lu
//
///=--------------------------------------------------------------------------=
//  A wrapper of Unix runtime library
//
#ifndef _IUNIXSERVICE_H_
#define _IUNIXSERVICE_H_

#include "JDNsprData.h"
#include "JDSupportUtils.h"

// A abstraction of Unix service representation
class IUnixService {
 public:
    // NSPR functions
    JD_IMETHOD_(void*)  JD_GetCurrentThread(void) = 0;
    
    JD_IMETHOD_(void*)  JD_NewMonitor(void) = 0;
    JD_IMETHOD_(void)   JD_DestroyMonitor(void* mon) = 0;

    JD_IMETHOD_(void)   JD_EnterMonitor(void* mon) = 0;
    JD_IMETHOD_(JDBool)   JD_ExitMonitor(void* mon) = 0;
    JD_IMETHOD_(JDBool)   JD_Wait(void* mon, JDUint32 ticks) = 0;
    JD_IMETHOD_(JDBool)   JD_NotifyAll(void* mon) = 0;
    
    JD_IMETHOD_(void*)    JD_NewTCPSocket(void) = 0;
    JD_IMETHOD_(JDBool)   JD_NewTCPSocketPair(void *fd[2]) = 0;
    JD_IMETHOD_(void*)    JD_Socket(JDint32 domain, JDint32 type, JDint32 proto) = 0;
    JD_IMETHOD_(JDBool)   JD_CreatePipe(void** readPipe, void** writePipe) = 0;
    JD_IMETHOD_(JDBool)   JD_Bind(void* fd, void* addr) = 0;
    JD_IMETHOD_(void*)    JD_Accept(void* fd, void* addr, JDUint32 timeout )= 0;
        
    JD_IMETHOD_(JDBool)   JD_Close(void* fd) = 0;
    JD_IMETHOD_(JDint32)  JD_Read(void* fd, void* buf, JDint32 amount) = 0;
    JD_IMETHOD_(JDint32)  JD_Write(void* fd, const void* buf, JDint32 amount) = 0;
    JD_IMETHOD_(JDBool)   JD_Sync(void* fd) = 0;
    JD_IMETHOD_(JDBool)   JD_Listen(void* fd, JDIntn backlog) = 0;
    
    JD_IMETHOD_(JDint32)  JD_Available(void* fd) = 0;
    JD_IMETHOD_(JDint32)  JD_Poll(struct JDPollDesc pds[], int npds, JDUint32 JDIntervalTim) = 0;
    

    JD_IMETHOD_(void*)    JD_CreateThread(JDThreadType type,
                             void (*start)(void* arg),
                             void* arg,
                             JDThreadPriority priority,
                             JDThreadScope    scope,
                             JDThreadState    state,
                             JDUint32 stackSize) = 0;  

    JD_IMETHOD_(JDint32) JD_GetError(void) = 0;
   
    JD_IMETHOD_(int) JDFileDesc_To_FD(void* pr) = 0;
};

#endif // _IUNIXSERVICE_H_
