/*
 * @(#)sysShmem.h	1.11 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _JAVASOFT_SYSSHMEM_H

#include <jni.h>
#include "sys.h"
#include "shmem_md.h"

int sysSharedMemCreate(const char *name, int length, sys_shmem_t *, void **buffer);
int sysSharedMemOpen(const char *name,  sys_shmem_t *, void **buffer);
int sysSharedMemClose(sys_shmem_t, void *buffer);

/* Mutexes that can be used for inter-process communication */
int sysIPMutexCreate(const char *name, sys_ipmutex_t *mutex);
int sysIPMutexOpen(const char *name, sys_ipmutex_t *mutex);
int sysIPMutexEnter(sys_ipmutex_t mutex, sys_event_t event);
int sysIPMutexExit(sys_ipmutex_t mutex);
int sysIPMutexClose(sys_ipmutex_t mutex);

/* Inter-process events */
int sysEventCreate(const char *name, sys_event_t *event, jboolean manualreset);
int sysEventOpen(const char *name, sys_event_t *event);
int sysEventWait(sys_process_t otherProcess, sys_event_t event, long timeout);
int sysEventSignal(sys_event_t event);
int sysEventClose(sys_event_t event);

jlong sysProcessGetID();
int sysProcessOpen(jlong processID, sys_process_t *process);
int sysProcessClose(sys_process_t *process);

/* access to errno or equivalent */
int sysGetLastError(char *buf, int size);

/* access to thread-local storage */
int sysTlsAlloc();
void sysTlsFree(int index);
void sysTlsPut(int index, void *value);
void *sysTlsGet(int index);

/* misc. functions */
void sysSleep(long duration);

#endif


