/*
 * @(#)CNSAdapter_NSPR.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * In order to make the Unix core file to be reused by both
 * Netscape 4 and Netscape 6 +, we abstract all the NSPR dependencies
 * to the adapter layer
 */

#ifndef _CNSADAPTER_NSPR_H_
#define _CNSADAPTER_NSPR_H_

#include "IUnixService.h"
#include "nspr.h"
#include "private/pprio.h"

/* from nsprpub/pr/include/md/_unixos.h */
struct _MDFileDesc {
    int osfd;
};

/* From nsprpub/pr/include/private/primpl.h */
/* Mozilla changes: This struct has changed */

typedef struct _MDFileDesc _MDFileDesc;
struct PRFilePrivate {
    PRInt32 state;
    PRBool nonblocking;
    PRBool inheritable;
    PRFileDesc *next;
    PRIntn lockCount;
    _MDFileDesc md;
#ifdef _PR_PTHREADS
    PRIntn eventMask[1];
#endif
};

class CNSAdapter_NSPR : public IUnixService {

public:
  
  JD_IMETHOD_(void*)    JD_GetCurrentThread(void);
    
  JD_IMETHOD_(void*)    JD_NewMonitor(void);
  JD_IMETHOD_(void)     JD_DestroyMonitor(void* mon);

  JD_IMETHOD_(void)     JD_EnterMonitor(void* mon);
  JD_IMETHOD_(JDBool)   JD_ExitMonitor(void* mon);
  JD_IMETHOD_(JDBool)   JD_Wait(void* mon, JDUint32 ticks);
  JD_IMETHOD_(JDBool)   JD_NotifyAll(void* mon);
    
  JD_IMETHOD_(void*)    JD_NewTCPSocket(void);
  JD_IMETHOD_(JDBool)   JD_NewTCPSocketPair(void *fd[2]);
  JD_IMETHOD_(void*)    JD_Socket(JDint32 domain, JDint32 type, JDint32 proto);
  JD_IMETHOD_(JDBool)   JD_CreatePipe(void** readPipe, void** writePipe);
  JD_IMETHOD_(JDBool)   JD_Bind(void* fd, void* addr);
  JD_IMETHOD_(void*)    JD_Accept(void* fd, void* addr, JDUint32 timeout );
        
  JD_IMETHOD_(JDBool)   JD_Close(void* fd);
  JD_IMETHOD_(JDint32)  JD_Read(void* fd, void* buf, JDint32 amount);
  JD_IMETHOD_(JDint32)  JD_Write(void* fd, const void* buf, JDint32 amount);
  JD_IMETHOD_(JDBool)   JD_Sync(void* fd);
  JD_IMETHOD_(JDBool)   JD_Listen(void* fd, JDIntn backlog);
    
  JD_IMETHOD_(JDint32)  JD_Available(void* fd);
  JD_IMETHOD_(JDint32)  JD_Poll(struct JDPollDesc pds[], int npds, JDUint32 JDIntervalTim);
    

  JD_IMETHOD_(void*)    JD_CreateThread(JDThreadType type,
					void (*start)(void* arg),
					void* arg,
					JDThreadPriority priority,
					JDThreadScope    scope,
					JDThreadState    state,
					JDUint32 stackSize);  

  JD_IMETHOD_(JDint32) JD_GetError(void);
   
  JD_IMETHOD_(int) JDFileDesc_To_FD(void* pr);
};

#endif // _CNSADAPTER_NSPR_H_
  
