/*
 * @(#)mutex_md.h	1.20 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Win32 implementation of mutexes. Here we use critical sections as
 * our mutexes. We could have used mutexes, but mutexes are heavier
 * weight than critical sections. Mutexes and critical sections are
 * semantically identical, the only difference being that mutexes
 * can operate between processes (i.e. address spaces).
 *
 * It's worth noting that the Win32 functions supporting critical
 * sections do not provide any error information whatsoever (i.e.
 * all critical section routines return (void)).
 */

#ifndef _JAVASOFT_WIN32_MUTEX_MD_H_
#define _JAVASOFT_WIN32_MUTEX_MD_H_

#include <windows.h>

typedef CRITICAL_SECTION mutex_t;

#define mutexInit(m)	InitializeCriticalSection(m)
#define mutexDestroy(m)	DeleteCriticalSection(m)
#define mutexLock(m)	EnterCriticalSection(m)
#define mutexUnlock(m)	LeaveCriticalSection(m)

#endif /* !_JAVASOFT_WIN32_MUTEX_MD_H_ */
