/*
 * @(#)sync.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
*	@(#)sync.c	1.39 98/12/02

	Contains:	synchronization functions.

	Written by:	The Kodak CMS Team

 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1994 - 1998               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************

*/

#include "kcms_sys.h"
#include "sync.h"

#if defined (KPUNIX)
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif 

#if defined (KPMAC)
#include <Events.h>	/* for TickCount call */
#endif

/********************/
/* Local Prototypes */
/********************/

#if defined (KPWIN)
static BOOL KpUseSemaphores (void);
static KpUInt32_t	FAR getSemName (const KpChar_p 	baseName, 
									KpUInt32_t 		semNum, 
									KpUInt32_t 		bufSize,
		    						KpChar_p 		semName);
#endif

/********************/
/* Common Functions */
/********************/

#if !defined(KPMAC) && !defined(KPWIN16) && !defined(KPSGI) && !defined(KPSGIALL)
/*--------------------------------------------------------------------
 * DESCRIPTION 
 * This function fills in the semaphore initialization array pointed to by
 * initArray.  The number of entries in the array is specified by nEntries.
 * The argument SemInitArray points to an array of KpSemInit structures.
 * 
 * Each entry of initArray is initialized to 1.  If SemInitArray is NULL
 * then the function returns.  If SemInitArray is not NULL then this 
 * function walks through the array.  If SemaphoreNum of the current entry
 * in the array is not negative then InitialValue of that entry is placed
 * in the initArray location which correspondes to SemaphoreNum.  Then the
 * next entry in the SemInitArray is processed.  This continues until
 * either a negative Semaphore number is found or nEntries entries are
 * processed. 
 *
 * RETURNS
 * 	KCMS_SUCCESS if initArray filled in correctly
 *	KCMS_FAIL	 if an illegal semaphore number was found
 * 
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 23, 1994
 *-------------------------------------------------------------------*/
static KpUInt32_t createInitArray (
				KpSemInit_t			FAR * SemInitArray, 
				KpUInt32_t			nEntries,
				KpSemInitData_t		initArray[])
{
	KpUInt32_t		index;
	KpInt32_t		semNum;

								/*  initialize initArray to contain all 
									ones except for entry 0.  That entry 
									is the usage count.						*/

	initArray[0] = 0;
	for (index = 1; index < nEntries; index++)
		initArray [index] = 1;

								/*	if SemInitArray != NULL then walk 
									through that array.  There can be at
									most nEntries-1 initial values because
									the usage count semaphore can't be 
									explicitly initialized */

	if (NULL != SemInitArray) {

		for (index = 0; index < nEntries-1; index++) {

			semNum = SemInitArray [index].SemaphoreNum;
			if (semNum <= 0) {
				break;
			}

			if (semNum >= (KpInt32_t) nEntries)
				return KCMS_FAIL;

			initArray [semNum] = SemInitArray [index].InitialValue;

		} /* for index */

	} /* if SemInitArray */

	return KCMS_SUCCESS;

} /* createInitArray */

#endif /* KPMAC */

/***********************************************/
/* Timers and Delay functions (16 bit windows) */
/***********************************************/

#if defined (KPWIN16)

void FAR KpSleep (
			KpUInt32_t	MilliSeconds,
			KpInt32_t	IntentIsDelayOnly)
{
	DWORD	StopTicks;

	if (IntentIsDelayOnly) {

	/* GetTickCount returns 1000 ticks per second */
		StopTicks = GetTickCount () + MilliSeconds;
		while (GetTickCount () < StopTicks) {
		}
	}
}

#elif defined (KPWIN32)

void FAR KpSleep (
			KpUInt32_t	MilliSeconds,
			KpInt32_t	IntentIsDelayOnly)
{
	IntentIsDelayOnly = IntentIsDelayOnly;

	Sleep (MilliSeconds);
}

#elif defined (KPMAC)

void FAR KpSleep (
			KpUInt32_t	MilliSeconds,
			KpInt32_t	IntentIsDelayOnly)
{
	long	TickCounter;

	if (IntentIsDelayOnly) {

	/* TickCount returns 60 ticks per second */
		TickCounter = TickCount () + (60 * MilliSeconds) / 1000;
		while (TickCount () < TickCounter) {
		}
	}
}

#elif defined (KPUNIX)

void FAR KpSleep (
			KpUInt32_t	MilliSeconds,
			KpInt32_t	IntentIsDelayOnly)
{
	IntentIsDelayOnly = IntentIsDelayOnly;

	sleep (MilliSeconds / 1000);
}

#else

void FAR KpSleep (
			KpUInt32_t	MilliSeconds,
			KpInt32_t	IntentIsDelayOnly)
{
	IntentIsDelayOnly = IntentIsDelayOnly;
	MilliSeconds = MilliSeconds;
}

#endif

/************************************************************/
/* Process/Thread Identification functions (16 bit windows) */
/************************************************************/

#if defined (KPWIN16)
/*--------------------------------------------------------------------
 * DESCRIPTION (Win16 Version)
 * Get a Hopefully a Unique Thread ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentThreadId (void)
{
	return (KpInt32_t) (void FAR *) GetCurrentTask ();
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Win16 Version)
 * Get a Hopefully a Unique Process ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentProcessId (void)
{
	return (KpInt32_t) (void FAR *) GetCurrentTask ();
}

/************************************************************/
/* Process/Thread Identification functions (32 bit windows) */
/************************************************************/

#elif defined (KPWIN32)
/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 *
 * Get a Hopefully a Unique Thread ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentThreadId (void)
{
	return GetCurrentThreadId ();
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 *
 * Get a Hopefully a Unique Process ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentProcessId (void)
{
	return GetCurrentProcessId ();
}

/************************************************************/
/* Process/Thread Identification functions (Macintosh) */
/************************************************************/

#elif defined (KPMAC)

static KpInt32_t theProcessId = 0;
/*--------------------------------------------------------------------
 * DESCRIPTION (Macintosh Version)
 *
 * Set a Unique Process ID
 *
 * AUTHOR
 * msm 
 *
 * DATE CREATED
 * June 21, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpSetCurrentProcessId (KpInt32_t processId)
{
	theProcessId = processId;
	return KCMS_SUCCESS;
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Macintosh Version)
 *
 * Get a Hopefully a Unique Process ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentProcessId (void)
{
	return (theProcessId);
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Macintosh Version)
 *
 * Geta Hopefully a Unique Thread ID
 *
 * AUTHOR
 * lsh 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentThreadId (void)
{
	return 1;
}

/******************************************************************/
/* Process/Thread Identification functions (SunOS4.x and Solaris) */
/******************************************************************/

#elif defined (KPUNIX)
/*--------------------------------------------------------------------
 * DESCRIPTION (Sun Version)
 *
 * Get a Hopefully a Unique Thread ID
 *
 * AUTHOR
 * mjb 
 *
 * DATE CREATED
 * Jun 16, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentThreadId (void)
{

#if defined (KPSOLARIS) || defined (KPLINUX)
#if defined (KP_POSIX_THREADS)
	return ((KpInt32_t) pthread_self());
#else
	return ((KpInt32_t) thr_self());
#endif
#else
	return (1);
#endif

}

/*--------------------------------------------------------------------
 * DESCRIPTION (Sun Version)
 *
 * Get a Hopefully a Unique Process ID
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 16, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpGetCurrentProcessId (void)
{
	return ((KpInt32_t) getpid());
}
#endif


/*************************************************************/
/* Process/Thread Synchronization functions (32 Bit Windows) */
/*************************************************************/

#if defined (KPWIN32) && !defined(KPMSMAC)

/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 * This function initialize/enables the use of CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until this function is called. This function may not be
 * called again without first calling KpDeleteCriticalSection() within the
 * current process.
 *
 * Prior to the call, the CriticalFlag variable must have the SyncFlag member
 * initialized to zero. The whole variable declaration should be done in data
 * memory and not on the stack. The structure needs to be long word aligned
 * to work correctly for the PC.  The CritcalFlag variable must not be touched,
 * moved, washed or dried after initialization!!!.
 *
 * Again, The SyncFlag member MUST be initialized to 0. The SAME flag must be
 * used for initializing and deleting the critcal section controls. 
 *
 * AUTHOR
 * Norton 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
void KpInitializeCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	KpInt32_t permission;

	for (;;) {
		if (CriticalFlag->SyncFlag == 1) {
			return;
		}
		permission = KpInterlockedExchange(&CriticalFlag->SyncFlag, -1);
		if (permission == 0) {
			InitializeCriticalSection (&CriticalFlag->CriticalFlag);
			InterlockedExchange (&CriticalFlag->SyncFlag, 1);
			return;
		}
		else if (permission == 1) {
			/* Keep it that way */
			InterlockedExchange (&CriticalFlag->SyncFlag, 1);
			return;
		}

		KpSleep (0, KPFALSE);

	} /* for (;;) */
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 * This function provides CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until KpInitializeCriticalSection() is called.
 * Upon return from this function and until KpLeaveCriticalSection is called,
 * the resource being protected cannot be touched by other threads of the
 * current process. If function doesn't return KCM_SUCCESS, it was called
 * when the critical section was no longer in an intializied (viable) state.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * Norton 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpEnterCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
//	while (CriticalFlag->SyncFlag == -1) {}	/* wait while someone else inits this flag */

	if (CriticalFlag->SyncFlag != 1) {
		return KCMS_FAIL;
	}

	EnterCriticalSection (&CriticalFlag->CriticalFlag);
	return KCMS_SUCCESS;
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 * This function provides CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until KpInitializeCriticalSection() is called.
 * Once this function is called, the resource being protected can be touched
 * by other threads of the current process.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * Norton 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
void KpLeaveCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	LeaveCriticalSection (&CriticalFlag->CriticalFlag);
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Win32 Version)
 * This function provides CriticalSection Synchronization Control. This
 * function frees up CriticalSection Resources. It must be called once the
 * resource being synchronized is of no importance to the current process.
 * That is, the process is about to exit/die and no longer needs to
 * synchronize the resource.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * Norton 
 *
 * DATE CREATED
 * May 14, 1994
 *-------------------------------------------------------------------*/
void KpDeleteCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	KpInt32_t 		permission;

	for (;;) {
		permission = KpInterlockedExchange (&CriticalFlag->SyncFlag, -1);
		if (permission == 1) {
			DeleteCriticalSection (&CriticalFlag->CriticalFlag);
			KpInterlockedExchange (&CriticalFlag->SyncFlag, 0);
			return;
		}
		else if (permission == 0) {
			/* Keep it that way */
			KpInterlockedExchange (&CriticalFlag->SyncFlag, 0);
			return;
		}

		KpSleep (0, KPFALSE);
	}
}

/******************************************************/
/* Process/Thread Synchronization functions (Solaris) */
/******************************************************/

#elif defined (KPSOLARIS) || defined (KPLINUX)

/*--------------------------------------------------------------------
 * DESCRIPTION (Solaris Version)
 * This function initialize/enables the use of CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until this function is called. This function may not be
 * called again without first calling KpDeleteCriticalSection() within the
 * current process.
 *
 * Prior to the call, the CriticalFlag variable must have the SyncFlag member
 * initialized to zero. The whole variable declaration should be done in data
 * memory and not on the stack. The structure needs to be long word aligned
 * to work correctly for the PC.  The CritcalFlag variable must not be touched,
 * moved, washed or dried after initialization!!!.
 *
 * Again, The SyncFlag member MUST be initialized to 0. The SAME flag must be
 * used for initializing and deleting the critcal section controls. 
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 17, 1994
 *-------------------------------------------------------------------*/
void KpInitializeCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{

	KpInt32_t		permission;
	int				sysRetVal;

	for (;;) {		  			 					  

		if (CriticalFlag->SyncFlag == 1) {
			return;
		}

		permission = KpInterlockedExchange (&CriticalFlag->SyncFlag, -1);

			 					/* If this critical section has not been
								   initialized yet then initialize it		*/

		if (0 == permission) {
		#if defined (KP_POSIX_THREADS)
			sysRetVal = pthread_mutex_init (&CriticalFlag->CriticalFlag, NULL);
		#else
			sysRetVal = mutex_init (&CriticalFlag->CriticalFlag, NULL, NULL);
		#endif
			if (0 == sysRetVal)  {
				CriticalFlag->ThreadId = 0;
				CriticalFlag->Count = 0;
				KpInterlockedExchange (&CriticalFlag->SyncFlag,1);
				return;
			}
		}

								/* The critical section was already 
								   initialized so leave it alone			*/

		else if (1 == permission) {
			KpInterlockedExchange (&CriticalFlag->SyncFlag,1);
			return;
		}

#if defined (KP_POSIX_THREADS)
		sched_yield();
#else
		thr_yield ();
#endif
		
	} /* for (;;) */

} /* KpInitializeCriticalSection */

/*--------------------------------------------------------------------
 * DESCRIPTION (Solaris Version)
 * This function provides CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until KpInitializeCriticalSection() is called.
 * Upon return from this function and until KpLeaveCriticalSection is called,
 * the resource being protected cannot be touched by other threads of the
 * current process. If function doesn't return KCM_SUCCESS, it was called
 * when the critical section was no longer in an intializied (viable) state.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 20, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpEnterCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{

	KpInt32_t	myThreadId;

	if (CriticalFlag->SyncFlag != 1)
		return KCMS_FAIL;

								/* If the Critical Section is locked and
								   it was locked by this thread thread
								   then increment the lock count and then
								   return									*/

	myThreadId = KpGetCurrentThreadId ();
	if ((CriticalFlag->Count > 0) && 
		(myThreadId == CriticalFlag->ThreadId)) {

		CriticalFlag->Count++;

	}
	else {

	#if defined (KP_POSIX_THREADS)
		if (pthread_mutex_lock (&CriticalFlag->CriticalFlag) != 0) {
	#else
		if (mutex_lock (&CriticalFlag->CriticalFlag) != 0) {
	#endif
			return KCMS_FAIL;
		}
		CriticalFlag->Count++;
		CriticalFlag->ThreadId = myThreadId;
	}

	return KCMS_SUCCESS;
} /* KpEnterCriticalSection */

/*--------------------------------------------------------------------
 * DESCRIPTION (Solaris Version)
 * This function provides CriticalSection Synchronization
 * Control. KpEnterCriticalSection() and KpLeaveCriticalSection() functions
 * may not be called until KpInitializeCriticalSection() is called.
 * Once this function is called, the resource being protected can be touched
 * by other threads of the current process.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 20, 1994
 *-------------------------------------------------------------------*/
void KpLeaveCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	KpInt32_t	myThreadId;

	myThreadId = KpGetCurrentThreadId ();
		
								/* If this thread doesn't own the 
								   critical section something is wrong	*/
	if (myThreadId != CriticalFlag->ThreadId) {
		for (;;);
	}

								/* Decrement the lock count and if the
								   new lock count is zero then really 
								   unlock the critical section			*/
	CriticalFlag->Count--;
	if (0 == CriticalFlag->Count) {

		CriticalFlag->ThreadId = 0;
	#if defined (KP_POSIX_THREADS)
		if (pthread_mutex_unlock (&CriticalFlag->CriticalFlag) != 0) {
	#else
		if (mutex_unlock (&CriticalFlag->CriticalFlag) != 0) {
	#endif
			for (;;);	  /* something is wrong */
		}
	}

}

/*--------------------------------------------------------------------
 * DESCRIPTION (Solaris Version)
 * This function provides CriticalSection Synchronization Control. This
 * function frees up CriticalSection Resources. It must be called once the
 * resource being synchronized is of no importance to the current process.
 * That is, the process is about to exit/die and no longer needs to
 * synchronize the resource.
 *
 * The CritcalFlag variable must not be touched, moved, washed or dried!!!
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 20, 1994
 *-------------------------------------------------------------------*/
void KpDeleteCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	KpInt32_t		permission;
	int				sysRet;

	for (;;) {

		permission = KpInterlockedExchange (&CriticalFlag->SyncFlag, -1);

								/* verify that critical section was
								   initialized and destroy it			*/

		if (1 == permission) {
		#if defined (KP_POSIX_THREADS)
			if (pthread_mutex_destroy (&CriticalFlag->CriticalFlag) != 0) {
		#else
			if (mutex_destroy (&CriticalFlag->CriticalFlag) != 0) {
		#endif
				for (;;);   /* something is wrong */
			}
			CriticalFlag->SyncFlag = 0;
			CriticalFlag->ThreadId = 0;
			CriticalFlag->Count = 0;
			return;
		}
	}
} /* KpDeleteCriticalSection */

#else

/*************************************************/
/* Process/Thread Synchronization stub functions */
/*    for OSes with out premtive multitasking	 */
/*************************************************/

/*--------------------------------------------------------------------*/
void KpInitializeCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	if (CriticalFlag) {};
}

/*--------------------------------------------------------------------*/
KpInt32_t KpEnterCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	if (CriticalFlag) {};

	return KCMS_SUCCESS;
}

/*--------------------------------------------------------------------*/
void KpLeaveCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	if (CriticalFlag) {};
}

/*--------------------------------------------------------------------*/
void KpDeleteCriticalSection (KpCriticalFlag_t FAR *CriticalFlag)
{
	if (CriticalFlag) {};
}

#endif

/***************************************************************/
/* Atomic Read Modify Write functions (32 Bit Windows Version) */
/***************************************************************/

#if defined (KPWIN32) && !defined(KPMSMAC)
/*--------------------------------------------------------------------
 * DESCRIPTION (32 Bit Windows Version)
 * This function read the data from the location specified by address and
 * writes the data specified by value to the location specified by address
 * as an atomic operation.  The data read from address is returned.
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * June 27, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpInterlockedExchange (KpInt32_p address, KpInt32_t value)
{
	return (InterlockedExchange (address, value));
}

/***************************************************************/
/* Atomic Read Modify Write functions (Solaris Version) */
/***************************************************************/

#elif defined (KPSOLARIS) || defined (KPLINUX)

								/*	This mutex is used to ensure that
									the exchange in the Solaris
									KpInterlockedExchange function is
									not preempted by another thread in
									a given process.  The C Language
									guarantees that external structures
									are initialized to contain all zeros.
									Given the mutex is initialized to
									all zeros eliminates the need to 
									call mutex_init since one of the
									ways to initialize a mutex is to
									zero all its entries.  Initializing
									this mutex this way ensures that it
									is only initialized once per process.	*/
									
#if defined (KP_POSIX_THREADS)
static pthread_mutex_t		exchangeMutex;
#else
static mutex_t		exchangeMutex;
#endif

/*--------------------------------------------------------------------
 * DESCRIPTION (Solaris Version)
 * This function reads the data from the location specified by address and
 * writes the data specified by value to the location specified by address
 * as an atomic operation.  The data read from address is returned.
 *
 * AUTHOR
 * mjb
 *
 * DATE CREATED
 * November 3, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpInterlockedExchange (KpInt32_p address, KpInt32_t value)
{

KpInt32_t	data, retVal;

#if defined (KP_POSIX_THREADS)
	retVal = pthread_mutex_lock (&exchangeMutex);
#else
	retVal = mutex_lock (&exchangeMutex);
#endif
	if (0 != retVal) {
		for (;;);			/* Something is drastically wrong */
	}

	data = *address;
	*address = value;

#if defined (KP_POSIX_THREADS)
	retVal = pthread_mutex_unlock (&exchangeMutex);
#else
	retVal = mutex_unlock (&exchangeMutex);
#endif
	if (0 != retVal) {
		for (;;);			/* Something is drastically wrong */
	}

	return (data);

}

/*************************************************/
/* Default Atomic Read Modify Write functions	 */
/*    (not really atomic)						 */
/*************************************************/

#else 

KpInt32_t KpInterlockedExchange (KpInt32_p address, KpInt32_t value)
{

KpInt32_t		data;

	data = *address;
	*address = value;
	return (data);

} /* KpInterlockedExchange */

#endif

/*******************************************************/
/* Semaphore functions (Solaris Versions) */
/*******************************************************/

#if defined (KPSOLARIS) || defined (KPLINUX)

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetInit (KPSOLARIS Version)
 *
 * DESCRIPTION 
 *	This function returns the semaphore set which is associated with the
 *	name string pointed to by Name.  If the semaphore set has not been 
 *	previously initialized then the semaphore set is created with NumSems 
 *	semaphores in the set.  If SemInitArray is not null, it contains an
 *	array of KpSemInit structures which are used to initialize the individual
 *	semaphores of the array.  The number of elements in the array must equal
 *	the number of semaphores in the set.  The first element of the array is
 *	the initial value for the first semaphore, the second element of the array
 *	is the initial value for the second semaphore and so on.  If SemInitArray 
 *	is null then each of the individual semaphores is initialized to 1.  
 *
 *	If the semaphore set had been previously initialized that semaphore set
 *	is returned and SemInitArray are ignored.
 *
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	June 21, 1994
 *-------------------------------------------------------------------*/
KpSemSet_t 
KpSemSetInit (char 			*Name, 
			  KpUInt32_t 	NumSems, 
			  KpSemInit_t	*SemInitArray)
{

KpUInt32_t		kcmsRet, semNum;
key_t			key;
int				semId, semFlag, sysRet, locked;
semun_t			semun;
KpSemInitData_t	FAR *initArray;
KpSemSetData_p	semSetDataPtr;
KpSemSetData_h	semSetDataHdl;
KpSemSet_t		kcmsSysSemSet;

								/*  If not initializing the KCMS_SYS 
									semaphore then get the KCMS_SYS 
									semaphore 							*/

	locked = 0;
	semId = -1;
	sysRet = strcmp (Name, KCMS_SYS_SEM_SET);
	if (0 != sysRet) {
		kcmsSysSemSet = acquireKcmsSysLock ();
		if (NULL == kcmsSysSemSet) {
			return NULL;
		}
		locked = 1;
	}
								/* Increment NumSems by 1 to allow
								   for the reference count semaphore, then
								   create the semaphore 					*/ 

	NumSems++;
	kcmsRet = getKeyFromName (Name, &key);
	if (KCMS_SUCCESS != kcmsRet) {
		goto InitFailed;
	}

	semFlag = KP_IPC_PERM_RWALL;
	semId = semget (key, NumSems, semFlag | IPC_CREAT | IPC_EXCL);

								/* If semget fails, either the semaphore
								   was previously created or there was a
								   real error.  Try to get the semaphore
								   without creating or initializing it.
								   If that fails then there is a real
								   error									*/

	if (-1 == semId) {
		semId = semget (key, NumSems, semFlag);
		if (-1 == semId) {
			goto InitFailed;
		}
	}
	else {

		  			 			/* The semaphore was just created so
								   initialize it.  Start by creating
								   an array of initial values for each
								   semaphore in the set			 			*/

		initArray = 
			(KpSemInitData_t *) allocBufferPtr (sizeof (KpSemInitData_t) * 
												NumSems);
		if (NULL == initArray) {
			goto InitFailed;
		}

		kcmsRet = createInitArray (SemInitArray, NumSems, initArray);
		if (KCMS_SUCCESS != kcmsRet) {
			freeBufferPtr (initArray);
			goto InitFailed;
		}

		 					  	/* Do the initialization */

		semun.array = initArray;
		sysRet = semctl (semId, 0, SETALL, semun);
		freeBufferPtr (initArray);
		if (0 != sysRet) {
			goto InitFailed;
		}

	} /* if semId */
		
		 					  	/* Create the KpSemSetData Structure */

	semSetDataHdl = 
			(KpSemSetData_h) allocBufferHandle (sizeof (KpSemSetData_t));
	if (NULL == semSetDataHdl) {
		goto InitFailed;
	}

	semSetDataPtr = (KpSemSetData_p) lockBuffer (semSetDataHdl);
	if (NULL == semSetDataPtr) {
		freeBuffer (semSetDataHdl);
        goto InitFailed;
	}

	semSetDataPtr->NumSemaphores = NumSems;
	semSetDataPtr->SemId = semId;
	unlockBuffer (semSetDataHdl);

								/*  If the semaphore being initialized is
									not the Kp_kcms_sys semaphore then, 
									increment the usage count semaphore.  
									(Releasing a semaphore increments it  
									by one).  Then release the Kp_kcms_sys
									semaphore.								*/

	if (1 == locked) {
		semNum = KP_SEM_USAGE;
		kcmsRet = KpSemaphoreRelease ((KpSemSet_t) semSetDataHdl, 1, &semNum,
									  KP_SEM_DEF_INCR);
		if (KCMS_SUCCESS != kcmsRet) {
			freeBuffer (semSetDataHdl);
	        goto InitFailed;
		}	

		kcmsRet = releaseKcmsSysLock (&kcmsSysSemSet);
		if (KCMS_SUCCESS != kcmsRet) {
			freeBuffer (semSetDataHdl);
			goto InitFailed;
		}
		locked = 0;
	}

	return (KpSemSet_t) semSetDataHdl;


								/* This code is executed if the 
								   initialization failed for any reason	*/

InitFailed:
	if (1 == locked) {
		releaseKcmsSysLock (&kcmsSysSemSet);
	}
	if (-1 != semId) {
		semctl (semId, 0, IPC_RMID);	/*  remove the semaphore since it
											was not initialized
											properly						*/
	}
	return NULL;	
										 
} /* KpSemSetInit */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetFree (KPSOLARIS Version)
 *
 * DESCRIPTION 
 * 	This function frees the KpSemSet structure pointed to by SemSet.
 * 	Any memory that was allocated for the SemSet structure is freed.  
 * 	The semaphore system resource is destroyed if the usage count semaphore's 
 *	value is zero
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	June 24, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t	
KpSemSetFree (KpSemSet_t *SemSet)
{

KpUInt32_t		kcmsRet, retVal, semNum;
int				usageCnt, sysRet;
KpSemSetData_p	semSetDataPtr;
KpSemSet_t		kcmsSysSemSet;


								/* Get the KCMS_SYS semaphore */

	retVal = KCMS_SUCCESS;
	kcmsSysSemSet = acquireKcmsSysLock ();
	if (NULL == kcmsSysSemSet) {
		return KCMS_FAIL;
	}

								/*  Decrement the usage count semaphore.  
									Getting a semaphore decrements it by 
									one.									*/

	semNum = KP_SEM_USAGE;
	kcmsRet = KpSemaphoreGet (*SemSet, 1, &semNum, KP_SEM_NO_WAIT);
	if (KCMS_SUCCESS != kcmsRet) {
		retVal = kcmsRet;
		goto FailedAndLocked;
	}
		
								/*  Get the value of the usage count
								   	semaphore.  If it is zero then destroy
									the semaphore							*/

	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) *SemSet);
	if (NULL == semSetDataPtr) {
		retVal = KCMS_FAIL;
		goto FailedAndLocked;
	}

	usageCnt = semctl (semSetDataPtr->SemId, semNum, GETVAL);
	unlockBuffer ((KpSemSetData_h) *SemSet);
	if (-1 == usageCnt) {
		retVal = KCMS_FAIL;
		goto FailedAndLocked;
	}
	if (0 == usageCnt) {
		sysRet = semctl (semSetDataPtr->SemId, 0, IPC_RMID);
		if (-1 == sysRet) {
			retVal = KCMS_FAIL;
			goto FailedAndLocked;
		}
	}

										/*  Free the local memory 
											associatted with the semaphore
											set								*/

	freeBuffer ((KpSemSetData_h) *SemSet);
	*SemSet = NULL;

								/* release the KCMS_SYS semaphore and return */

FailedAndLocked:
	kcmsRet = releaseKcmsSysLock (&kcmsSysSemSet);
	if (KCMS_SUCCESS != kcmsRet) {
		return kcmsRet;
	}

	return retVal;

} /* KpSemSetFree */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetDestroy (KPSOLARIS Version)
 *
 * DESCRIPTION 
 *	This function is the same as KpSemSet Free except that the 
 *	semaphore system resource is always destroyed.
 *
 *	NOTE - use this function only if it is known that no other process
 *		   currently have a valid semaphore id which corresponds to 
 *		   the semaphore being destroyed.
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 17, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t
KpSemSetDestroy (char *name)
{

KpUInt32_t		semNum;
int				usageCnt, sysRet;
KpSemSetData_p	semSetDataPtr;
KpSemSet_t		semSet;
KpFileProps_t		fileProps;
char			filename[256];
char			*semDirPath;

								/*  Form the complete file name from the
									semDirPath and name.					*/

	semDirPath = getenv ("KPSEMDIRPATH");
	if (NULL == semDirPath) {
		strcpy (filename, "/tmp");
	}
	else {
		strcpy (filename, semDirPath);
	}
	strcat (filename, "/");
	strcat (filename, name);

								/*	If the init fails then the semaphore
									does not exist so return success		*/

	semSet = KpSemSetInit (name, 0, NULL);
	if (NULL == semSet) {
		KpFileDelete (filename, &fileProps);
		return KCMS_SUCCESS;
	}
		
	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) semSet);
	if (NULL == semSetDataPtr) {
		return KCMS_FAIL;
	}

	semNum = KP_SEM_USAGE;
	usageCnt = semctl (semSetDataPtr->SemId, semNum, GETVAL);
	if (-1 == usageCnt) {
		unlockBuffer ((KpSemSetData_h) semSet);
		return KCMS_FAIL;
	}

	sysRet = semctl (semSetDataPtr->SemId, 0, IPC_RMID);
    unlockBuffer ((KpSemSetData_h) semSet);
	if (-1 == sysRet) {
		return KCMS_FAIL;
	}

										/*  Free the local memory 
											associatted with the semaphore
											set								*/

	freeBuffer ((KpSemSetData_h) semSet);

										/* delete the semaphore key file	*/

	KpFileDelete (filename, &fileProps);
	return KCMS_SUCCESS;

} /* KpSemSetDestroy */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemaphoreGet (KPSOLARIS Version)
 *
 * DESCRIPTION 
 *	This function attempts to get one or more semaphores from the semaphore
 *	set specified by SemSet.  SemList is a pointer to an array which contains
 *	the semaphore numbers of the of the semaphores to get.  NumEntries
 *	specifies the number of enties in the array.
 *
 *	If all of the requested semaphores are available (greater than zero) they 
 *	are all decremented by 1 and the function returns KCMS_SUCCESS.  If one or
 *	more of the semaphores are not available (equal to zero) then the function
 *	returns KCMS_FAIL when TimeOut is set to KP_SEM_NO_WAIT.  Otherwise the 
 *	function blocks waiting for all of the semaphores to become available
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	June 24, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t	
KpSemaphoreGet (KpSemSet_t	SemSet, 
				KpUInt32_t	NumEntries, 
				KpUInt32_t	*SemList, 
				KpUInt32_t	TimeOut)
{
KpSemSetData_p		semSetDataPtr;
struct sembuf		*operations;
KpIndex_t			index;
short				semFlag;
int					sysRetVal;

	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) SemSet);
	if (NULL == semSetDataPtr)
		return KCMS_FAIL;

								/* create the array of sembuf structs 
								   (the semaphore operations)			*/

	operations = (struct sembuf *) allocBufferPtr (sizeof (struct sembuf) * 
												   NumEntries);
	if (NULL == operations) {
		unlockBuffer ((KpSemSetData_h) SemSet);
		return KCMS_FAIL;
	}

	if (KP_SEM_NO_WAIT == TimeOut) {
		semFlag = IPC_NOWAIT;
	}
	else {
		semFlag = 0;
	}

	for (index = 0; index < NumEntries; index++) {
		operations[index].sem_num = SemList[index];
		operations[index].sem_op = KP_GET_SEMAPHORE;
		operations[index].sem_flg = semFlag;
	}

								/* Now try to get the specified semaphores	*/

	sysRetVal = semop (semSetDataPtr->SemId, operations, NumEntries);
	freeBufferPtr (operations);
	unlockBuffer ((KpSemSetData_h) SemSet);
	if (0 != sysRetVal)
		return KCMS_FAIL;

	return KCMS_SUCCESS;

} /* KpSemaphoreGet */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemaphoreRelease (KPSOLARIS Version)
 *
 * DESCRIPTION 
 * 	This function releases one or more semaphores, in the semaphore set 
 * 	specified by SemSet, by incrementing the value of the semaphore by 
 *	the amount specified by Increment.  SemList is a pointer to an array 
 *	which contains the semaphore numbers of the semaphores to release.  
 *	NumEntries specifies the number of enties in the array.  
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	June 24, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t 
KpSemaphoreRelease (KpSemSet_t	SemSet, 
					KpUInt32_t	NumEntries, 
					KpUInt32_t	*SemList, 
					KpUInt32_t	Increment)
{
KpSemSetData_p		semSetDataPtr;
struct sembuf		*operations;
KpIndex_t			index;
int					sysRetVal;

	semSetDataPtr = lockBuffer ((KpSemSetData_h) SemSet);
	if (NULL == semSetDataPtr)
		return KCMS_FAIL;

								/* create the array of sembuf structs 
								   (the semaphore operations)			*/

	operations = (struct sembuf *) allocBufferPtr (sizeof (struct sembuf) * 
												   NumEntries);
	if (NULL == operations) {
		unlockBuffer ((KpSemSetData_h) SemSet);
		return KCMS_FAIL;
	}

	for (index = 0; index < NumEntries; index++) {
		operations[index].sem_num = SemList[index];
		operations[index].sem_op = Increment;
		operations[index].sem_flg = 0;
	}

								/* Now release the specified semaphores	*/

	sysRetVal = semop (semSetDataPtr->SemId, operations, NumEntries);
	freeBufferPtr (operations);
	unlockBuffer ((KpSemSetData_h) SemSet);
	if (0 != sysRetVal)
		return KCMS_FAIL;
	
	return KCMS_SUCCESS;

} /* KpSemaphoreRelease */


/*--------------------------------------------------------------------
 * FUNCTION
 *	getKeyFromName (KPSOLARIS Version)
 *
 * DESCRIPTION 
 *	This function converts the name string pointed to by name into a 
 *	key to be used for creating semaphores.  The key is returned in 
 *	the location pointed to by key.
 *
 * RETURNS
 *		KCMS_SUCCESS 	if successful
 *		KCMS_FAIL		otherwise
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 18, 1994
 *-------------------------------------------------------------------*/ 
KpUInt32_t 
getKeyFromName (char	*name, 
				key_t	*key)
{

char 			*semDirPath;
char			filename [256];
KpFileProps_t		dirProps;
KpFileId		fid;
KpInt32_t		length, sysRet;
KpUInt32_t		kcmsRet;


								/*  Form the complete file name from the
									semDirPath and name.					*/

	semDirPath = getenv ("KPSEMDIRPATH");
	if (NULL == semDirPath) {
		strcpy (filename, "/tmp");
	}
	else {
		strcpy (filename, semDirPath);
	}
	strcat (filename, "/");
	strcat (filename, name);

								/* Make sure the file exists */

	kcmsRet = KpFileOpen (filename, "r", &dirProps, &fid);
	if (KCMS_IO_SUCCESS != kcmsRet) {
		kcmsRet = KpFileDelete (filename, &dirProps);
		kcmsRet = KpFileOpen (filename, "e", &dirProps, &fid);
		if (KCMS_IO_SUCCESS != kcmsRet) {
			return KCMS_FAIL;
		}

		length = strlen (name);
		kcmsRet = KpFileWrite (fid, name, length);
		if (KCMS_IO_SUCCESS != kcmsRet) {
			KpFileClose (fid);
			return KCMS_FAIL;
		}	

		sysRet = fchmod (fid, 0666);
		if (0 != sysRet) {
			KpFileClose (fid);
			return KCMS_FAIL;
		}

	}

	kcmsRet = KpFileClose (fid);
	if (KCMS_IO_SUCCESS != kcmsRet) {
		return KCMS_FAIL;
	}	

								/* Now get the key */

	*key = ftok (filename, atoi ("k"));
	if (-1 == *key) {
		return KCMS_FAIL;
	}

	return KCMS_SUCCESS;
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	acquireKcmsSysLock
 *
 * DESCRIPTION (KPSOLARIS Version)
 *	This function initializes and then locks the KCMS_SYS semaphore.
 *
 * RETURNS
 *	KCMS_SUCCESS		if successful
 *	KCMS_FAIL			otherwise
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 16, 1994
 *-------------------------------------------------------------------*/
KpSemSet_t
acquireKcmsSysLock (void)
{

KpSemSet_t		semSet;
KpUInt32_t		kcmsRet, semNum;

	semSet = KpSemSetInit (KCMS_SYS_SEM_SET, KCMS_SYS_NUM_SEMS, NULL);
	if (NULL == semSet) {
		return NULL;
	}

	semNum = KCMS_SYS_SEMAPHORE;
	kcmsRet = KpSemaphoreGet (semSet, 1, &semNum, KP_SEM_INFINITE);
	if (KCMS_SUCCESS != kcmsRet) {
		return NULL;
	}

	return semSet;

} /* acquireKcmsSysLock */


/*--------------------------------------------------------------------
 * FUNCTION
 *	releaseKcmsSysLock
 *
 * DESCRIPTION (KPSOLARIS Version)
 *	This function unlocks the KCMS_SYS semaphore and frees the local
 *	memory associated with it.
 *
 * RETURNS
 *	KCMS_SUCCESS		if successful
 *	KCMS_FAIL			otherwise
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 16, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t
releaseKcmsSysLock (KpSemSet_t *SemSet)
{

KpUInt32_t		kcmsRet, semNum;
int				sysRet;

	semNum = KCMS_SYS_SEMAPHORE;
	kcmsRet = KpSemaphoreRelease (*SemSet, 1, &semNum, KP_SEM_DEF_INCR);
	if (KCMS_SUCCESS != kcmsRet) {
		return kcmsRet;
	}

											/*  Free the local memory 
											associatted with the semaphore
											set								*/

	freeBuffer ((KpSemSetData_h) *SemSet);
	*SemSet = NULL;
	return KCMS_SUCCESS;

} /* releaseKcmsSysLock */

/*******************************************************/
/* Semaphore functions (Windows 32 Bit Version) 	   */
/*******************************************************/

#elif defined (KPWIN32)	&& !defined(KPMSMAC)

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetInit (KPWIN32 Version)
 *
 * DESCRIPTION 
 *	This function returns the semaphore set which is associated with the
 *	name string pointed to by Name.  If the semaphore set has not been 
 *	previously initialized then the semaphore set is created with NumSems 
 *	semaphores in the set.  If SemInitArray is not null, it contains an
 *	array of KpSemInit structures which are used to initialize the individual
 *	semaphores of the array.  The number of elements in the array must equal
 *	the number of semaphores in the set.  The first element of the array is
 *	the initial value for the first semaphore, the second element of the array
 *	is the initial value for the second semaphore and so on.  If SemInitArray 
 *	is null then each of the individual semaphores is initialized to 1.  
 *
 *	If the semaphore set had been previously initialized that semaphore set
 *	is returned and SemInitArray are ignored.
 *
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 18, 1994
 *-------------------------------------------------------------------*/
KpSemSet_t 
FAR 
KpSemSetInit (char 			FAR *Name, 
			  KpUInt32_t 	NumSems, 
			  KpSemInit_t 	FAR *SemInitArray)
{

KpSemId_t			FAR * FAR * semIdHdl = NULL;
KpSemId_t			FAR * semIdArray;
KpSemInitData_t		FAR *initArray = NULL;
KpUInt32_t			index, kcmsRet, semNum;
char				semName[256];
KpSemSetData_h		semSetDataHdl = NULL;
KpSemSetData_p		semSetDataPtr;

								/*	Check if semaphores are supported.  If
									not then return something other than 
									NULL to indicate success				*/

	if (!KpUseSemaphores()) {
		return (KpSemSet_t) 1;
	}

								/*	Increment NumSems by 1 to allow
									for the reference count semaphore		*/ 

	NumSems++;

								/*	Allocate memory for semIdArray	and 
									initialize to NULL						*/

	semIdHdl = 
		(KpSemId_t FAR * FAR *) allocBufferHandle (sizeof (KpSemId_t) * 
												   NumSems);
	if (NULL == semIdHdl) {
		goto initFailed;
	}

	semIdArray = (KpSemId_t FAR *) lockBuffer (semIdHdl);
	if (NULL == semIdArray) {
		goto initFailed;
	}

	for (index = 0; index < NumSems; index++) {
		semIdArray[index] = NULL;
	}

								/*	Allocate memory for initArray and then
									initialize the array.					*/

	initArray = (KpSemInitData_t *) allocBufferPtr (sizeof (KpSemInitData_t) * 
													NumSems);
	if (NULL == initArray) {
		goto initFailed;
	}

	kcmsRet = createInitArray (SemInitArray, NumSems, initArray);
	if (KCMS_SUCCESS != kcmsRet) {
		goto initFailed;
	}

								/*	Create each of the individual 
									semaphores, saving the handles in
									semIdArray.								*/

	for (index = 0; index < NumSems; index++) {

		kcmsRet = getSemName (Name, index, sizeof (semName), semName);
		if (KCMS_SUCCESS != kcmsRet) {
			goto initFailed;
		}
		
		semIdArray[index] = CreateSemaphore (NULL, initArray[index], 
											 KP_SEM_MAX_VAL, semName); 
		if (NULL == semIdArray[index]) {
			goto initFailed;
		}
	}

								/*	Unlock the handle to the semId array
									and free the memory allocated to the
									initArray								*/

	unlockBuffer (semIdHdl);
	freeBufferPtr (initArray);
	initArray = NULL;

								/* Create the KpSemSetData structure		*/

	semSetDataHdl = 
			(KpSemSetData_h) allocBufferHandle (sizeof (KpSemSetData_t));
	if (NULL == semSetDataHdl)
		goto initFailed;

	semSetDataPtr = (KpSemSetData_p) lockBuffer (semSetDataHdl);
	if (NULL == semSetDataPtr)
		goto initFailed;

	semSetDataPtr->NumSemaphores = NumSems;
	semSetDataPtr->semId = semIdHdl;

	unlockBuffer (semSetDataHdl);

								/*	Everything was created and initialized
									properly so increment the usage count
									semaphore.  KpSemaphoreRelease is used 
									to increment the semaphore				*/

	semNum = KP_SEM_USAGE;
	kcmsRet = KpSemaphoreRelease ((KpSemSet_t) semSetDataHdl, 1, &semNum, KP_SEM_DEF_INCR);
	if (KCMS_SUCCESS != kcmsRet) {
		goto initFailed;
	}	

	return (KpSemSet_t) semSetDataHdl;


								/* This code is executed if the 
								   initialization failed for any reason.
								   It closes the handles to all of the 
								   semaphores that were created and 
								   frees the memory allocated for 
								   semIdArray, initArray and the 
								   semSetData structure						*/

initFailed:

	if (NULL != semIdHdl) {


		semIdArray = (KpSemId_t FAR *) lockBuffer (semIdHdl);
		if (NULL != semIdArray) {
			for (index = 0; index < NumSems; index++) {
		
				if (NULL != semIdArray[index]) {
					CloseHandle (semIdArray[index]);
				}
			}
		}
		freeBuffer (semIdHdl);
	}

	if (NULL != initArray) {
		freeBufferPtr (initArray);
	}

	if (NULL != semSetDataHdl) {
		freeBuffer (semSetDataHdl);
	}

	return NULL;
										 
} /* KpSemSetInit */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetFree (KPWIN32 Version)
 *
 * DESCRIPTION 
 * 	This function frees the KpSemSet structure pointed to by SemSet.
 * 	The handles for all of the semaphores in the set are closed and any 
 *	memory that was allocated for the SemSet structure is freed.
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 19, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemSetFree (KpSemSet_t FAR *SemSet)
{

KpIndex_t		error = 0;
KpUInt32_t		index, numSems;
KpSemSetData_p	semSetDataPtr;
KpSemId_p		semIdPtr;

								/*	Check if semaphores are supported.  If
									not then return success					*/

	if (!KpUseSemaphores()) {
		return KCMS_SUCCESS;
	}

								/* 	Close the handles to all of the 
									semaphores.								*/

	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) *SemSet);
	if (NULL == semSetDataPtr) {
		return KCMS_FAIL;
	}

	semIdPtr = (KpSemId_p) lockBuffer ((KpSemId_h) semSetDataPtr->semId);
	if (NULL == semIdPtr) {
		return KCMS_FAIL;
	}

	numSems = semSetDataPtr->NumSemaphores;

	for (index = 0; index < numSems; index++) {
		if (NULL != semIdPtr[index]) {
			if (!CloseHandle (semIdPtr[index])) {
				error = 1;
			}
		}
	}

	unlockBuffer ((KpSemId_h) semSetDataPtr->semId);
	unlockBuffer ((KpSemSetData_h) *SemSet);

										/*  Free the local memory 
											associatted with the semaphore
											set								*/

	freeBuffer ((KpSemId_h) semSetDataPtr->semId);
	freeBuffer ((KpSemSetData_h) *SemSet);
	*SemSet = NULL;

	if (1 == error) {
		return KCMS_FAIL;
	}
	return KCMS_SUCCESS;

} /* KpSemSetFree */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemSetDestroy (KPWIN32 Version)
 *
 * DESCRIPTION 
 *	This function always returns success for Windows 32 Bit version
 *	because the system will destroy the semaphore object when all of 
 *	the handles to it are closed.  Unlike solaris where the semaphore 
 *	object must be explicitly destroyed.
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 17, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t
FAR
KpSemSetDestroy (char FAR *name)
{

	name = name;
	return KCMS_SUCCESS;

} /* KpSemSetDestroy */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemaphoreGet (KPWIN32 Version)
 *
 * DESCRIPTION 
 *	This function attempts to get one or more semaphores from the semaphore
 *	set specified by SemSet.  SemList is a pointer to an array which contains
 *	the semaphore numbers of the semaphores to get.  NumEntries specifies the 
 *	number of enties in the array.
 *
 *	If all of the requested semaphores are available (greater than zero) they 
 *	are all decremented by 1 and the function returns KCMS_SUCCESS.  If one or
 *	more of the semaphores are not available (equal to zero) then the function
 *	returns KCMS_FAIL when TimeOut is set to KP_SEM_NO_WAIT.  Otherwise the 
 *	function blocks waiting for all of the semaphores to become available
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 19, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemaphoreGet (KpSemSet_t	SemSet, 
				KpUInt32_t	NumEntries,
				KpUInt32_t	FAR *SemList,
				KpUInt32_t	TimeOut)
{

KpSemSetData_p		semSetDataPtr;
KpSemId_p			semIdPtr, semIdList = NULL;
KpUInt32_t			index;
DWORD				waitRet;


								/*	Check if semaphores are supported.  If
									not then return success					*/

	if (!KpUseSemaphores()) {
		return KCMS_SUCCESS;
	}

	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) SemSet);
	if (NULL == semSetDataPtr)
		return KCMS_FAIL;

	semIdPtr = (KpSemId_p) lockBuffer ((KpSemId_h) semSetDataPtr->semId);
	if (NULL == semSetDataPtr)
		goto GetOut;

								/* create the array of semaphore ids 		*/

	semIdList = (KpSemId_p) allocBufferPtr (sizeof (KpSemId_t) * NumEntries);
	if (NULL == semIdList) {
		goto GetOut;
	}

	for (index = 0; index < NumEntries; index++) {
		if (SemList[index] >= semSetDataPtr->NumSemaphores) {
			goto GetOut;
		}
		semIdList[index] = semIdPtr[SemList[index]];
	}

								/* determine whether to wait				*/

	if (KP_SEM_NO_WAIT == TimeOut) {
		TimeOut = 0;
	}
	
	if (KP_SEM_INFINITE == TimeOut) {
		TimeOut = INFINITE;
	}

								/* Now try to get the specified semaphores	*/

	waitRet = WaitForMultipleObjects (NumEntries, semIdList, TRUE, TimeOut);
	if (WAIT_FAILED == waitRet) {
		goto GetOut;
	}

	freeBufferPtr (semIdList);
	unlockBuffer ((KpSemId_h) semSetDataPtr->semId);
	unlockBuffer ((KpSemSetData_h) SemSet);

	return KCMS_SUCCESS;

								/*	This code is entered if an error 
									occured for any reason					*/

GetOut:
	if (NULL != semIdList) {
		freeBufferPtr (semIdList);
	}
	unlockBuffer ((KpSemId_h) semSetDataPtr->semId);
	unlockBuffer ((KpSemSetData_h) SemSet);
	return KCMS_FAIL;

} /* KpSemaphoreGet */

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpSemaphoreRelease (KPWIN32 Version)
 *
 * DESCRIPTION 
 * 	This function releases one or more semaphores, in the semaphore set 
 * 	specified by SemSet, by incrementing the value of the semaphore by 
 *	the amount specified by Increment.  SemList is a pointer to an array 
 *	which contains the semaphore numbers of the semaphores to release.  
 *	NumEntries specifies the number of enties in the array.  
 *
 * AUTHOR
 * 	mjb
 *
 * DATE CREATED
 * 	August 19, 1994
 *-------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemaphoreRelease (KpSemSet_t	SemSet,
					KpUInt32_t	NumEntries,
					KpUInt32_t	FAR *SemList,
					KpUInt32_t	Increment)
{

KpSemSetData_p		semSetDataPtr;
KpSemId_p			semIdArray;
KpIndex_t			failed = 0;
KpUInt32_t			index, numSems;
KpInt32_t			prevVal;
BOOL				retVal;

								/*	Check if semaphores are supported.  If
									not then return success					*/

	if (!KpUseSemaphores()) {
		return KCMS_SUCCESS;
	}

	semSetDataPtr = (KpSemSetData_p) lockBuffer ((KpSemSetData_h) SemSet);
	if (NULL == semSetDataPtr) {
		return KCMS_FAIL;
	}

	semIdArray = (KpSemId_p) lockBuffer ((KpSemId_h) semSetDataPtr->semId);
	if (NULL == semSetDataPtr) {
		failed = 1;
		goto GetOut;
	}

								/* Now release the specified semaphores	*/

	numSems = semSetDataPtr->NumSemaphores;
	for (index = 0; index < NumEntries; index++) {
		if (SemList[index] > numSems) {
			failed = 1;
		}
		retVal = ReleaseSemaphore (semIdArray[SemList[index]], 
								   Increment, &prevVal);
		if (TRUE != retVal) {
			failed = 1;
		}
	}

GetOut:
	unlockBuffer ((KpSemId_h) semSetDataPtr->semId);
	unlockBuffer ((KpSemSetData_h) SemSet);

	if (1 == failed) {
		return KCMS_FAIL;
	}

	return KCMS_SUCCESS;

} /* KpSemaphoreRelease */


/*--------------------------------------------------------------------
 * FUNCTION
 *	getSemName (KPWIN32 Version)
 *
 * DESCRIPTION 
 *	This function forms the semaphore name from the base name specified
 *	by baseName and the semaphore number specified by semNum.  The resultant
 *	name is returned in the buffer pointed to by semName.  The size of the
 *	result buffer is specified by bufSize.  This function returns 
 *	KCMS_SUCCESS if the name was formed successfully and KCMS_FAIL otherwise.
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	August 19, 1994
 *-------------------------------------------------------------------*/
static
KpUInt32_t
FAR
getSemName (const KpChar_p 	baseName, 
			KpUInt32_t 		semNum, 
			KpUInt32_t 		bufSize,
		    KpChar_p 		semName)

{
			 
KpUInt32_t		size;
char			numStr[10];

									/* 	Convert semNum to a character string	*/

	KpItoa (semNum, numStr);

									/* 	Make sure the combined string will 
										fit in the result buffer			*/

	size = strlen (baseName) + strlen (numStr) + 1;
	if (size > bufSize) {
		return KCMS_FAIL;
	}

									/* 	Form the semaphore name */

	strcpy (semName, baseName);
	strcat (semName, numStr);
	return KCMS_SUCCESS;

} /* get SemName */

	
/*--------------------------------------------------------------------
 * FUNCTION
 *	KpUseSemaphores (KPWIN32 Version)
 *
 * DESCRIPTION 
 *	This function examines what version of Windows is running and
 *	returns TRUE if that version of Windows supports semaphores.  
 *	Otherwise it returns FALSE
 *
 * AUTHOR
 *	mjb
 *
 * DATE CREATED
 *	September 16, 1994
 *-------------------------------------------------------------------*/
static BOOL KpUseSemaphores (void)
{
static BOOL	Initialize = TRUE;
static BOOL	UseSemaphores = FALSE;
	DWORD	winVersion;
	WORD	winVersionNum;
	int		majorVersion;
	WORD	winOS;

	if (Initialize) {
		Initialize = FALSE;
		winVersion = GetVersion ();
		winVersionNum = LOWORD (winVersion);
		majorVersion = LOBYTE (winVersionNum);
		winOS = HIWORD (winVersion);
		if (majorVersion == 4) {
			/* WIN_TYPE_CHICAGO */
			UseSemaphores = TRUE;
		}
		else if (winOS & 0x8000) {
			/* WIN_TYPE_WIN32s */
			UseSemaphores = FALSE;
		}
		else {
			/* WIN_TYPE_NT */
			UseSemaphores = TRUE;
		}
	}

	return UseSemaphores;
}
/*************************************************/
/* Semaphore stub functions 	  			 	 */
/*    for OSes with out premtive multitasking	 */
/*************************************************/

#else

static struct KpSemSet_tag	KpSemSet;

/*----------------------------------------------------------------------*/
KpSemSet_t 
FAR 
KpSemSetInit (char			FAR *Name,
			  KpUInt32_t	NumSems,
			  KpSemInit_t	FAR *SemInitArray)
{
	if (Name) {};
	if (NumSems) {};
	if (SemInitArray) {};

	return &KpSemSet;
}

/*----------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemSetFree (KpSemSet_t FAR *SemSet)
{
	if (SemSet) {};

	return KCMS_SUCCESS;
}

/*----------------------------------------------------------------------*/
KpUInt32_t
FAR
KpSemSetDestroy (char FAR *name)
{

	if (name) {};
	
	return KCMS_SUCCESS;

}

/*----------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemaphoreGet (KpSemSet_t	SemSet,
				KpUInt32_t	NumEntries,
				KpUInt32_t	FAR *SemList,
				KpUInt32_t	TimeOut)
{
	if (SemSet) {};
	if (NumEntries) {};
	if (SemList) {};
	if (TimeOut) {};

	return KCMS_SUCCESS;
}

/*----------------------------------------------------------------------*/
KpUInt32_t	
FAR 
KpSemaphoreRelease (KpSemSet_t	SemSet,
					KpUInt32_t	NumEntries,
					KpUInt32_t	FAR *SemList,
					KpUInt32_t	Increment)
{
	if (SemSet) {};
	if (NumEntries) {};
	if (SemList) {};
	if (Increment) {};

	return KCMS_SUCCESS;
}

#endif /* Semaphores */
