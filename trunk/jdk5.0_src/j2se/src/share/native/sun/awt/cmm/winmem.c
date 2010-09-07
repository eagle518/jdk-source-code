/*
 * @(#)winmem.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * WINMEM.C - Windows Memory Allocation Functions
 *
 *
 * Rev Level: @(#)winmem.c	1.11 12/2/98
 *
 *	Windows Revision Level:
 *		$Workfile: winmem.c $
 *		$Logfile: /DLL/KodakCMS/kpsys_lib/winmem.c $
 *		$Revision: 6 $
 *		$Date: 5/15/02 1:45p $
 *		$Author: Msm $
 *
 */

/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997-2002                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/


#include "kcms_sys.h"

void KpDelThreadMemCS ();

#define KpInvProcId		((KpInt32_t) 0)

/***********************************************************************
	This structure is at the beginning of each
	block of sub allocated memory.
 ***********************************************************************/
typedef struct {
	BOOL		InUse;		/* Block is allocaed flag */
	KpUInt32_t	SubSize;	/* Size of sub block including user data */
} KpMemHeapSubBlk_t;

/***********************************************************************
	This structure is at the beginning of each block of
	memory allocated by the system (GlobalAlloc).
 ***********************************************************************/
typedef struct KpMemHeapBlk_tag {
	struct KpMemHeapBlk_tag		FAR *Next;	/* Linked list of large blocks */
	struct KpMemHeapBlk_tag		FAR *Prev;

	KpUInt32_t		Size;		/* Size of allocated block */
	KpUInt32_t		SubBlocks;	/* Number of subblocks */
	KpUInt32_t		FreeBlocks;	/* Number of free subblocks */
} KpMemHeapBlk_t;

/***********************************************************************
	This structure is used to find the list of memory blocks for
	particular process.  Note that a special process ID will be
	used for memory that is allocated to be shared between tasks
	on Windows 3.x and Win32s.
 ***********************************************************************/
typedef struct {
	KpInt32_t		ProcId;
	KpMemHeapBlk_t	SubRoot;
} KpMemHeapRoot_t;

#define KpMemHeapsRootsInit	1
#define KpMemHeapsRootsInc	5
typedef struct {
	KpUInt32_t		Allocated;	/* size of Root array */
	KpUInt32_t		Used;		/* number of entries used */
	KpMemHeapRoot_t	Roots [1];	/* the 1 is a place holder */
} KpMemHeapRoots_t;

/* critical section flag for memory allocation */
static KpCriticalFlag_t	CritFlag = {0};

/* Point to list of heap roots */
static KpMemHeapRoots_t	FAR *HeapRoots = NULL;

/* constants -- initialized when first block is allocated */
static KpUInt32_t	KpMemAlignSize = 1;
static KpUInt32_t	KpMemSubBlockMinSize = 0;
static KpUInt32_t	KpMemSubBlkHdrSize = 0;
static KpUInt32_t	KpMemBlkHdrSize = 0;

/* constants -- initialized when first block is allocated and debug enabled */
static KpUInt32_t	KpMemSubAllocBlkSize = 16*1024;
static KpUInt32_t	KpMemAllocExtra = 0;
static int			KpMemInitValue = -1;
static int			KpMemLogLocks = 0;

#if defined (MCHK)
#include <stdio.h>

static char KpMemSettingsFile [] = "kpcms.ini";
static char KpMemSettingsSection [] = "DEBUG";

long KpSeqNumStopAt=-1;
static long	KpMemLogSeqNum = 0;
static long	KpMemLogUseCount;
static BOOL KpMemLogError;
static char	KpMemLogName [256];

typedef enum {
	KpMemLogAlloc,
	KpMemLogFree,
	KpMemLogLock,
	KpMemLogUnlock,
} KpMemLogType_t;


/*--------------------------------------------------------------------
 * DESCRIPTION
 *  Set debugger break point here to get control at memory
 *  allocation/free sequence number specified in KpSeqNumStopAt.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
void KpMemLogBreak (void)
{
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do memory allocation logging.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
static void KpMemLogFunc (void FAR *Ptr, long Size, KpMemLogType_t Type)
{
	char	MsgText [81];
	HFILE	hLogFileId;

	if (NULL == Ptr)
		return;

/* check for ignoring lock/unlock */
	if (!KpMemLogLocks) {
		if ((KpMemLogLock == Type) || (KpMemLogUnlock == Type))
			return;
	}

/* check for first time */
	if (0 == KpMemLogSeqNum) {
		++KpMemLogSeqNum;
		KpMemLogError = FALSE;
		KpMemLogUseCount = 0;

	/* get the log file name */
		if (0 == GetPrivateProfileString (KpMemSettingsSection,
										"MemoryTraceFile", "",
										KpMemLogName, sizeof(KpMemLogName),
										KpMemSettingsFile)) {
			KpMemLogError = TRUE;
			return;
		}

	/* create the log file */
		hLogFileId = _lcreat (KpMemLogName, 0);
		if (HFILE_ERROR == hLogFileId) {
			KpMemLogError = TRUE;
			return;
		}
	}
	else {
		++KpMemLogSeqNum;

	/* open the existing log file */
		hLogFileId = _lopen (KpMemLogName, OF_WRITE);
		if (HFILE_ERROR == hLogFileId) {
			KpMemLogError = TRUE;
			return;
		}
	}

/* position to end of file */
	_llseek (hLogFileId, 0L, 2);

/* create proper log message */
	switch (Type) {
	case KpMemLogAlloc:
		KpMemLogUseCount++;
		sprintf (MsgText, "%08lx Allocate %9ld %9ld %9ld\015\012",
	         Ptr, KpMemLogSeqNum, KpMemLogUseCount, Size);
		break;

	case KpMemLogFree:
		KpMemLogUseCount--;
		sprintf (MsgText, "%08lx Free     %9ld %9ld\015\012",
	         Ptr, KpMemLogSeqNum, KpMemLogUseCount);
		break;

	case KpMemLogLock:
		sprintf (MsgText, "%08lx Lock     %9ld %9ld\015\012",
	         Ptr, KpMemLogSeqNum, KpMemLogUseCount);
		break;

	case KpMemLogUnlock:
		sprintf (MsgText, "%08lx Unlock   %9ld %9ld\015\012",
	         Ptr, KpMemLogSeqNum, KpMemLogUseCount);
		break;

	default:
		sprintf (MsgText, "Unknown memory log type\015\012");
		break;
	}

/* write the message */
	_lwrite (hLogFileId, MsgText, lstrlen (MsgText));

/* close the log file */
	if (!KpMemLogError)
		_lclose( hLogFileId );

/* check for user requested break */
	if (KpMemLogSeqNum == KpSeqNumStopAt)
		KpMemLogBreak ();
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Initialize a block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
static void KpMemBlkInit (char KPHUGE *Ptr, KpUInt32_t Size)
{
	if (-1 != KpMemInitValue)
		KpMemSet (Ptr, KpMemInitValue, Size);
}

#define KpMemLogAlloc(Ptr, Size)	KpMemLogFunc (Ptr, Size, KpMemLogAlloc)
#define KpMemLogFree(Ptr)			KpMemLogFunc (Ptr, 0, KpMemLogFree)
#define KpMemLogLock(Ptr)			KpMemLogFunc (Ptr, 0, KpMemLogLock)
#define KpMemLogUnlock(Ptr)			KpMemLogFunc (Ptr, 0, KpMemLogUnlock)

#else

#define KpMemLogAlloc(Ptr, Size)
#define KpMemLogFree(Ptr)
#define KpMemLogLock(Ptr)
#define KpMemLogUnlock(Ptr)
#define KpMemBlkInit(Ptr, Size)

#endif


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the ID of the currently executing process.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpInt32_t KpGetProcId (BOOL Shared)
{
	if (Shared)
		return KpInvProcId;

 	return KpGetCurrentProcessId ();
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate memory from the Global Heap.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void FAR *DoGlobalAlloc (
			BOOL		Shared,
			KpUInt32_t	Size)
{
	HGLOBAL	MemHandle;

	MemHandle = GlobalAlloc (Shared ? GMEM_SHARE : GMEM_MOVEABLE, Size);
	if (NULL == MemHandle)
		return NULL;

	return GlobalLock (MemHandle);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free memory from the Global Heap.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	July 19, 1994
 *------------------------------------------------------------------*/
static void DoGlobalFree (
			void	FAR *Ptr)
{
	HGLOBAL	MemHandle;

#if defined (KPWIN32)
	MemHandle = GlobalHandle (Ptr);
#else
	MemHandle = (HGLOBAL) LOWORD (GlobalHandle (SELECTOROF (Ptr)));
#endif

	GlobalFree (MemHandle);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Round size up to size of most restrictive data type size.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpUInt32_t KpAlignedSizeOf (
			KpUInt32_t		Size)
{
	Size += (KpMemAlignSize - 1);
	Size /= KpMemAlignSize;
	Size *= KpMemAlignSize;

	return Size;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do runtime initialization of globals.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void KpMemInitGlobals (void)
{
	KpMemAlignSize = sizeof (long);
	KpMemBlkHdrSize = KpAlignedSizeOf (sizeof (KpMemHeapBlk_t));
	KpMemSubBlkHdrSize = KpAlignedSizeOf (sizeof (KpMemHeapSubBlk_t));
	KpMemSubBlockMinSize = KpMemSubBlkHdrSize + 4 * KpMemAlignSize;

#if defined (MCHK)
	KpMemAllocExtra = GetPrivateProfileInt (KpMemSettingsSection,
											"MemoryAllocFudge", 0,
											KpMemSettingsFile);

	KpMemInitValue = GetPrivateProfileInt (KpMemSettingsSection,
											"MemoryInitValue", -1,
											KpMemSettingsFile);

	KpMemSubAllocBlkSize = GetPrivateProfileInt (KpMemSettingsSection,
												"MemorySubBlkSize", 16*1024,
												KpMemSettingsFile);

	KpMemLogLocks = GetPrivateProfileInt (KpMemSettingsSection,
											"MemoryLogLocks", 0,
											KpMemSettingsFile);
#endif
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get pointer to first sub allocation block header.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpMemHeapSubBlk_t FAR *KpMemHeapSubFirst (
			KpMemHeapBlk_t	FAR *Block)
{
	char	KPHUGE *Ptr;

	Ptr = (char KPHUGE *) Block;
	Ptr += KpAlignedSizeOf (sizeof (*Block));

	return (KpMemHeapSubBlk_t FAR *) Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get pointer to next sub allocation block header.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpMemHeapSubBlk_t FAR *KpMemHeapSubNext (
			KpMemHeapBlk_t		FAR *Block,
			KpMemHeapSubBlk_t	FAR *SubBlock)
{
	char	KPHUGE *Ptr;
	char		KPHUGE	*Last;

	Ptr = (char KPHUGE *) SubBlock;
	Ptr += SubBlock->SubSize;

/* check for last block */
	Last = ((char KPHUGE *) Block) + Block->Size;
	if (Last <= Ptr)
		return NULL;

	return (KpMemHeapSubBlk_t FAR *) Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get pointer to user data in suballocation block.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void FAR *KpMemHeapSubData (
			KpMemHeapSubBlk_t	FAR *SubBlock)
{
	char	KPHUGE *Ptr;

	Ptr = (char KPHUGE *)SubBlock;
	Ptr += KpMemSubBlkHdrSize;

	return (void FAR *) Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Find sub block the given address represents.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static BOOL KpMemHeapSubFind (
			KpMemHeapBlk_t		FAR *Block,
			void				FAR *Ptr,
			KpMemHeapSubBlk_t	FAR * FAR *PrevBlk,
			KpMemHeapSubBlk_t	FAR * FAR *SubBlk)
{
	KpUInt32_t	i;
	char		KPHUGE	*First;
	char		KPHUGE	*Last;

/* initialize return values */
	*PrevBlk = NULL;
	*SubBlk = NULL;

/* check that given address is within this block */
	First = ((char KPHUGE *) Block) + KpMemSubBlkHdrSize + KpMemBlkHdrSize;
	Last = ((char KPHUGE *) Block) +Block->Size;
	if (((char KPHUGE *) Ptr < First) || (Last < (char KPHUGE *) Ptr))
		return FALSE;

/* look at all the sub blocks for this user data address */
	for (i = 0, *SubBlk = KpMemHeapSubFirst (Block);
			i < Block->SubBlocks;
					i++, *SubBlk = KpMemHeapSubNext (Block, *SubBlk)) {
		if (Ptr == KpMemHeapSubData (*SubBlk))
			return TRUE;

		*PrevBlk = *SubBlk;
	}

	return FALSE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free a sub allocated block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static BOOL KpMemHeapSubFree (
			KpMemHeapBlk_t	FAR *Block,
			void			FAR *Ptr)
{
	KpMemHeapSubBlk_t	FAR *SubBlk;
	KpMemHeapSubBlk_t	FAR *PrevBlk;
	KpMemHeapSubBlk_t	FAR *NextBlk;

/* try to locate the sub block */
	if (!KpMemHeapSubFind (Block, Ptr, &PrevBlk, &SubBlk))
		return FALSE;

/* deallocate the sub block */
	SubBlk->InUse = FALSE;
	Block->FreeBlocks++;

/* try to combine this block with previous block */
	if (NULL != PrevBlk) {
		if (!PrevBlk->InUse) {
			Block->FreeBlocks--;
			Block->SubBlocks--;
			PrevBlk->SubSize += SubBlk->SubSize;
			SubBlk = PrevBlk;
		}
	}

/* try to combine this block with next block */
	NextBlk = KpMemHeapSubNext (Block, SubBlk);
	if (NULL != NextBlk) {
		if (!NextBlk->InUse) {
			Block->FreeBlocks--;
			Block->SubBlocks--;
			SubBlk->SubSize += NextBlk->SubSize;
		}
	}

	return TRUE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Look for free sub block of needed size.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void FAR *KpMemHeapSubAlloc (
			KpMemHeapBlk_t	FAR *Block,
			KpUInt32_t		Size)
{
	KpMemHeapSubBlk_t	FAR *SubBlk;
	KpMemHeapSubBlk_t	FAR *NewBlk;
	KpUInt32_t			i;
	KpUInt32_t			BlkSizeOrg;

	if (NULL == Block)
		return NULL;

	if (0 == Block->FreeBlocks)
		return NULL;

	Size = KpAlignedSizeOf (Size);

	for (i = 0, SubBlk = KpMemHeapSubFirst (Block);
			i < Block->SubBlocks;
					i++, SubBlk = KpMemHeapSubNext (Block, SubBlk)) {
		if (SubBlk->InUse)
			continue;
		
		if (SubBlk->SubSize < Size + KpMemSubBlkHdrSize)
			continue;

		if (SubBlk->SubSize - Size < KpMemSubBlockMinSize) {

		/* just use the existing sub block */
			SubBlk->InUse = TRUE;
			Block->FreeBlocks--;
			return KpMemHeapSubData (SubBlk);
		}
		else {

		/* split the sub block */
			BlkSizeOrg = SubBlk->SubSize;
			SubBlk->SubSize = Size + KpMemSubBlkHdrSize;
			SubBlk->InUse = TRUE;

			NewBlk = KpMemHeapSubNext (Block, SubBlk);
			NewBlk->SubSize = BlkSizeOrg - SubBlk->SubSize;
			NewBlk->InUse = FALSE;

			Block->SubBlocks++;
			return KpMemHeapSubData (SubBlk);
		}
	}

/* if we get here we did not find a subblock to allocate */
	return NULL;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create new block for sub allocation.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpMemHeapBlk_t FAR *KpMemHeapSubCreate (
			KpMemHeapBlk_t	FAR *Root,
			BOOL			Shared,
			KpUInt32_t		Size)
{
	KpMemHeapBlk_t		FAR *Block;
	KpMemHeapSubBlk_t	FAR *SubBlk;
	KpUInt32_t			AllocedSize;

	Size = KpAlignedSizeOf (Size);
	AllocedSize = Size + KpMemBlkHdrSize + KpMemSubBlkHdrSize;

	Block = DoGlobalAlloc (Shared, AllocedSize);
	if (NULL == Block)
		return NULL;

/* initialize the block data */
	Block->Size = AllocedSize;
	Block->SubBlocks = 1;
	Block->FreeBlocks = 1;

/* initialize the sub block, only one and it is free */
	SubBlk = KpMemHeapSubFirst (Block);
	SubBlk->InUse = FALSE;
	SubBlk->SubSize = Size + KpMemSubBlkHdrSize;

/* link this block into the list */
	Block->Next = Root;
	Block->Prev = Root->Prev;
	Block->Next->Prev = Block;
	Block->Prev->Next = Block;

	return Block;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Destroy a block for sub allocation.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 18, 1994
 *------------------------------------------------------------------*/
static void KpMemHeapSubDestroy (
			KpMemHeapBlk_t	FAR *Blk)
{
	Blk->Next->Prev = Blk->Prev;
	Blk->Prev->Next = Blk->Next;

	DoGlobalFree (Blk);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Find root for the specified process.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpMemHeapRoot_t FAR *KpMemHeapFind (
			KpInt32_t	ProcId)
{
	KpUInt32_t		i;
	KpMemHeapRoot_t	KPHUGE *Root;

/* check for no heaps allocated */
	if (NULL == HeapRoots)
		return NULL;

	for (i = 0, Root = HeapRoots->Roots;
			i < HeapRoots->Used;
					i++, Root++) {
	
		if (ProcId == Root->ProcId)
			return (KpMemHeapRoot_t FAR *) Root;
	}

	return NULL;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Add root for the specified process.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static KpMemHeapRoot_t FAR *KpMemHeapAdd (
			KpInt32_t	ProcId)
{
	KpMemHeapRoots_t	FAR *TempRoots;
	KpMemHeapRoot_t		KPHUGE *SrcPtr;
	KpMemHeapRoot_t		KPHUGE *DestPtr;
	KpUInt32_t			NewAllocated;
	KpUInt32_t			Index;

/* check for no heaps allocated */
	if (NULL == HeapRoots) {
		KpMemInitGlobals ();
		HeapRoots = DoGlobalAlloc (TRUE, sizeof (KpMemHeapRoot_t) +
					KpMemHeapsRootsInit * sizeof (KpMemHeapRoot_t));

		if (NULL == HeapRoots)
			return NULL;

		HeapRoots->Allocated = KpMemHeapsRootsInit;
		HeapRoots->Used = 0;
	}

/* check for room to add a heap */
	if (HeapRoots->Used == HeapRoots->Allocated) {
		NewAllocated = HeapRoots->Allocated + KpMemHeapsRootsInc;
		TempRoots = DoGlobalAlloc (TRUE, sizeof (KpMemHeapRoot_t) +
					NewAllocated * sizeof (KpMemHeapRoot_t));
		if (NULL == TempRoots)
			return NULL;

		TempRoots->Allocated = NewAllocated;
		TempRoots->Used = HeapRoots->Used;
		SrcPtr = HeapRoots->Roots;
		DestPtr = TempRoots->Roots;
		for (Index = 0; Index < HeapRoots->Used; Index++) {
			*DestPtr = *SrcPtr;
			if (DestPtr->SubRoot.Next == &SrcPtr->SubRoot)
				DestPtr->SubRoot.Next = &DestPtr->SubRoot;
			if (DestPtr->SubRoot.Prev == &SrcPtr->SubRoot)
				DestPtr->SubRoot.Prev = &DestPtr->SubRoot;
			DestPtr->SubRoot.Next->Prev = &DestPtr->SubRoot;
			DestPtr->SubRoot.Prev->Next = &DestPtr->SubRoot;
			DestPtr++;
			SrcPtr++;
		}

		DoGlobalFree (HeapRoots);
		HeapRoots = TempRoots;
	}

/* initialize next available entry */
	HeapRoots->Used++;
	DestPtr = HeapRoots->Roots;
	DestPtr += HeapRoots->Used - 1;

	DestPtr->ProcId = ProcId;

	DestPtr->SubRoot.Next = &DestPtr->SubRoot;
	DestPtr->SubRoot.Prev = &DestPtr->SubRoot;
	DestPtr->SubRoot.Size = 0;
	DestPtr->SubRoot.SubBlocks = 0;
	DestPtr->SubRoot.FreeBlocks = 0;

	return (KpMemHeapRoot_t FAR *) DestPtr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate memory find and allocate block.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void FAR *KpMemAllocDoIt (
			BOOL		Shared,
			KpUInt32_t	Size)
{
	KpInt32_t		ProcId;
	KpMemHeapRoot_t	FAR *HeapRoot;
	KpMemHeapBlk_t	FAR *HeapBlk;
	void			FAR *Ptr;

/* get process id */
	ProcId = KpGetProcId (Shared);

/* find root for this process */
	HeapRoot = KpMemHeapFind (ProcId);

/* no root, create one */
	if (NULL == HeapRoot)
		HeapRoot = KpMemHeapAdd (ProcId);

/* still no root, must be out of memory, return error */
	if (NULL == HeapRoot)
		return NULL;

/* add optional extra space, used for overwrite testing */
	Size += KpMemAllocExtra;

/* handle special case of large blocks */
	if (Size > KpMemSubAllocBlkSize) {
#if defined (KPWIN32)
		HeapBlk = KpMemHeapSubCreate (&HeapRoot->SubRoot, Shared, Size);
		if (NULL == HeapBlk)
			return NULL;

		return KpMemHeapSubAlloc (HeapBlk, Size);
#else
		return DoGlobalAlloc (Shared, Size);
#endif
	}

/* look through blocks trying to find space we can use */
	for (HeapBlk = HeapRoot->SubRoot.Next;
			HeapBlk != &HeapRoot->SubRoot;
					HeapBlk = HeapBlk->Next) {
		Ptr = KpMemHeapSubAlloc (HeapBlk, Size);
		if (NULL != Ptr)
			return Ptr;
	}

/*
 * since we are here, we did not find space in any
 * of the currently allocated blocks
 */

	HeapBlk = KpMemHeapSubCreate (&HeapRoot->SubRoot, Shared,
									KpMemSubAllocBlkSize);
	if (NULL == HeapBlk)
		return NULL;

	return KpMemHeapSubAlloc (HeapBlk, Size);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free an allocated block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static void KpMemFreeDoIt (
			BOOL	Shared,
			void	FAR *Ptr)
{
	KpInt32_t		ProcId;
	KpMemHeapRoot_t	FAR *HeapRoot;
	KpMemHeapBlk_t	FAR *HeapBlk;

#if !defined (KPWIN32)
	if (0 == LOWORD (Ptr)) {
		DoGlobalFree (Ptr);
		return;
	}
#endif

/* get process id */
	ProcId = KpGetProcId (Shared);

/* find root for this process */
	HeapRoot = KpMemHeapFind (ProcId);
	if (NULL == HeapRoot)
		return;

/* look through blocks trying to find the block space */
	for (HeapBlk = HeapRoot->SubRoot.Next;
			HeapBlk != &HeapRoot->SubRoot;
					HeapBlk = HeapBlk->Next) {
		if (KpMemHeapSubFree (HeapBlk, Ptr)) {

		/* check for freeing last sub block */
			if (HeapBlk->SubBlocks == HeapBlk->FreeBlocks)
				KpMemHeapSubDestroy (HeapBlk);

			return;
		}
	}
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get size of an allocated block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static KpUInt32_t KpMemGetSizeDoIt (
			BOOL	Shared,
			void	FAR *Ptr)
{
	KpInt32_t			ProcId;
	KpMemHeapRoot_t		FAR *HeapRoot;
	KpMemHeapBlk_t		FAR *HeapBlk;
	KpMemHeapSubBlk_t	FAR *PrevBlk;
	KpMemHeapSubBlk_t	FAR *SubBlk;

#if !defined (KPWIN32)
	if (0 == LOWORD (Ptr))
		return GlobalSize ((HGLOBAL) LOWORD (GlobalHandle (SELECTOROF (Ptr))));
#endif

/* get process id */
	ProcId = KpGetProcId (Shared);

/* find root for this process */
	HeapRoot = KpMemHeapFind (ProcId);
	if (NULL == HeapRoot)
		return 0;

/* look through blocks trying to find the block space */
	for (HeapBlk = HeapRoot->SubRoot.Next;
			HeapBlk != &HeapRoot->SubRoot;
					HeapBlk = HeapBlk->Next) {
		if (KpMemHeapSubFind (HeapBlk, Ptr, &PrevBlk, &SubBlk))
			return SubBlk->SubSize - KpMemSubBlkHdrSize;
	}

	return 0;
}

#if defined (KPWIN32)

#define HEAP_FLAG	0


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the version of Windows(NT, Chicago, Win32s under 3.1).
 *
 * AUTHOR
 * 	pgt
 *
 * DATE CREATED
 *	Nov 30, 1993
 *------------------------------------------------------------------*/
static BOOL KpUseHeap (void)
{
	static BOOL	UseHeap = FALSE;
	static BOOL	Initialize = TRUE;
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
			UseHeap = TRUE;
		}
		else if (winOS & 0x8000) {
			/* WIN_TYPE_WIN32s */
			UseHeap = FALSE;
		}
		else {
			/* WIN_TYPE_NT */
			UseHeap = TRUE;
		}
	}

	return UseHeap;
}




/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get a heap handle if needed.
 *
 * AUTHOR
 * 	pgt
 *
 * DATE CREATED
 *	Nov 30, 1993
 *------------------------------------------------------------------*/
static HANDLE KpGetHeapHandle (void)
{
	static HANDLE	heapHandle = NULL;

	if (NULL == heapHandle)
		heapHandle = HeapCreate (0, 1024 * 1024, 0);

	return heapHandle;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate a block of memory, using heaps.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
static void FAR *KpMemAllocHeap (KpInt32_t Size)
{
	HANDLE	hHeap;
	void	FAR *Ptr;

/* add optional extra space, used for overwrite testing */
	Size += KpMemAllocExtra;

	hHeap = KpGetHeapHandle ();
	if (NULL != hHeap)
		Ptr = HeapAlloc (hHeap, HEAP_FLAG, Size);
	else
		Ptr = NULL;

	return Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free a block of memory, using heaps.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
static void KpMemFreeHeap (void FAR *Ptr)
{
	HANDLE	hHeap;

	hHeap = KpGetHeapHandle ();
	if (hHeap != NULL)
		HeapFree (hHeap, 0, Ptr);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get size of a block of memory, using heaps.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 4, 1993
 *------------------------------------------------------------------*/
static KpInt32_t KpMemGetSizeHeap (void FAR *ptr)
{
	HANDLE		hHeap;

	hHeap = KpGetHeapHandle ();
	if (NULL != hHeap)
		return HeapSize (hHeap, HEAP_FLAG, ptr);

	return 0;
}
#endif


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Initialize memory.
 *
 * AUTHOR
 * 	msm
 *
 * DATE CREATED
 *	Sept. 31, 1997
 *------------------------------------------------------------------*/
void KpMemInit ()
{
	KpSysInit ();		/* Initialize the kcms_sys memory functions */
	KpInitializeCriticalSection (&CritFlag);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Terminate memory.
 *
 * AUTHOR
 * 	msm
 *
 * DATE CREATED
 *	May 9, 2002
 *------------------------------------------------------------------*/
void KpMemTerm ()
{
	KpDeleteCriticalSection (&CritFlag);
	KpDelThreadMemCS ();
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 13, 1994
 *------------------------------------------------------------------*/
static void FAR *KpMemAlloc (
			BOOL		Shared,
			KpUInt32_t	Size)
{
	void			FAR *Ptr;

	if (KpEnterCriticalSection (&CritFlag))
		return NULL;

#if defined (KPWIN32)
	if (KpUseHeap ())
		Ptr = KpMemAllocHeap (Size);
	else
		Ptr = KpMemAllocDoIt (Shared, Size);
#else
	Ptr = KpMemAllocDoIt (Shared, Size);
#endif

	if (NULL != Ptr) {
		KpMemBlkInit (Ptr, Size);
		KpMemLogAlloc (Ptr, Size);
		KpMemLogLock (Ptr);
	}

	KpLeaveCriticalSection (&CritFlag);
	return Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free an allocated block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static void KpMemFree (
			BOOL	Shared,
			void	FAR *Ptr)
{
/* ignore NULL pointers */
	if (NULL == Ptr)
		return;

	if (!KpEnterCriticalSection (&CritFlag)) {

	/* log what is being freed */
		KpMemLogUnlock (Ptr);
		KpMemLogFree (Ptr);

#if defined (KPWIN32)
		if (KpUseHeap ())
			KpMemFreeHeap (Ptr);
		else
			KpMemFreeDoIt (Shared, Ptr);
#else
		KpMemFreeDoIt (Shared, Ptr);
#endif
		KpLeaveCriticalSection (&CritFlag);
	}
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get size of an allocated block of memory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static KpUInt32_t KpMemGetSize (
			BOOL	Shared,
			void	FAR *Ptr)
{
	KpUInt32_t	Size;

/* ignore NULL pointers */
	if (NULL == Ptr)
		return 0;

	if (KpEnterCriticalSection (&CritFlag))
		return 0;

#if defined (KPWIN32)
	if (KpUseHeap ())
		Size = KpMemGetSizeHeap (Ptr);
	else
		Size = KpMemGetSizeDoIt (Shared, Ptr);
#else
	Size = KpMemGetSizeDoIt (Shared, Ptr);
#endif

	KpLeaveCriticalSection (&CritFlag);
	return Size;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Change the size of an allocated memory block.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	May 15, 1994
 *------------------------------------------------------------------*/
static void FAR *KpMemRealloc (
			BOOL		Shared,
			void		FAR *OldPtr,
			KpUInt32_t	NewSize)
{
	KpUInt32_t	OldSize;
	void		FAR *NewPtr;

/* check to determine if we really need to change the amount of space */
	OldSize = KpMemGetSize (Shared, OldPtr);
	if (OldSize >= NewSize)
		return OldPtr;

/**********************/
/* we need more space */
/* do it the wimp way */
/**********************/

/* allocate a new block */
	NewPtr = KpMemAlloc (Shared, NewSize);
	if (NULL == NewPtr)
		return NULL;

/* copy the data */
	KpMemCpy (NewPtr, OldPtr, OldSize);

/* free the old block */
	KpMemFree (Shared, OldPtr);

/* tell caller where the data is now */
	return NewPtr;
}

/**********************************************************************
 * Public memory allocation interface (Process specific allocations)
 **********************************************************************/

/**********************************************************************/
KpHandle_t FAR allocBufferHandlePrv (KpInt32_t Size)
{
	return (KpHandle_t) KpMemAlloc (FALSE, (KpUInt32_t) Size);
}

/**********************************************************************/
KpHandle_t FAR allocLargeBufferHandle (KpInt32_t Size)
{
	return allocBufferHandle (Size);
}

/**********************************************************************/
KpGenericPtr_t FAR allocBufferPtr (KpInt32_t Size)
{
	return allocBufferHandle (Size);
}

/**********************************************************************/
KpGenericPtr_t FAR reallocBufferPtrPrv (KpGenericPtr_t OldPtr, KpInt32_t Size)
{
	return KpMemRealloc (FALSE, OldPtr, (KpUInt32_t) Size);
}

/**********************************************************************/
void FAR freeBufferPrv (KpHandle_t Handle)
{
	KpMemFree (FALSE, (void FAR *) Handle);
}

/**********************************************************************/
void FAR freeBufferPtr (KpGenericPtr_t Ptr)
{
	freeBuffer (Ptr);
}

/**********************************************************************/
KpInt32_t FAR getBufferSizePrv (KpHandle_t Handle)
{
	return (KpInt32_t) KpMemGetSize (FALSE, (void FAR *) Handle);
}

/**********************************************************************/
KpInt32_t FAR getPtrSize (KpGenericPtr_t Ptr)
{
	return getBufferSize (Ptr);
}

/**********************************************************************/
KpHandle_t FAR getHandleFromPtrPrv (KpGenericPtr_t Ptr)
{
	return (KpHandle_t) Ptr;
}

/**********************************************************************/
KpGenericPtr_t FAR lockBufferPrv (KpHandle_t Handle)
{
	KpMemLogLock (Handle);
	return (KpGenericPtr_t) Handle;
}

/**********************************************************************/
KpInt32_t FAR unlockBufferPrv (KpHandle_t Handle)
{
	KpMemLogUnlock (Handle);
	return (NULL != Handle) ? TRUE : FALSE;
}

/**********************************************************************/
KpHandle_t FAR unlockBufferPtr (KpGenericPtr_t Ptr)
{
KpHandle_t	Handle;
KpInt32_t	result;

	Handle = getHandleFromPtr (Ptr);
	result = unlockBuffer (Handle);
	if (result) {
		return Handle;
	} else {
		return NULL;
	}
}

/**********************************************************************
 * Public memory allocation interface ("Shared" allocations)
 **********************************************************************/

/**********************************************************************/
KpHandle_t FAR allocSysBufferHandle (KpInt32_t Size)
{
	return allocBufferHandle (Size);
}

/**********************************************************************/
KpHandle_t FAR allocSysLargeBufferHandle (KpInt32_t Size)
{
	return allocBufferHandle (Size);
}

/**********************************************************************/
KpGenericPtr_t FAR allocSysBufferPtr (KpInt32_t Size)
{
	return allocBufferPtr (Size);
}

/**********************************************************************/
void FAR freeSysBuffer (KpHandle_t Handle)
{
	freeBuffer (Handle);
}

/**********************************************************************/
void FAR freeSysBufferPtr (KpGenericPtr_t Ptr)
{
	freeBufferPtr (Ptr);
}

/**********************************************************************/
KpInt32_t FAR getSysBufferSize (KpHandle_t Handle)
{
	return getBufferSize (Handle);
}

/**********************************************************************/
KpInt32_t FAR getSysPtrSize (KpGenericPtr_t Ptr)
{
	return getPtrSize (Ptr);
}

/**********************************************************************/
KpHandle_t FAR getSysHandleFromPtr (KpGenericPtr_t Ptr)
{
	return getHandleFromPtr (Ptr);
}

/**********************************************************************/
KpGenericPtr_t FAR lockSysBuffer (KpHandle_t Handle)
{
	return lockBuffer (Handle);
}

/**********************************************************************/
KpInt32_t FAR unlockSysBuffer (KpHandle_t Handle)
{
	return unlockBuffer (Handle);
}

/**********************************************************************/
KpHandle_t FAR unlockSysBufferPtr (KpGenericPtr_t Ptr)
{
	return unlockBufferPtr (Ptr);
}


/*
 ***************************************************************
 ***************************************************************
 * We put in stubs for the heap functions when the Macintosh
 * cross compiler is being used because they were too stupid
 * to stub these themselves in their libraries. We don't use
 * them for Win32s and Mac so it does not matter
 ***************************************************************
 ***************************************************************
 */
#if defined(KPMSMAC)

HANDLE HeapCreate(DWORD dwOptions,DWORD dwInitSize,DWORD dwMaxSize)
{
	return   NULL;
}

LPVOID HeapAlloc(HANDLE hHeap,DWORD dwFlags,DWORD dwBytes)
{
	return   NULL;
}

BOOL HeapFree(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem)
{
	return   FALSE;
}

DWORD HeapSize(HANDLE hHeap,DWORD dwFlags,LPCVOID lpMem)
{
	return   0;
}

#endif	/* defined(KPMSMAC) */



/*************************/
/*************************/
/* FIXME -- Test Program */
/*************************/
/*************************/
#if 0

#include <stdio.h>
#include <string.h>

/**********************************************************************/
static void KpMemHeapSubDump (
			FILE			*log,
			KpMemHeapBlk_t	FAR *Block)
{
	KpUInt32_t			i;
	KpMemHeapSubBlk_t	FAR *SubBlk;

	fprintf (log, "Subblock at %Fp Size: %ld SubBlocks: %ld FreeBlocks: %ld\n",
				Block, Block->Size, Block->SubBlocks, Block->FreeBlocks);
	for (i = 0, SubBlk = KpMemHeapSubFirst (Block);
			i < Block->SubBlocks;
					i++, SubBlk = KpMemHeapSubNext (Block, SubBlk)) {
		fprintf (log, "  %3ld %s %ld\n", i, (SubBlk->InUse) ? "Used" : "Free",
								SubBlk->SubSize);
	}
}

/**********************************************************************/
static void KpMemHeapDump (
			FILE			*log,
			KpMemHeapRoot_t	FAR *HeapRoot)
{
	KpMemHeapBlk_t	FAR *HeapBlk;

	fprintf (log, "Heap at %Fp ProcId: %ld\n", HeapRoot, HeapRoot->ProcId);

/* dump all the sub allocated blocks */
	for (HeapBlk = HeapRoot->SubRoot.Next;
			HeapBlk != &HeapRoot->SubRoot;
					HeapBlk = HeapBlk->Next)
		KpMemHeapSubDump (log, HeapBlk);
}

/**********************************************************************/
static void KpMemDump (FILE *log)
{
	KpUInt32_t		i;
	KpMemHeapRoot_t	KPHUGE *Root;

	if (NULL == HeapRoots)
		return;

	for (i = 0, Root = HeapRoots->Roots;
			i < HeapRoots->Used;
					i++, Root++)
		KpMemHeapDump (log, (KpMemHeapRoot_t FAR *) Root);
}

/**********************************************************************/
static void SubAllocTest (FILE *log)
{
	KpMemHeapBlk_t	Root;
	KpMemHeapBlk_t	FAR *Blk;
	void			FAR *Ptr1;
	void			FAR *Ptr2;
	void			FAR *Ptr3;
	void			FAR *Ptr4;
	void			FAR *Ptr5;

	fprintf (log, "Starting Suballocation test.\n");

	KpMemInitGlobals ();

	Root.Next =
	Root.Prev = &Root;

	Blk = KpMemHeapSubCreate (&Root, FALSE, 1024);
	fprintf (log, "Blk: %Fp\n", Blk);
	KpMemHeapSubDump (log, Blk);

	Ptr1 = KpMemHeapSubAlloc (Blk, 50);
	fprintf (log, "Ptr1: %Fp\n", Ptr1);

	Ptr2 = KpMemHeapSubAlloc (Blk, 50);
	fprintf (log, "Ptr2: %Fp\n", Ptr2);

	Ptr5 = KpMemHeapSubAlloc (Blk, 50);
	fprintf (log, "Ptr5: %Fp\n", Ptr5);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr2);
	KpMemHeapSubDump (log, Blk);

	Ptr2 = KpMemHeapSubAlloc (Blk, 10);
	fprintf (log, "Ptr2: %Fp\n", Ptr2);

	Ptr3 = KpMemHeapSubAlloc (Blk, 10);
	fprintf (log, "Ptr3: %Fp\n", Ptr3);

	Ptr4 = KpMemHeapSubAlloc (Blk, 10);
	fprintf (log, "Ptr4: %Fp\n", Ptr4);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr1);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr2);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr5);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr3);
	KpMemHeapSubDump (log, Blk);

	KpMemHeapSubFree (Blk, Ptr4);
	KpMemHeapSubDump (log, Blk);

	fprintf (log, "End of Suballocation test.\n");
}

/**********************************************************************/
static void FAR *MemAllocTestDoIt (
			FILE		*log,
			BOOL 		Shared,
			KpUInt32_t	Size,
			char		FAR *Name)
{
	void	FAR *Ptr;

	Ptr = KpMemAlloc (Shared, Size);
	fprintf (log, "%s (%ld): %Fp %ld\n",
					Name, Size, Ptr, KpMemGetSize (Shared, Ptr));
	return Ptr;
}

/**********************************************************************/
static void MemAllocTest (FILE *log)
{
	void			FAR *Ptr1;
	void			FAR *Ptr2;
	void			FAR *Ptr3;
	void			FAR *Ptr4;
	void			FAR *Ptr5;

	Ptr1 = MemAllocTestDoIt (log, FALSE, 20000, "Ptr1");
	Ptr2 = MemAllocTestDoIt (log, TRUE,  20000, "Ptr2");
	Ptr3 = MemAllocTestDoIt (log, FALSE, 20000, "Ptr3");
	Ptr4 = MemAllocTestDoIt (log, TRUE,      2, "Ptr4");
	Ptr5 = MemAllocTestDoIt (log, TRUE,      1, "Ptr5");

	KpMemDump (log);

	KpMemFree (FALSE, Ptr1);
	KpMemFree (FALSE, Ptr3);
	KpMemDump (log);

	KpMemFree (TRUE, Ptr2);
	KpMemFree (TRUE, Ptr4);
	KpMemFree (TRUE, Ptr5);
	KpMemDump (log);
}

/**********************************************************************/
int main (void)
{
	FILE	*log;
	char	LogName [512];
	char	*Ptr;

	Ptr = getenv ("TEMP");
	if (NULL != Ptr)
		strcpy (LogName, Ptr);
	else
		strcpy (LogName, "c:");

	strcat (LogName, "\\winmem.log");

	log = fopen (LogName, "w");
	if (NULL == log)
		return 1;

	fprintf (log, "Hello, Start of tests.\n");
/*	SubAllocTest (log); */
	MemAllocTest (log);
	fprintf (log, "Good Bye, end of tests.\n");

	fclose (log);

	return 0;
}
#endif
