/*
 * @(#)unixmem.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains routines to manage memory for
				unix versions of this library.

				Created by sek, June 26, 1949

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1994 - 2003 by Eastman Kodak Company, all rights reserved.

	Windows Revision Level:
		$Workfile: unixmem.c $
		$Logfile: /DLL/KodakCMS/kpsys_lib/unixmem.c $
		$Revision: 5 $
		$Date: 4/03/03 11:24a $
		$Author: Arourke $

	SCCS Revision:
		@(#)unixmem.c	1.9	12/22/97

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1994-2003                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "kcms_sys.h"
#include <stdlib.h>

#define HEADER	8     /* number of extra bytes to add to the byte count */
#define DLWORD	2     /* number of long words in a double */

#if defined(MCHK)
#include <string.h>

static KpFileProps_t	KCPDataDirProps;
static long		KpSeqNumStopAt=-1;
static long		lLogSeqNum = 0;
static long		lLogUseCount;
static KpBool_t		fLogFileError;
static int		FudgeValue = 0;
static int		InitValue = -1;

typedef enum {
	MemLogAlloc,
	MemLogFree
} MemLogType_t;


/************************************************************************/
static KpInt32_t KpMemFudge (KpInt32_t Size)
{
	return Size + FudgeValue;
}

/************************************************************************/
static void initMemBlk (KpHandle_t Ptr, KpInt32_t Size)
{
	if (-1 != InitValue)
		KpMemSet (Ptr, InitValue, Size);
}

/************************************************************************
 ************************************************************************
 *** Set debugger break point here to get control at memory
 *** allocation/free sequence number specified in KpSeqNumStopAt.
 ************************************************************************
 ************************************************************************/
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

static void MemLogFunc (void FAR *Ptr, long Size, MemLogType_t Type)
{
	char		MsgText [82];
	static KpFileId	fid;
	int32		MsgSize, i;
	char		*sLogName;
	
	if (NULL == Ptr)
		return;

/* check for first time */
	if (lLogSeqNum == 0) {
		fLogFileError = KPFALSE;
		lLogUseCount = 0;

		sLogName = getenv("KPMEMTRACEFILE");
		if (sLogName == NULL){
			fLogFileError = KPTRUE;
			return;
		}

		/* create the log file */
		if (KpFileOpen(sLogName, "w", &KCPDataDirProps, &fid) == 0) {
			fLogFileError = KPTRUE;
			return;
		}
	}
	if (fLogFileError == KPTRUE)
		return;

	++lLogSeqNum;

	for (i = 0; i < 80; i++) {
		MsgText[i] = 32;	/* SP */
	}
	MsgText[i++] = '\n';
	MsgText[i++] = 0;
	
/* create proper log message */
	switch (Type) {
	case MemLogAlloc:
		lLogUseCount++;
		KpLtos((int32)Ptr, MsgText);
		strcpy (&MsgText[10], "Allocate");
		KpItoa(lLogSeqNum, &MsgText[20]);
		KpItoa(lLogUseCount, &MsgText[30]);
		KpItoa(Size, &MsgText[40]);
		break;

	case MemLogFree:
		lLogUseCount--;
		KpLtos((int32)Ptr, MsgText);
		strcpy (&MsgText[10], "Free");
		KpItoa(lLogSeqNum, &MsgText[20]);
		KpItoa(lLogUseCount, &MsgText[30]);
		break;

	default:
		strcpy (MsgText, "Unknown memory log type\n");
		break;
	}

	for (i = 0; i < 80; i++) {
		if (MsgText[i] == 0) {
			MsgText[i] = 32;	/* SP */
		}
	}
	
/* write the message */
	MsgSize = strlen (MsgText);
	if  (KpFileWrite(fid, &MsgText, MsgSize) == 0){
		fLogFileError = KPTRUE;
		return;
	}

/* check for user requested break */
	if (lLogSeqNum == KpSeqNumStopAt) {
		KpMemLogBreak ();
	}
}

#define MemLogInitBlk(Ptr, Size) initMemBlk (Ptr, Size)
#define MemLogAlloc(Ptr, Size)	MemLogFunc (Ptr, Size, MemLogAlloc)
#define MemLogFree(Ptr)	MemLogFunc (Ptr, 0, MemLogFree)

#else

#define KpMemFudge(size) (Size)
#define MemLogInitBlk(Ptr, Size)
#define MemInitBlk(Ptr, Size)
#define MemLogAlloc(Ptr, Size)
#define MemLogFree(Ptr)

#endif


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getBufferSize   (Sun Version)
 *
 * DESCRIPTION
 * This function returns the size of a buffer given its handle.
 *
 * AUTHOR
 * Scott Kohnle
 *
 * DATE CREATED
 * March 21, 1994
 *
 * Modified
 * 
 *
--------------------------------------------------------------------*/

KpInt32_t getBufferSizePrv (KpHandle_t h)
{
	KpInt32_t *Ptr;

	Ptr = (KpInt32_t *)h;
	return ((KpInt32_t) *(Ptr-DLWORD));
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getSysBufferSize   (Sun Version)
 *
 * DESCRIPTION
 * This function returns the size of a buffer given its handle.
 *
 * AUTHOR
 * Scott Kohnle
 *
 * DATE CREATED
 * March 21, 1994
 *
--------------------------------------------------------------------*/

KpInt32_t getSysBufferSize (KpHandle_t h)
{
	return (getBufferSize(h));
}

KpGenericPtr_t
allocBufferPtr (KpInt32_t numBytes)
{

        return ((KpGenericPtr_t) allocBufferHandle (numBytes));
}

KpHandle_t
allocLargeBufferHandle (KpInt32_t numBytes)
{
        return (allocBufferHandle (numBytes));
}
 
KpHandle_t
allocSysBufferHandle (KpInt32_t numBytes)
{
        return (allocBufferHandle (numBytes));
}
 
KpHandle_t
allocSysLargeBufferHandle (KpInt32_t numBytes)
{
        return (allocBufferHandle (numBytes));
}
 

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * reallocBufferPtr  (Sun Version)
 *
 * DESCRIPTION
 * This function allocates a memory buffer, locks it, copies data from
 * the original buffer, frees the original buffer, and returns a handle
 * to the newly locked block.
 *
 * AUTHOR
 * Stan Pulchtopek (Mac and Windows)
 *
 * DATE CREATED
 * March 25, 1997
 *
--------------------------------------------------------------------*/

KpGenericPtr_t FAR reallocBufferPtrPrv(KpGenericPtr_t fp,
				KpInt32_t numBytes)
{
	KpHandle_t	h;
	/* initialize it in case of error */
	KpGenericPtr_t	p = (KpGenericPtr_t) NULL;

	KpInt32_t	oldNumBytes;
 
	if (fp != NULL)
	{
		/* get size of old buffer */
		oldNumBytes = getPtrSize(fp);
 
		if (numBytes >= 0)
		{
			/* get memory */
			h = allocBufferHandle (numBytes);
 
			if (h != NULL)
			{
				/* lock it down */
				p = lockBuffer (h);
	 
				if (p == NULL)
				{
					freeBuffer (h);
				} else
				{
					/* make sure sizes make sense */
					if (oldNumBytes <= numBytes)
					{
						/* copy old contents */
						memmove(p,
						       fp, 
						       oldNumBytes);
					}
 
					/* free old buffer */
					freeBufferPtr(fp);
				}
			}
		}
	}
 
	return p;
}
 

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * allocBufferHandle  (Sun Version)
 *
 * DESCRIPTION
 * This function allocates a memory buffer, locks it, and returns a handle
 * to the newly locked block.  In the current implementations, nothing really
 * happens because handles and pointers are the same thing.
 *
 * AUTHOR
 * Scott Kohnle
 *
 * DATE CREATED
 * March 21, 1994
 *
--------------------------------------------------------------------*/

KpHandle_t allocBufferHandlePrv (KpInt32_t numBytes)
{
	KpInt32_t *Ptr;

	Ptr = (KpInt32_t *) malloc ((unsigned int)numBytes+HEADER);
	if (Ptr != NULL) {
	    *Ptr = numBytes;	/* store the number of bytes allocated */
	    Ptr += DLWORD;

	    MemLogAlloc (Ptr, numBytes);
	}

	return (Ptr);
}
 
KpGenericPtr_t allocSysBufferPtr (KpInt32_t numBytes)
{
        return (allocBufferHandle (numBytes));
}
 
void freeBufferPrv (KpHandle_t handle)
{
	if (NULL != handle) {
	   MemLogFree (handle);
	   (void) free ((char *) handle-HEADER);
	}
}
 
void freeBufferPtr (KpGenericPtr_t p)
{
       freeBuffer ((KpGenericPtr_t) p);
}
 
void freeSysBuffer (KpHandle_t handle)
{
       freeBuffer (handle);
}
 
void freeSysBufferPtr (KpGenericPtr_t p)
{
       freeBuffer ((KpHandle_t) p);
}

KpHandle_t getHandleFromPtrPrv (KpGenericPtr_t p)
{
        return ((KpHandle_t) p);
}
 
KpHandle_t getSysHandleFromPtr (KpGenericPtr_t p)
{
        return (getHandleFromPtr (p));
}
 
KpGenericPtr_t lockBufferPrv (KpHandle_t handle)
{
        return ((KpGenericPtr_t)handle);

}
 
KpGenericPtr_t lockSysBuffer (KpHandle_t handle)
{
        return (lockBuffer (handle));
}
 
KpInt32_t unlockBufferPrv (KpHandle_t handle)
{
        return ( (handle != 0));
}
 
KpInt32_t unlockSysBuffer (KpHandle_t handle)
{
        return (unlockBuffer (handle));
}
 
KpHandle_t unlockBufferPtr (KpGenericPtr_t p)
{
        return ((KpHandle_t)p);
}
 
KpHandle_t unlockSysBufferPtr (KpGenericPtr_t p)
{
        return (unlockBufferPtr (p));
}
