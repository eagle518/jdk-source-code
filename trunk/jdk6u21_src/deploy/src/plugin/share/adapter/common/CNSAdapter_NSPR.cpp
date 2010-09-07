/*
 * @(#)CNSAdapter_NSPR.cpp	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * In order to make the Unix core file to be reused by both
 * Netscape 4 and Netscape 6 +, we abstract all the NSPR dependencies
 * to the adapter layer
 */
#include "JDNsprData.h"
#include "CNSAdapter_NSPR.h"


JD_METHOD_(void*)  CNSAdapter_NSPR::JD_GetCurrentThread(void)
{
  return PR_GetCurrentThread();
}
    
    
JD_METHOD_(void*)  CNSAdapter_NSPR::JD_NewMonitor(void)
{
  return PR_NewMonitor();
}

JD_METHOD_(void)  CNSAdapter_NSPR::JD_DestroyMonitor(void* mon)
{
  PR_DestroyMonitor((PRMonitor*)mon);
}

JD_METHOD_(void)  CNSAdapter_NSPR::JD_EnterMonitor(void* mon)
{
  PR_EnterMonitor((PRMonitor*)mon);
}
    
JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_ExitMonitor(void* mon)
{
  return PR_ExitMonitor((PRMonitor*)mon);
}

JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_Wait(void* mon, JDUint32 ticks)
{
  return PR_Wait((PRMonitor*)mon, (PRIntervalTime)ticks);
}
    
JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_NotifyAll(void* mon)
{
  return PR_NotifyAll((PRMonitor*)mon);
}
    
JD_METHOD_(void*)  CNSAdapter_NSPR::JD_NewTCPSocket(void)
{
  return PR_NewTCPSocket();
}
    
JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_NewTCPSocketPair(void *fd[2])
{
  return PR_NewTCPSocketPair((PRFileDesc**)fd);
}
    
JD_METHOD_(void*)  CNSAdapter_NSPR::JD_Socket(JDint32 domain, JDint32 type, JDint32 proto)
{
  return PR_Socket((PRInt32)domain, (PRInt32)type, (PRInt32)proto);
}

JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_CreatePipe(void** readPipe, void** writePipe)
{
  return PR_CreatePipe((PRFileDesc**)readPipe, (PRFileDesc**)writePipe);
}

JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_Bind(void* fd, void* addr)
{
  return PR_Bind((PRFileDesc*)fd, (PRNetAddr*)addr);
}

void*  CNSAdapter_NSPR::JD_Accept(void* fd, void* addr, JDUint32 timeout)
{
  return PR_Accept((PRFileDesc*)fd, (PRNetAddr*)addr, (PRIntervalTime)timeout);
}
        
JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_Close(void* fd)
{
  return PR_Close((PRFileDesc*)fd);
}

JD_METHOD_(JDint32)  CNSAdapter_NSPR::JD_Read(void* fd, void* buf, JDint32 amount)
{
  return PR_Read((PRFileDesc*)fd, buf, (PRInt32)amount);
}
    
JD_METHOD_(JDint32)  CNSAdapter_NSPR::JD_Write(void* fd, const void* buf, JDint32 amount)
{
  return PR_Write((PRFileDesc*)fd, buf, (PRInt32)amount);
}

JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_Sync(void* fd)
{
  return PR_Sync((PRFileDesc*)fd);
}

JD_METHOD_(JDBool) CNSAdapter_NSPR::JD_Listen(void* fd, JDIntn backlog)
{
  return PR_Listen((PRFileDesc*)fd, (PRIntn)backlog);
}
    
JD_METHOD_(JDint32)  CNSAdapter_NSPR::JD_Available(void* fd)
{
  return PR_Available((PRFileDesc*)fd);
}

JD_METHOD_(JDint32)   CNSAdapter_NSPR::JD_Poll(struct JDPollDesc pds[], int npds, JDUint32 JDIntervalTime)
{
  return PR_Poll((PRPollDesc*)pds, (PRIntn)npds, (PRIntervalTime)JDIntervalTime);
}
    

JD_METHOD_(void*)  CNSAdapter_NSPR::JD_CreateThread(JDThreadType type,
						     void (*start)(void* arg),
						     void* arg,
						     JDThreadPriority priority,
						     JDThreadScope    scope,
						     JDThreadState    state,
						     JDUint32 stackSize)
{
  return PR_CreateThread((PRThreadType)type,
			 start,
			 arg,
			 (PRThreadPriority)priority,
			 (PRThreadScope)scope,
			 (PRThreadState)state,
			 (PRUint32)stackSize);
}

JD_METHOD_(JDint32) CNSAdapter_NSPR::JD_GetError(void)
{
  return (JDint32)PR_GetError();
}
   
JD_METHOD_(int) CNSAdapter_NSPR::JDFileDesc_To_FD(void* pr)
{
  PRFileDesc* temp = (PRFileDesc*)pr;
  return (temp)->secret->md.osfd;
}
    

