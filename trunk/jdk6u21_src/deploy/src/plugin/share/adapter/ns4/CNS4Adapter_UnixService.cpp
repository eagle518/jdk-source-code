/*
 * @(#)CNS4Adapter_UnixService.cpp	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * In order to make the Unix core file to be reused by both
 * Netscape 4 and Netscape 6 +, we abstract all the NSPR dependencies
 * to the adapter layer
 * Comment by XLU
 */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/param.h>		// For MAXPATHLEN
#include <fcntl.h>
#include <stropts.h>
#include <string.h>
#include <stdarg.h>
#include <poll.h>
#include <dlfcn.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/dirent.h>
#define _DIRENTRY_H
#else
#include <ulimit.h>
#endif
#include <dirent.h>

#include <dlfcn.h>
#include <locale.h>
#include <libintl.h>
#include "JDNsprData.h"
#include "JDSupportUtils.h"
#include "CNS4Adapter_UnixService.h"

#define trace(s)
#define tracing 0
/* Portable implementation of pipe */
#ifndef SVR4

int s_pipe(int fds[2]) {
    int res = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (tracing) {
	int flags0 = fcntl(fds[0], F_GETFL);
	int flags1 = fcntl(fds[1], F_GETFL);
	trace("Not SVR4 Flgs0 = %X Flgs1 = %X appnd=%X nonblck=%X sync=%X\n", 
	      flags0, flags1, O_APPEND, O_NONBLOCK, O_SYNC);
    }
    return res; 
}

#else

#ifdef __linux__

/* This apparently does not seem to work on Linux. Need to use a socketpair
   insted of pipe, as the nsIPlugin <--> JPI uses a bidirectional comm
   sandeep.konchady@eng.sun.com 05/09/2000.
*/
int s_pipe(int fds[2]) {
    int res = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (tracing) {
	int flags0 = fcntl(fds[0], F_GETFL);
	int flags1 = fcntl(fds[1], F_GETFL);
	/*
	trace("SVR4 Flgs0 = %X Flgs1 = %X appnd=%X nonblck=%X sync=%X\n", 
	      flags0, flags1, O_APPEND, O_NONBLOCK, O_SYNC);
	*/
    }
    return res; 
}
#else
int s_pipe(int fds[2]) {
    int res = (pipe(fds));
    if (tracing) {
	int flags0 = fcntl(fds[0], F_GETFL);
	int flags1 = fcntl(fds[1], F_GETFL);
	
    }
    return res;

}
#endif
#endif


/* Not implemented for Netscape 4*/
JD_METHOD_(void*)  CNS4Adapter_UnixService::JD_GetCurrentThread(void)
{
  return NULL;
}
    
JD_METHOD_(void*)  CNS4Adapter_UnixService::JD_NewMonitor(void)
{
  return NULL;
}

JD_METHOD_(void)  CNS4Adapter_UnixService::JD_DestroyMonitor(void* mon)
{
  return;
}

JD_METHOD_(void)  CNS4Adapter_UnixService::JD_EnterMonitor(void* mon)
{
  return;
}
    
JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_ExitMonitor(void* mon)
{
  return JD_FALSE;
}

JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_Wait(void* mon, JDUint32 ticks)
{
  return JD_FALSE;
}
    
JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_NotifyAll(void* mon)
{
  return JD_FALSE;
}
    
JD_METHOD_(void)*  CNS4Adapter_UnixService::JD_NewTCPSocket(void)
{
  return NULL;
}
    
JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_NewTCPSocketPair(void *fd[2])
{
  int rc = s_pipe((int*)fd);
  if (rc == 0)
    return JD_TRUE;

  return JD_FALSE;
}

/* Not useful for Netscape 4*/
JD_METHOD_(void*)  CNS4Adapter_UnixService::JD_Socket(JDint32 domain, JDint32 type, JDint32 proto)
{
  int fd = socket((int)domain, (int)type, (int)proto);
  return (void*)&fd;
}

JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_CreatePipe(void** readPipe, void** writePipe)
{
  static int fd[2];
  if (pipe(fd) == -1)
    return JD_FALSE;

  *readPipe =  (void*)fd[0];
  *writePipe = (void*)fd[1];

  return JD_TRUE;
}

JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_Bind(void* fd, void* addr)
{
  if (bind((int)fd, (struct sockaddr*)addr, sizeof(struct sockaddr)) == -1)
    return JD_FALSE;

  return JD_TRUE;
}

JD_METHOD_(void*)  CNS4Adapter_UnixService::JD_Accept(void* fd, void* addr, JDUint32 timeout)
{
  socklen_t len = (socklen_t)sizeof(struct sockaddr);

  int rc = accept((int)fd, (struct sockaddr*)addr, &len);

  return (void*)&rc;
}
        
JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_Close(void* fd)
{
  if (close((int)fd) == -1)
    return JD_FALSE;

  return JD_TRUE;
}

JD_METHOD_(JDint32)  CNS4Adapter_UnixService::JD_Read(void* fd, void* buf, JDint32 amount)
{
  return (JDint32)read((int)fd, buf, (size_t)amount);
}
    
JD_METHOD_(JDint32)  CNS4Adapter_UnixService::JD_Write(void* fd, const void* buf, JDint32 amount)
{
  return (JDint32)write((int)fd, buf, (size_t)amount);
}

JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_Sync(void* fd)
{
  return fsync((int)fd);
}

JD_METHOD_(JDBool) CNS4Adapter_UnixService::JD_Listen(void* fd, JDIntn backlog)
{
  if (listen((int)fd, (int)backlog) == -1)
    return JD_FALSE;

  return JD_TRUE;
}
    
JD_METHOD_(JDint32)  CNS4Adapter_UnixService::JD_Available(void* fd)
{
  return 0;
}

JD_METHOD_(JDint32)  CNS4Adapter_UnixService::JD_Poll(struct JDPollDesc pds[], int npds, JDUint32 JDIntervalTime)
{
  struct pollfd *fdArray = (struct pollfd*)malloc(npds * sizeof(struct pollfd));
  int i = 0, rc;

  if (fdArray == NULL)
    return -1;

  for (i = 0; i < npds; i++) {
    fdArray[i].fd = (int)(pds[i].fd);
    fdArray[i].events = (short)(pds[i].in_flags);
  }

  if ((rc = poll(fdArray, (unsigned long)npds, (int)JDIntervalTime)) > 0) {
    for (i = 0; i < npds; i++) 
      pds[i].out_flags = (JDint16)(fdArray[i].revents);
  }
  
  free(fdArray);
  fdArray = NULL;
    
  return rc;
}
    

  JD_METHOD_(void)*  CNS4Adapter_UnixService::JD_CreateThread(JDThreadType type,
				   void (*start)(void* arg),
				   void* arg,
				   JDThreadPriority priority,
				   JDThreadScope    scope,
				   JDThreadState    state,
				   JDUint32 stackSize)
{
  return NULL;
}

JD_METHOD_(JDint32) CNS4Adapter_UnixService::JD_GetError(void)
{
  return (JDint32)errno;
}
   
JD_METHOD_(int) CNS4Adapter_UnixService::JDFileDesc_To_FD(void* pr)
{
  return (int)pr;
}
