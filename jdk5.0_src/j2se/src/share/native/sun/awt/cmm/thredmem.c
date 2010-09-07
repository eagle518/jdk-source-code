/*
 * @(#)thredmem.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/****************************************************************
 * file: thredmem.c	@(#)thredmem.c	1.10	12/22/97
 *
 * Memory Allocation Management For Threads
 *
 * Conceived by Larry, started by Norton and reworked by Larry.
 *
 * The four exported functions are:
 *
 * KpThreadMemCreate() 	-	Create memory block for thread
 * KpThreadMemDestroy()	-	Remove memory block for thread
 * KpThreadMemFind()	-	Retrieve the memory block for the requesting thread
 * KpThreadMemUnlock()	-	Unlock the thread memory block
 *
 * Only one memory block can be allocated for a thread and one globally
 * accessible block accessible by all threads for a given process.
 *
 */ 

/****************************************************************
 ****************************************************************
 *   COPYRIGHT (c) 1994-1995 Eastman Kodak Company.
 *   As an  unpublished work  pursuant to  Title 17 of the  United
 *   States Code.  All rights reserved.
 ****************************************************************
 ****************************************************************/

#include "kcms_sys.h"

/* Thread Specific Memory Storage */
typedef struct {
	KpInt32_t				ProcessId;
	KpInt32_t				ThreadId;
	KpInt32_t				LockCount;
	KpThreadMemHdl_t FAR *	RootId;
	KpHandle_t				DataHandle;
} KpThreadMem_t;

typedef struct {
	KpUInt32_t			TotalMemberCount;
	KpUInt32_t			CurrentMemberCount;
	KpHandle_t			ThreadBaseHandle;
	KpThreadMem_t FAR *	TableList;
} KpRootThread_t;


#define KPTMEM_ALLOCNUMBER 64
#define KPTMEM_SLOTSIZE	(KpUInt32_t)sizeof(KpThreadMem_t)

void KpDelThreadMemCS ();
static KpRootThread_t	RootList = {0, 0, 0, NULL};
static KpRootThread_t	FAR *RootListPtr = NULL;
static KpCriticalFlag_t theCriticalThing = {0};


/*--------------------------------------------------------------------
 * Locates a particular slot containing the memory block handle for the
 * given thread. If the thread slot doesn't exist, return NULL.
 * See lockSlotBase() for more details to slot scheme.
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 14, 1994
 *-------------------------------------------------------------------*/
static KpThreadMem_t FAR *findThreadRoot (
			KpRootThread_t FAR *	pRoot,
			KpThreadMemHdl_t FAR *	RootId,
			KpInt32_t				ThreadFlag)
{
	KpThreadMem_t	FAR *pThread;
	KpInt32_t		processId;
	KpInt32_t		threadId;
	KpUInt32_t		count;

	if (pRoot == NULL)
		return NULL;

	processId = KpGetCurrentProcessId ();
	if (ThreadFlag == KPTHREADMEM)
		threadId  = KpGetCurrentThreadId ();
	else
		threadId = 0;	/* special case, global process memory for any thread */ 
	for (count = 0, pThread = pRoot->TableList;
			count < pRoot->CurrentMemberCount;
					count++, pThread++) {
	 	if ((pThread->ProcessId == processId) &&
			(pThread->ThreadId  == threadId)  &&
			(pThread->RootId    == RootId))
		
				return pThread;
			
	}
	return NULL;			
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Since the TableList structure is a finite array of elements pointing to
 *	memory storage locations, it can be exhausted. This function will grow
 *	the table. It returns the next available slot address for use.
 *
 * AUTHOR
 *	Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *	May 14, 1994
 *-------------------------------------------------------------------*/
static KpThreadMem_t FAR *growSlotTable (
			KpRootThread_t	FAR *pRoot)
{
	KpUInt32_t		tableSize, slotEntries, currentCount;
	KpThreadMem_t	FAR *pThreadBase;
	KpThreadMem_t	FAR *pThreadSrc;
	KpThreadMem_t	FAR *pThreadDest;

	if (pRoot == NULL)
		return NULL;

/* allocate a new table */
	slotEntries = pRoot->TotalMemberCount + KPTMEM_ALLOCNUMBER;
	tableSize = KPTMEM_SLOTSIZE * slotEntries;
	currentCount = pRoot->CurrentMemberCount;
	pThreadBase = allocSysBufferPtr (tableSize);
	if (NULL == pThreadBase)
		return NULL;

/* copy entries from old table to new table */
	for (pThreadSrc = pRoot->TableList, pThreadDest = pThreadBase;
			 currentCount;
					currentCount--)
		*pThreadDest++ = *pThreadSrc++;	

/* free old table and update global pointers */
	freeSysBufferPtr (pRoot->TableList);
	pRoot->TableList = pThreadBase;
	pRoot->ThreadBaseHandle = getSysHandleFromPtr (pThreadBase);
	pRoot->TotalMemberCount = slotEntries;

/* return pointer to available slot */
	return &pRoot->TableList [pRoot->CurrentMemberCount]; 
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * AUTHOR
 *	Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *	May 14, 1994
 *-------------------------------------------------------------------*/
static KpThreadMem_t FAR *addNewSlotItem(
				KpRootThread_t FAR *	pRoot,
				KpThreadMemHdl_t FAR *	RootId,
				KpInt32_t				ThreadFlag,
				KpGenericPtr_t			memory)
{
	KpThreadMem_t	FAR *pThread;
	KpInt32_t		processId;
	KpInt32_t		threadId;

	if (pRoot == NULL)
		return NULL;

	processId = KpGetCurrentProcessId ();
	if (ThreadFlag == KPTHREADMEM)
		threadId = KpGetCurrentThreadId ();
	else
		threadId = 0;	/* global process memory for any thread */ 

/* check for need to grow the slot table */
	if (pRoot->CurrentMemberCount == pRoot->TotalMemberCount) {
		pThread = growSlotTable (pRoot);
		if (NULL == pThread)
			return NULL;
	}
	else
		pThread = &pRoot->TableList [pRoot->CurrentMemberCount];

/* initialize the slot */
	pThread->DataHandle = (KpHandle_t) getHandleFromPtr (memory);
	pThread->RootId = RootId;
	pThread->ThreadId = threadId;
	pThread->ProcessId = processId;
	pThread->LockCount = 1;
	pRoot->CurrentMemberCount++;
	
	return pThread;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * Delete a slot entry. In the process of deleting a slot item, the
 * TableList is repacked so no empty slots exist.
 *
 * AUTHOR
 * LSH 
 *
 * DATE CREATED
 * May 25, 1994
 *
 *-------------------------------------------------------------------*/
static void deleteSlotItemOne (
			KpRootThread_t	FAR *pRoot,
			KpThreadMem_t	FAR *pSlot)
{
	if (pSlot->DataHandle != NULL) {
		unlockBuffer (pSlot->DataHandle);
		freeBuffer (pSlot->DataHandle);
		pSlot->DataHandle = NULL;			/* precautionary only!! */
	}

/* update the number of active entries */
	pRoot->CurrentMemberCount--;

/* done for not deleting entry at end of active list */
	if (pSlot != &pRoot->TableList [pRoot->CurrentMemberCount]) {

	/* move last active entry to the current slot */
		*pSlot = pRoot->TableList [pRoot->CurrentMemberCount];	
	}
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * Delete a slot entry. In the process of deleting a slot item, the
 * TableList is repacked so no empty slots exist. If threadFlag == KPPROCMEM
 * all entries for the ROOT process is deleted.
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 14, 1994
 *-------------------------------------------------------------------*/
static KpInt32_t deleteSlotItem (
			KpRootThread_t FAR *	pRoot,
			KpThreadMem_t FAR *		pSlot,
			KpInt32_t				threadFlag)
{
	KpInt32_t				ProcessId;
	KpThreadMemHdl_t FAR *	RootId;
	KpUInt32_t				Count;

	if ((pRoot == NULL) || (pSlot == NULL) || (pRoot->CurrentMemberCount == 0))
		return KCMS_FAIL;

/* save Root Id for latter */
	RootId = pSlot->RootId;
	ProcessId = pSlot->ProcessId;

/* delete the slot */
	deleteSlotItemOne (pRoot, pSlot);

/* done if this was a thread block delete */
	if (threadFlag != KPTHREADMEM) {

	/* need to find all thread blocks for the current process */
		Count = 0;
		pSlot = pRoot->TableList;
		while (Count < pRoot->CurrentMemberCount) {
		 	if ((pSlot->ProcessId == ProcessId) && (pSlot->RootId == RootId)) {
				deleteSlotItemOne (pRoot, pSlot);
			}
			else {
				Count++;
				pSlot++;
			}
		}
	}
	
	if (pRoot->CurrentMemberCount == 0) {
		freeSysBufferPtr (pRoot->TableList);
		pRoot = NULL;
		RootListPtr = NULL;
	}	

	return KCMS_SUCCESS; 	
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * Because MAC needs to lock/unlock memory or it gets into trouble,	a
 * economical mechanism has been devised to lock and unlock one memory
 * array containing information of all the thread memory blocks created.
 * Each entry slot contains a structure element identifying what thread
 * owns memory. A base structure identifies the array size and how many
 * elements it contains. This first structure to the list of allocated
 * memory blocks needs to be lock or unlock as well (when needed or not).
 * A pointer to the actual list Table is established within the
 * base structure. 
 *
 * If the calling handle is NULL, the initial structure is
 * created and initialized appropriately.
 *
 * Returns NULL if it can't accomplishthe opening.
 * Otherwise, returns a locked pointer to KpRootThread_t structure.
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 14, 1994
 *-------------------------------------------------------------------*/
static KpRootThread_t FAR *lockSlotBase (void)
{
	KpUInt32_t		tableSize;

/* check for need to initialize the memory list control block */
	if (RootListPtr == NULL) {

	/* initialize the memory list control block */
		RootListPtr = &RootList;
		RootListPtr->TotalMemberCount =	KPTMEM_ALLOCNUMBER;
		RootListPtr->CurrentMemberCount = 0;

		tableSize = KPTMEM_SLOTSIZE * RootListPtr->TotalMemberCount;
		RootListPtr->ThreadBaseHandle = 
					(KpHandle_t) allocSysBufferHandle (tableSize);
		if (NULL == RootListPtr->ThreadBaseHandle) {
			RootListPtr = NULL;
			return NULL;
		}
	}

/* lock the actual table */
	RootListPtr->TableList = lockSysBuffer (RootListPtr->ThreadBaseHandle);
	if (NULL == RootListPtr->TableList)
		RootListPtr = NULL;

	return RootListPtr;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * Because MAC needs to lock/unlock memory or it gets into trouble, the
 * first structure to the list of allocated memory blocks needs to unlocked
 * when no longer needed. The TableList pointer within the structure is also
 * unlocked. Returns handle to first (base) structure.
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 14, 1994
 *
 *-------------------------------------------------------------------*/
static void unlockSlotBase (void)
{
	if (RootListPtr != NULL)
		unlockSysBuffer (RootListPtr->ThreadBaseHandle);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function creates a block of process/thread specific memory for the 
 * currently executing process/thread. Only one block of memory will be
 * allocated for each thread and one additional block of memory which can
 * be fetch by all threads in the currently executing process. 
 *
 * Input:
 *
 *	Root must point to a NULL pointer on the first call to this function.
 *
 *	ThreadFlag = KPPROCMEM will allocate one memory block accessible by
 *				all threads within the currently executing process.
 *
 *	ThreadFlag = KPTHREADMEM will allocate one memory block only accessible
 *				by the currently executing thread of the the process.
 *
 * Return:
 *
 *	a pointer to block of locked memory of requested size if no errors.
 *
 *	NULL if memory could not be allocated due to memory allocation failure
 *	or a block of memory has already been allocated for the process or thread. 
 * 
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 16, 1994
 *-------------------------------------------------------------------*/
KpGenericPtr_t KpThreadMemCreate (
				KpThreadMemHdl_t FAR *	RootId,
				KpInt32_t				ThreadFlag,
				KpUInt32_t				Size)
{
	KpGenericPtr_t	memory;
	KpRootThread_t	FAR *pRoot;
	KpThreadMem_t	FAR *pSlot;

	KpInitializeCriticalSection (&theCriticalThing);
	if (0 != KpEnterCriticalSection (&theCriticalThing))
		return NULL;

	memory = NULL;
	pRoot = lockSlotBase ();
	if (NULL != pRoot) {

	/* look for block already defined */
		pSlot = findThreadRoot (pRoot, RootId, ThreadFlag);

	/* block not found, create requested memory block */
		if (NULL == pSlot)
			memory = allocBufferPtr (Size);

	/* see if memory was allocated */
		if (NULL != memory) {

		/* add this block to the table */
			if (NULL == addNewSlotItem (pRoot, RootId, ThreadFlag, memory)) {

			/* could not find space in the table for this entry */
				freeBufferPtr (memory);
				memory = NULL;
			}	
		}	
		
		unlockSlotBase ();	
	}
	KpLeaveCriticalSection (&theCriticalThing);
	return memory;	
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 16, 1994
 *-------------------------------------------------------------------*/
KpInt32_t KpThreadMemDestroy (
			KpThreadMemHdl_t FAR *	RootId,
			KpInt32_t		 		ThreadFlag)
{
	KpRootThread_t	FAR *pRoot;
	KpThreadMem_t	FAR *pSlot;
	KpInt32_t		status;

	if (KpEnterCriticalSection (&theCriticalThing) != 0)
		return KCMS_FAIL;

/* check for nothing to do, this is an error */
	if (NULL == RootListPtr) {
		KpLeaveCriticalSection (&theCriticalThing);
		return KCMS_FAIL;
	}

	status = KCMS_FAIL;
	pRoot = lockSlotBase ();
	if (NULL != pRoot) {
		pSlot = findThreadRoot (pRoot, RootId, ThreadFlag);
		status = deleteSlotItem (pRoot, pSlot, ThreadFlag);
		unlockSlotBase ();
	}
	KpLeaveCriticalSection(&theCriticalThing);
	return status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete Critical Section memory.
 *
 * AUTHOR
 * 	msm
 *
 * DATE CREATED
 *	May 15, 2002
 *------------------------------------------------------------------*/
void KpDelThreadMemCS ()
{
	KpDeleteCriticalSection (&theCriticalThing);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 16, 1994
 *-------------------------------------------------------------------*/
KpGenericPtr_t KpThreadMemFind (
				KpThreadMemHdl_t FAR *	RootId,
				KpInt32_t		 		ThreadFlag)
{
	void			FAR *memory;
	KpRootThread_t	FAR *pRoot;
	KpThreadMem_t	FAR *pSlot;

	if (KpEnterCriticalSection (&theCriticalThing) != 0)
		return NULL;

	memory = NULL;
	pRoot = lockSlotBase ();
	if (NULL != pRoot) {
		pSlot = findThreadRoot (pRoot, RootId, ThreadFlag);
		if (NULL != pSlot) {
			if (NULL != pSlot->DataHandle) {
				memory = lockBuffer (pSlot->DataHandle);
				pSlot->LockCount++;
			}
		}

		unlockSlotBase ();
	}
	KpLeaveCriticalSection (&theCriticalThing);
	return memory;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *
 * AUTHOR
 *  Norton 
 *
 * Last modified by: LSH
 *
 * DATE CREATED
 *  May 16, 1994
 *-------------------------------------------------------------------*/
void KpThreadMemUnlock (
			KpThreadMemHdl_t FAR *	RootId,
			KpInt32_t		 		ThreadFlag)
{
	KpRootThread_t	FAR *pRoot;
	KpThreadMem_t	FAR *pSlot;

	if (KpEnterCriticalSection (&theCriticalThing) != 0)
		return;

	pRoot = lockSlotBase ();
	if (NULL != pRoot) {
		pSlot = findThreadRoot (pRoot, RootId, ThreadFlag);
		if (NULL != pSlot) {
			if (NULL != pSlot->DataHandle) {

			/* check for last nested unlock request */
				if (1 == pSlot->LockCount)
					unlockBuffer (pSlot->DataHandle);

			/* don't decrement lock counter below 0 */
				if (0 < pSlot->LockCount)
					pSlot->LockCount--;
			}
		}

		unlockSlotBase ();
	}
	KpLeaveCriticalSection (&theCriticalThing);
}
