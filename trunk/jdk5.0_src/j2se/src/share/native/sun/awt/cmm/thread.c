/*
 * @(#)thread.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	File:			thread.c

	Contains:       OS Independent thread creation, termination and 
					manipulation functions

	Copyright (c) 1994 Eastman Kodak Company, all rights reserved.

	SCCSID = "6/25/98   @(#)thread.c	1.3";

*/

#include "kcms_sys.h"
#include "sithread.h"

#if defined (KPSOLARIS)
#include <thread.h>
#include <signal.h>
#include <errno.h>
#endif
#if defined (KPLINUX)
#include <pthread.h>
#include <signal.h>
#include <asm/errno.h>
#endif

/***************************************************************************
 * Windows (32 bit) Thread Creation, Termination and Manipulation functions
 ***************************************************************************/
#if defined (KPWIN32)

/******************************************************************
 * KpThreadCreate (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function creates a new thread which starts execution at the 
 *	function specified by startFunc.  The function startFunc receives
 *  one 32 bit argument specified by arg.  The new thread has its own 
 *	stack the size of which is specified by stackSize.  If stackSize is
 *	zero then the default stack size is used.  The KpThreadFlags structure 
 *	pointed to by flags is used to specify the security attributes and 
 *	creation flags for the thread.  If flags is NULL then the default 
 *	security attributes and creation flags are used.
 *
 *	The argument stackBase is not used of this version of the function
 *	and must be NULL
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t
FAR PASCAL
KpThreadCreate (KpThrStartFunc startFunc, KpGenericPtr_t arg, 
				KpGenericPtr_t stackBase, KpInt32_t stackSize, 
				KpThreadFlags_p flags)

{

KpThread_t				threadHdl;
KpUInt32_t				threadId, lclFlags;
LPSECURITY_ATTRIBUTES	securityAttr;

								/* stackBase must be NULL of windows version  */

	if (NULL != stackBase) {
		return (NULL);
	}

								/* Create the thread  */

	if (NULL == flags) {
		securityAttr = NULL;
		lclFlags = 0;
	}
	else {
		securityAttr = flags->SecurityAttr;
		lclFlags = flags->CreationFlags;
	}
	threadHdl = CreateThread (securityAttr, stackSize, 
							  (LPTHREAD_START_ROUTINE) startFunc, 
							  arg, lclFlags, &threadId);
	return (threadHdl);

} /* KpThreadCreate */


/******************************************************************
 * KpThreadExit (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function causes the currently executing thread to exit with the 
 *	exit code specified by exitCode 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
FAR PASCAL
KpThreadExit (KpInt32_t exitCode)
{

	ExitThread (exitCode);

} /* KpThreadExit */


/******************************************************************
 * KpThreadTerminate (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function causes the thread specified the thread to be terminated.  
 *	The terminated thread will return the exit code specified by exitCode. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
FAR PASCAL
KpThreadTerminate (KpThread_t thread, KpInt32_t exitCode)
{
BOOL	retVal;

	retVal = TerminateThread (thread, exitCode);
	if (TRUE != retVal) {
		return (KCMS_FAIL);
	}

	return (KCMS_SUCCESS);

} /* KpThreadTerminate */ 


/******************************************************************
 * KpThreadSetPriority (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function sets the priority of the thread specified by thread to
 *  the value specified by newPriority. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
FAR PASCAL
KpThreadSetPriority (KpThread_t thread, KpInt32_t newPriority)
{

BOOL		retVal;

	retVal = SetThreadPriority (thread, (int) newPriority);
	if (TRUE != retVal) {
		return (KCMS_FAIL);
	}
	
	return (KCMS_SUCCESS);
	
} /* KpThreadSetPriority */	

/******************************************************************
 * KpThreadYield (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function causes the calling thread to yield control to 
 *	another thread if one of equal or higher priority is ready
 *	to run. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
FAR PASCAL
KpThreadYield (void)
{

	Sleep (0);
		
} /* KpThreadYield */	

/******************************************************************
 * KpThreadWait (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function suspends execution of the current thread and waits until
 *	one or all of the specified threads terminates.  The argument 
 *	threadArray is a pointer to an array of threads for which this function 
 *	will wait for.  The number of threads in the array is specified by 
 *	numThreads.  If the value of waitMode is THREAD_WAIT_ALL then the 
 *	current thread will be suspended until all of the specified threads 
 *	terminate.  If the value of waitMode is THREAD_WAIT_ANY then the current
 *	thread will be suspended until any one of the specified threads
 *	terminates.  The index into the thread array of the thread which was 
 *	terminated is returned in the location pointed to by whichThread.  The 
 *	timeout argument is used to specify a maximum time (in milliseconds) to 
 *	wait for the threads to terminate.  A value of THREAD_TIMEOUT_INFINITE 
 *	causes the thread to wait forever.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
FAR PASCAL
KpThreadWait (KpThread_p threadArray, KpInt32_t numThreads,
		   	  KpInt32_t waitMode, KpUInt32_t timeout, 
		   	  KpInt32_p whichThread)
{
 
BOOL		waitFlag;
KpInt32_t	retVal = 0;
KpUInt32_t	myTimeout;

								/* Determine the Wait Mode */

	switch (waitMode) {
		
		case THREAD_WAIT_ONE:	/* THREAD_WAIT_ONE was added for SUN */
		case THREAD_WAIT_ALL:
			waitFlag = TRUE;
			break;
			
		case THREAD_WAIT_ANY:
			waitFlag = FALSE;
			break;
						
		default:
			return (KCMS_FAIL);
			
	} /* switch waitMode */
	
								/* Determine the timeout */
								
	if (THREAD_TIMEOUT_INFINITE == timeout) {
		myTimeout = INFINITE;
	}
	else {
		myTimeout = timeout;
	}
	
								/* Now wait */
								
	retVal = WaitForMultipleObjects (numThreads, threadArray, waitFlag, 
									 myTimeout);
	if (WAIT_FAILED == retVal) {
		return (KCMS_FAIL);
	}
	
	if (NULL != whichThread) {
		*whichThread = retVal - WAIT_OBJECT_0;
	}

	return (KCMS_SUCCESS);
	
} /* KpThreadWait */		
 
  
/******************************************************************
 * KpThreadDestroy (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function destroys the thread object which was created by
 *	KpThreadCreate.  The argument thread specifies which thread object is
 *	to be destroyed.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
FAR PASCAL
KpThreadDestroy (KpThread_t thread)
{

BOOL	retVal;

	retVal = CloseHandle (thread);
	if (TRUE != retVal) {
		return (KCMS_FAIL);
	}

	return (KCMS_SUCCESS);

} /* KpThreadDestroy */

/******************************************************************
 * KpGetCurrentThread (Windows 32 Bit Version)
 * 
 * Description
 *	
 *	This function returns the currently executing Thread
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t 
FAR PASCAL
KpGetCurrentThread (void)
{

	return (KpThread_t) GetCurrentThread ();

} /* KpGetCurrentThread */


#endif /* KPWIN32 */

/*******************************************************************
 * Solaris Thread Creation, Termination and Manipulation functions
 *******************************************************************/
#if defined (KPSOLARIS)

/******************************************************************
 * KpThreadCreate (Solaris Version)
 * 
 * Description
 *	
 *	This function creates a new thread which starts execution at the 
 *	function specified by startFunc.  The function startFunc receives
 *  one 32 bit argument specified by arg.  The new thread has its own 
 *	stack starting at the location specified by stackBase and continuing
 *	for stackSize bytes.  If stackBase is NULL, a new stack is created.  If
 *	stackSize is zero then the default stack size is used.  The 
 *	KpThreadFlags structure pointed to by flags is used to specify the 
 *	security attributes and creation flags for the thread.  If flags is NULL 
 *	then the default security attributes and creation flags are used.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t
KpThreadCreate (KpThrStartFunc startFunc, KpGenericPtr_t arg, 
				KpGenericPtr_t stackBase, KpInt32_t stackSize, 
				KpThreadFlags_p flags)

{

KpThread_t		thread;
KpInt32_t		lclFlags;
int				retVal;

								/* Create the thread  */

	if (NULL == flags) {
		lclFlags = 0;
	}
	else {
		lclFlags = flags->CreationFlags;
	}
	retVal = thr_create (stackBase, stackSize, startFunc, arg, 
						 lclFlags, &thread);
	if (0 != retVal) {
		return (NULL);
	}

	return (thread);

} /* KpThreadCreate */


/******************************************************************
 * KpThreadExit (Solaris Version)
 * 
 * Description
 *	
 *	This function causes the currently executing thread to exit.   
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
KpThreadExit (KpInt32_t exitCode)
{

	thr_exit (&exitCode);

} /* KpThreadExit */


/******************************************************************
 * KpThreadTerminate (Solaris Version)
 * 
 * Description
 *	
 *	This function causes the thread specified by thread to be terminated.  
 *
 *	The exitCode argument is ignored for this version
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
KpThreadTerminate (KpThread_t thread, KpInt32_t exitCode)
{
int	retVal;

	exitCode = 0;		/* eliminate compiler warning */

	retVal = thr_kill (thread, SIGKILL);
	if (0 != retVal) {
		return (KCMS_FAIL);
	}

	return (KCMS_SUCCESS);

} /* KpThreadTerminate */ 


/******************************************************************
 * KpThreadSetPriority (Solaris Version)
 * 
 * Description
 *	
 *	This function sets the priority of the thread specified by thread to
 *  the value specified by newPriority. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
KpThreadSetPriority (KpThread_t thread, KpInt32_t newPriority)
{

int		retVal;

	retVal = thr_setprio (thread, newPriority);
	if (0 != retVal) {
		return (KCMS_FAIL);
	}
	
	return (KCMS_SUCCESS);
	
} /* KpThreadSetPriority */

/******************************************************************
 * KpThreadYield (Solaris Version)
 * 
 * Description
 *	
 *	This function causes the calling thread to yield control to 
 *	another thread if one of equal or higher priority is ready
 *	to run. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
KpThreadYield (void)
{

	thr_yield ();
		
} /* KpThreadYield */	

/******************************************************************
 * KpThreadWait (Solaris Version)
 * 
 * Description
 *	
 *	This function suspends execution of the current thread and waits until
 *	one or all of the specified threads terminates.  The argument 
 *	threadArray is a pointer to an array of threads for which this function 
 *	will wait for.  The number of threads in the array is specified by 
 *	numThreads.  If the value of waitMode is THREAD_WAIT_ALL then the 
 *	current thread will be suspended until all of the specified threads 
 *	terminate.  If the value of waitMode is THREAD_WAIT_ANY then the current
 *	thread will be suspended until any one of the specified threads
 *	terminates.  The index into the thread array of the thread which was 
 *	terminated is returned in the location pointed to by whichThread.  The 
 *	timeout argument is used to specify a maximum time (in milliseconds) to 
 *	wait for the threads to terminate.  A value of THREAD_TIMEOUT_INFINITE 
 *	causes the thread to wait forever.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
KpThreadWait (KpThread_t threadArray[], KpInt32_t numThreads,
 			  KpInt32_t waitMode, KpUInt32_t timeout,
 			  KpInt32_p whichThread)
{
 
KpInt32_t	retVal, numWaitingFor;
KpIndex_t	index;
KpThread_t	ThreadID, terminatedThread;
KpInt32_t	TStatus;
void		*ThrStat = &TStatus;
								/* Determine the Wait Mode */

	ThreadID = (KpThread_t)0;
	switch (waitMode) {
		
		case THREAD_WAIT_ALL:
			numWaitingFor = numThreads;
			break;
			
		case THREAD_WAIT_ANY:
			numWaitingFor = 1;
			break;
			
		case THREAD_WAIT_ONE:
			ThreadID = threadArray[0];
			numWaitingFor = 1;
			break;
			
		default:
			return (KCMS_FAIL);
			
	} /* switch waitMode */
	
	/* Now wait for a thread to terminate.  
	   When one terminates and it is one we are
	   waiting for then decrement numWaitingFor.
	   If numWaitingFor is not zero then wait 
	   for another thread to terminate			 */
			
	while (numWaitingFor) {
	
		retVal = thr_join (ThreadID, &terminatedThread, &ThrStat);
		if (0 == retVal) {
	
	/* One of the threads in the process has
	   terminated.  Determine if it is one that
	   we are waiting on.						*/
								   
			for (index = 0; index < numThreads; index++){
				if (threadArray[index] == terminatedThread) {
					numWaitingFor--;
					if (NULL != whichThread) {
						*whichThread = index;
					}
					break;			/* Break out of for loop only */
				}
			}
		} else
		if (retVal == ESRCH) /* Thread already terminated */
		{		     /* Can only be reached for exact
					thread join */
			numWaitingFor--;
			if (NULL != whichThread) 
				*whichThread = 0;
		} else
		{
			return (KCMS_FAIL);
		}

	} /* while numWaitingFor */

	return (KCMS_SUCCESS);
	
} /* KpThreadWait */		
 
  /******************************************************************
 * KpThreadDestroy (Solaris Version)
 * 
 * Description
 *	
 *	This function always returns KCMS_SUCCESS for solaris because the
 *	thread object is destroyed automatically.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
KpThreadDestroy (KpThread_t thread)
{

KpInt32_t	retVal;

	thread = 0;
	return (KCMS_SUCCESS);

} /* KpThreadDestroy */

/******************************************************************
 * KpGetCurrentThread (Solaris Version)
 * 
 * Description
 *	
 *	This function returns the currently executing Thread
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t 
KpGetCurrentThread (void)
{

	return (KpThread_t) thr_self();

} /* KpGetCurrentThread */
#endif /*KPSOLARIS */

/*******************************************************************
 * LINUX Thread Creation, Termination and Manipulation functions
 *******************************************************************/
#if defined (KPLINUX)

/******************************************************************
 * KpThreadCreate (LINUX Version)
 * 
 * Description
 *	
 *	This function creates a new thread which starts execution at the 
 *	function specified by startFunc.  The function startFunc receives
 *  one 32 bit argument specified by arg.  The new thread has its own 
 *	stack starting at the location specified by stackBase and continuing
 *	for stackSize bytes.  If stackBase is NULL, a new stack is created.  If
 *	stackSize is zero then the default stack size is used.  The 
 *	KpThreadFlags structure pointed to by flags is used to specify the 
 *	security attributes and creation flags for the thread.  If flags is NULL 
 *	then the default security attributes and creation flags are used.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t
KpThreadCreate (KpThrStartFunc startFunc, KpGenericPtr_t arg, 
				KpGenericPtr_t stackBase, KpInt32_t stackSize, 
				KpThreadFlags_p flags)

{

KpThread_t		thread;
pthread_attr_t		attr;
int				retVal;

								/* Create the thread  */

	if (NULL == flags) { }

	pthread_attr_init(&attr);
	retVal = pthread_create (&thread, &attr,
				startFunc, arg);
	if (0 != retVal) {
		return (0);
	}

	return (thread);

} /* KpThreadCreate */


/******************************************************************
 * KpThreadExit (LINUX Version)
 * 
 * Description
 *	
 *	This function causes the currently executing thread to exit.   
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
KpThreadExit (KpInt32_t exitCode)
{

	pthread_exit (&exitCode);

} /* KpThreadExit */


/******************************************************************
 * KpThreadTerminate (LINUX Version)
 * 
 * Description
 *	
 *	This function causes the thread specified by thread to be terminated.  
 *
 *	The exitCode argument is ignored for this version
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
KpThreadTerminate (KpThread_t thread, KpInt32_t exitCode)
{
int	retVal;

	exitCode = 0;		/* eliminate compiler warning */

	retVal = pthread_kill (thread, SIGKILL);
	if (0 != retVal) {
		return (KCMS_FAIL);
	}

	return (KCMS_SUCCESS);

} /* KpThreadTerminate */ 


/******************************************************************
 * KpThreadSetPriority (LINUX Version)
 * 
 * Description
 *	
 *	This function sets the priority of the thread specified by thread to
 *  the value specified by newPriority. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpInt32_t
KpThreadSetPriority (KpThread_t thread, KpInt32_t newPriority)
{

int		retVal;
struct sched_param  SchParm;

	SchParm.sched_priority = newPriority;
	retVal = pthread_setschedparam (thread, SCHED_OTHER, &SchParm);
	if (0 != retVal) {
		return (KCMS_FAIL);
	}
	
	return (KCMS_SUCCESS);
	
} /* KpThreadSetPriority */

/******************************************************************
 * KpThreadYield (LINUX Version)
 * 
 * Description
 *	
 *	This function causes the calling thread to yield control to 
 *	another thread if one of equal or higher priority is ready
 *	to run. 
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
void
KpThreadYield (void)
{
int	retVal;

	retVal = sched_yield ();
		
} /* KpThreadYield */	

/******************************************************************
 * KpThreadWait (LINUX Version)
 * 
 * Description
 *	
 *	This function suspends execution of the current thread and waits until
 *	one or all of the specified threads terminates.  The argument 
 *	threadArray is a pointer to an array of threads for which this function 
 *	will wait for.  The number of threads in the array is specified by 
 *	numThreads.  If the value of waitMode is THREAD_WAIT_ALL then the 
 *	current thread will be suspended until all of the specified threads 
 *	terminate.  If the value of waitMode is THREAD_WAIT_ANY then the current
 *	thread will be suspended until any one of the specified threads
 *	terminates.  The index into the thread array of the thread which was 
 *	terminated is returned in the location pointed to by whichThread.  The 
 *	timeout argument is used to specify a maximum time (in milliseconds) to 
 *	wait for the threads to terminate.  A value of THREAD_TIMEOUT_INFINITE 
 *	causes the thread to wait forever.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
KpThreadWait (KpThread_t threadArray[], KpInt32_t numThreads,
 			  KpInt32_t waitMode, KpUInt32_t timeout,
 			  KpInt32_p whichThread)
{
 
KpInt32_t	retVal, numWaitingFor;
KpIndex_t	index;
KpThread_t	ThreadID, terminatedThread;
void		*ThrStat = &terminatedThread;
								/* Determine the Wait Mode */

	ThreadID = (KpThread_t)0;
	switch (waitMode) {
		
		case THREAD_WAIT_ALL:
			numWaitingFor = numThreads;
			break;
			
		case THREAD_WAIT_ANY:
			numWaitingFor = 1;
			break;
			
		case THREAD_WAIT_ONE:
			ThreadID = threadArray[0];
			numWaitingFor = 1;
			break;
			
		default:
			return (KCMS_FAIL);
			
	} /* switch waitMode */
	
	/* Now wait for a thread to terminate.  
	   When one terminates and it is one we are
	   waiting for then decrement numWaitingFor.
	   If numWaitingFor is not zero then wait 
	   for another thread to terminate			 */
			
	while (numWaitingFor) {
	
		retVal = pthread_join (ThreadID, (void *)&ThrStat);
		if (0 == retVal) {
	
	/* One of the threads in the process has
	   terminated.  Determine if it is one that
	   we are waiting on.						*/
								   
			for (index = 0; index < numThreads; index++){
				if (threadArray[index] == terminatedThread) {
					numWaitingFor--;
					if (NULL != whichThread) {
						*whichThread = index;
					}
					break;			/* Break out of for loop only */
				}
			}
		} else
		if (retVal == ESRCH) /* Thread already terminated */
		{		     /* Can only be reached for exact
					thread join */
			numWaitingFor--;
			if (NULL != whichThread) 
				*whichThread = 0;
		} else
		{
			return (KCMS_FAIL);
		}

	} /* while numWaitingFor */

	return (KCMS_SUCCESS);
	
} /* KpThreadWait */		
 
  /******************************************************************
 * KpThreadDestroy (LINUX Version)
 * 
 * Description
 *	
 *	This function always returns KCMS_SUCCESS for solaris because the
 *	thread object is destroyed automatically.
 *
 * Author
 *	mjb
 *
 * Created
 *	July 18, 1994
 *****************************************************************************/
KpInt32_t
KpThreadDestroy (KpThread_t thread)
{


	thread = 0;
	return (KCMS_SUCCESS);

} /* KpThreadDestroy */

/******************************************************************
 * KpGetCurrentThread (LINUX Version)
 * 
 * Description
 *	
 *	This function returns the currently executing Thread
 *
 * Author
 *	mjb
 *
 * Created
 *	July 5, 1994
 *****************************************************************************/
KpThread_t 
KpGetCurrentThread (void)
{

	return (KpThread_t) pthread_self();

} /* KpGetCurrentThread */

#endif /* KPLINUX */
