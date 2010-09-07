/*
 * @(#)sithread.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	File:			sithread.h

	Contains:       Typedefs and prototypes for OS Independent thread creation, 
					termination and manipulation functions

	Copyright (c) 1994 Eastman Kodak Company, all rights reserved.

	SCCSID = "1/9/98   @(#)sithread.h	1.2";

*/

#ifndef _SITHREAD_H_
#define _SITHREAD_H_

#if defined (KPSOLARIS)
#include <thread.h>
#include <signal.h>
#endif /* KPSOLARIS */

#if defined (KPLINUX)
#include <pthread.h>
#endif

#if defined (KPDU)
#include <signal.h>
#endif /* KPDU */

/***********************
 * Common definitons
 ***********************/
 
#define THREAD_WAIT_ANY		0
#define THREAD_WAIT_ALL		1
#define THREAD_WAIT_ONE		2

#define THREAD_TIMEOUT_INFINITE		0xffffffff
 
typedef void * (*KpThrStartFunc)(void *);

/******************************
 * Windows 32 Bit definitons
 ******************************/ 
#if defined (KPWIN32)

typedef HANDLE KpThread_t, FAR * KpThread_p;

typedef struct KpThreadFlags_s {
		LPSECURITY_ATTRIBUTES	SecurityAttr;
		KpInt32_t				CreationFlags;
} KpThreadFlags_t, FAR * KpThreadFlags_p, FAR * FAR * KpThreadFlags_h;

#endif /* KPWIN32 */

/******************************
 * Solaris definitons
 ******************************/ 
#if defined (KPSOLARIS)

typedef thread_t KpThread_t, *KpThread_p;

typedef struct KpThreadFlags_s {
		KpInt32_t				CreationFlags;
} KpThreadFlags_t, FAR * KpThreadFlags_p, FAR * FAR * KpThreadFlags_h;

#endif /* KPSOLARIS */

/******************************
 * Linux definitons
 ******************************/ 
#if defined (KPLINUX)

typedef pthread_t KpThread_t, *KpThread_p;

typedef struct KpThreadFlags_s {
		KpInt32_t				CreationFlags;
} KpThreadFlags_t, FAR * KpThreadFlags_p, FAR * FAR * KpThreadFlags_h;

#endif /* KPLINUX */

/***********************
 * Function Prototypes 
 ***********************/
KpThread_t FAR PASCAL	KpThreadCreate (KpThrStartFunc startFunc, KpGenericPtr_t arg, 
						    			KpGenericPtr_t stackBase, KpInt32_t stackSize, 
						    			KpThreadFlags_p flags);
void FAR PASCAL			KpThreadExit (KpInt32_t exitCode);
KpInt32_t FAR PASCAL	KpThreadTerminate (KpThread_t thread, KpInt32_t exitCode);
KpInt32_t FAR PASCAL	KpThreadSetPriority (KpThread_t thread, KpInt32_t newPriority);
void FAR PASCAL			KpThreadYield (void);
KpInt32_t FAR PASCAL	KpThreadWait (KpThread_t threadArray[], KpInt32_t numThreads,
 			  			  			  KpInt32_t waitMode, KpUInt32_t timeout,
 			  			  			  KpInt32_p whichThread);
KpInt32_t FAR PASCAL	KpThreadDestroy (KpThread_t thread);
KpThread_t FAR PASCAL	KpGetCurrentThread (void);

#endif /* _SITHREAD_H_ */
