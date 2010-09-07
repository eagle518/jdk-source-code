/*
 * @(#)memory.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)memory.c	1.45 98/12/01

	Contains:
		This module contains routines to handle memory allocation for
		the KCMS API library.  It is based on the memory handle allocation
		mechanism used by systems such as Macintosh and Microsoft Windows
		3.0.  This file will contain a section for each operating system
		supported with each section separated by #if.  The routines which
		must be supported are:
			KpHandle_t      allocBufferHandle (KpInt32_t);
			KpHandle_t      allocLargeBufferHandle (KpInt32_t);
			KpGenericPtr_t 	allocBufferPtr (KpInt32_t);
			KpGenericPtr_t 	reallocBufferPtr (KpGenericPtr_t, KpInt32_t);
			void            freeBuffer (KpHandle_t);
			void            freeBufferPtr (KpGenericPtr_t);
			KpInt32_t       getBufferSize (KpHandle_t);
			KpHandle_t      getHandleFromPtr (KpGenericPtr_t);
			KpGenericPtr_t	lockBuffer (KpHandle_t);
			bool            unlockBuffer (KpHandle_t);
			KpHandle_t      unlockBufferPtr (KpGenericPtr_t);

			KpHandle_t      allocSysBufferHandle (KpInt32_t);
			KpHandle_t      allocSysLargeBufferHandle (KpInt32_t);
			KpGenericPtr_t  allocSysBufferPtr (KpInt32_t);
			void            freeSysBuffer (KpHandle_t);
			void            freeSysBufferPtr (KpGenericPtr_t);
			KpInt32_t       getSysBufferSize (KpHandle_t);
			KpHandle_t      getSysHandle (KpGenericPtr_t);
			KpHandle_t      getSysHandleFromPtr (KpGenericPtr_t);
			KpGenericPtr_t  lockSysBuffer (KpHandle_t);
			bool            unlockSysBuffer (KpHandle_t);
			KpHandle_t      unlockSysBufferPtr (KpGenericPtr_t);
			Created by Peter Tracy, May 1, 1991


	Windows Revision Level:
		$Workfile: memory.c $
		$Logfile: /DLL/KodakCMS/kpsys_lib/memory.c $
		$Revision: 7 $
		$Date: 5/30/03 5:01p $
		$Author: Arourke $

	COPYRIGHT (c) 1991-2003 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/


#include "kcms_sys.h"

/* #define KP_DEBUG	 defined includes debug stuff */

#if defined(KP_DEBUG)
#define DEBUGTRAP(x) DebugStr(x);
#else
#define DEBUGTRAP(x)
#endif



		/*   STUFF WHICH IS NOT OS DEPENDENT   */

#if !defined(KPWIN)

/*---------------------------------------------------------------------
 * FUNCTION NAME
 * getPtrSize (OS Independent Version)
 *
 * DESCRIPTION
 * This function gets the size of a buffer given a pointer to it.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * November 21, 1991
 *
---------------------------------------------------------------------*/
KpInt32_t FAR getPtrSize (KpGenericPtr_t p)
{
KpHandle_t     h;

	h = getHandleFromPtr (p);
	return (h == (KpHandle_t) NULL) ? (KpInt32_t) 0 : getBufferSize (h);
}


/*---------------------------------------------------------------------
 * FUNCTION NAME
 * getSysPtrSize (OS Independent Version)
 *
 * DESCRIPTION
 * This function gets the size of a buffer given a pointer to it.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * November 21, 1991
 *
---------------------------------------------------------------------*/
KpInt32_t FAR getSysPtrSize (KpGenericPtr_t p)
{
KpHandle_t       h;

	h = getSysHandleFromPtr (p);
	return (h == (KpHandle_t) NULL) ? (KpInt32_t) 0 : getSysBufferSize (h);
}
#endif


/* Versions for the Macintosh */
#if defined(KPDOS) || defined(KPWIN)

/* this section now in win_mem.c */


/* Versions for the Macintosh */
#elif defined(KPMAC)
#include <Memory.h>

#include "memloger.h"

/*---------------------------------------------------------------------
 * FUNCTION NAME
 * allocBufferHandle (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a buffer and returns a handle to it.  The
 * DOS implementation fakes all this out, of course.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
---------------------------------------------------------------------*/
KpHandle_t allocBufferHandlePrv (KpInt32_t numBytes)
{
Size	blockSize;             /* size of memory to allocate */
Handle	hBuffer;               /* handle to allocated buffer */
OSErr	err;

	blockSize = (Size) numBytes;
	hBuffer = NewHandle (blockSize);

	err = MemError();
	if ((err != noErr) && (err != memFullErr)) {
		DEBUGTRAP("\p allocBufferHandle NewHandle")
		hBuffer = NULL;
	}

	if (hBuffer == (Handle) nil) {
		/* if there is no memory in the apps heap try the system heap */
		hBuffer = TempNewHandle (blockSize, &err);
	
		if ((err != noErr) && (err != memFullErr)) {
			DEBUGTRAP("\p allocBufferHandle TempNewHandle")
			hBuffer = NULL;
		}
	}

	if (hBuffer != (Handle) nil) {
		MemLogAlloc (hBuffer, numBytes);
	}
	
	return (KpHandle_t) hBuffer;
}
/*--------------------------------------------------------------------
 * FUNCTION NAME
 * reallocBufferPtr  (Macintosh Version)
 *
 * DESCRIPTION
 * This function reallocates a memory buffer.
 *
 * AUTHOR
 * Stan Pulchtopek
 *
 * DATE CREATED
 * May 27, 1994
 *
--------------------------------------------------------------------*/

KpGenericPtr_t FAR reallocBufferPtrPrv (KpGenericPtr_t fp, KpInt32_t numBytes)
{
KpHandle_t		h;
KpGenericPtr_t	p = (KpGenericPtr_t) NULL;	/* initialize it in case of error */
KpInt32_t		oldNumBytes;

	if (fp != NULL) {
		oldNumBytes = getPtrSize(fp);			/* get size of old buffer */

		if (oldNumBytes != 0) {	
			h = allocBufferHandle (numBytes);	/* get memory */

			if (h != NULL) {
				p = lockBuffer (h);				/* lock it down */

				if (p == NULL) {
					freeBuffer (h);
				}
				else {
					if (oldNumBytes <= numBytes) {	/* make sure sizes make sense */
						BlockMove((Ptr) fp, (Ptr) p, (Size) oldNumBytes);	/* copy old contents */
					}
					
					freeBufferPtr(fp);	/* free old buffer */
				}
			}
		}
	}

	return p;
}


/*---------------------------------------------------------------------
 * FUNCTION NAME
 * allocSysBufferHandle (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a buffer in system memory and
 * returns a handle to it.
 *
 * AUTHOR
 * Bonnie Hill
 *
 * DATE CREATED
 * March 4, 1992
 *
---------------------------------------------------------------------*/
KpHandle_t allocSysBufferHandle( KpInt32_t numBytes )
{
Handle	hBuffer;         /* handle to allocated buffer */
OSErr	err;

#if (TARGET_API_MAC_CARBON)
		/* Targeting Carbon - Don't know if this is correct but 
		    we'll try for now. */
	hBuffer = NewHandle ((Size) numBytes);
#else 	
	hBuffer = NewHandleSys ((Size) numBytes);
#endif

	err = MemError();
	if ((err != noErr) && (err != memFullErr)) {
		DEBUGTRAP("\p allocSysBufferHandle NewHandleSys")
		hBuffer = NULL;
	}

	if (hBuffer != (Handle) nil) {
		MemLogAlloc (hBuffer, numBytes);
	}

	return (KpHandle_t) hBuffer;
}


/*---------------------------------------------------------------------
 * FUNCTION NAME
 * allocSysLargeBufferHandle (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a buffer in system memory and
 * returns a handle to it.
 * This routine is used when the buffer may be larger than 64K bytes
 * because the foibles of DOS.  Of course, the Macintosh version
 * doesn't need to do anything special
 *
 * AUTHOR
 * Anne Rourke
 *
 * DATE CREATED
 * January 24, 1996
 *
---------------------------------------------------------------------*/
KpHandle_t allocSysLargeBufferHandle (KpInt32_t numBytes)
{
	return allocSysBufferHandle (numBytes);
}
	

/*---------------------------------------------------------------------
 * FUNCTION NAME
 * allocLargeBufferHandle (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a buffer and returns a handle to it.
 * This routine is used when the buffer may be larger than 64K bytes
 * because the foibles of DOS.  Of course, the Macintosh version
 * doesn't need to do anything special
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
---------------------------------------------------------------------*/
KpHandle_t allocLargeBufferHandle (KpInt32_t numBytes)
{
	return allocBufferHandle (numBytes);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * allocBufferPtr  (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a memory buffer, locks it, and
 * returns a handle to the newly locked block.
 *
 * AUTHOR
 * George Pawle
 *
 * DATE CREATED
 * 26 June 1991
 *
--------------------------------------------------------------------*/
KpGenericPtr_t allocBufferPtr (KpInt32_t numBytes)
{
KpHandle_t		h;
KpGenericPtr_t	p = NULL;          /* initialize it in case of error */

	h = allocBufferHandle (numBytes);        /* get memory */
	if (h != NULL) {
		p = lockBuffer (h);            /* lock it down */
		if (p == NULL) {
			freeBuffer (h);
		}
	}

	return p;
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * allocSysBufferPtr  (Macintosh Version)
 *
 * DESCRIPTION
 * This function allocates a buffer in system memory, locks it, and
 * returns a handle to the newly locked block.
 *
 * AUTHOR
 * Bonnie Hill
 *
 * DATE CREATED
 * 4 March 1992
 *
--------------------------------------------------------------------*/
KpGenericPtr_t allocSysBufferPtr( KpInt32_t numBytes )
{
KpHandle_t		h;
KpGenericPtr_t	p = NULL;          /* initialize it in case of error */

	h = allocSysBufferHandle (numBytes);     /* get memory */
	if (h != NULL) {
		p = lockBuffer (h);            /* lock it down */
		if (p == NULL) {
			freeBuffer (h);
		}
	}

	return p;
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * freeBuffer   (Macintosh Version)
 *
 * DESCRIPTION
 * This function frees a memory buffer given its handle and
 * returns nothing.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
--------------------------------------------------------------------*/
void freeBufferPrv (KpHandle_t handle)
{
OSErr	err;

	if (handle != NULL) {
		MemLogFree (handle);
		DisposeHandle(handle);

		err = MemError();
		if (err != noErr) {
			DEBUGTRAP("\p freeBuffer DisposeHandle")
		}
	}
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * freeSysBuffer   (Macintosh Version)
 *
 * DESCRIPTION
 * This function frees a memory buffer given its handle and returns nothing.
 *
 * AUTHOR
 * Stan Pulchtopek
 *
 * DATE CREATED
 * May 23, 1994
 *
--------------------------------------------------------------------*/
void freeSysBuffer( KpHandle_t handle )
{
	freeBuffer (handle);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getHandlefromPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function returns the handle corresponding to an input
 * pointer.
 *
 * AUTHOR
 * Bonnie Hill
 *
 * DATE CREATED
 * November 27, 1991
 *
--------------------------------------------------------------------*/
KpHandle_t getHandleFromPtrPrv (KpGenericPtr_t p)
{
KpHandle_t	h = NULL;
OSErr		err;

	if (p != NULL) {
		h = RecoverHandle (p);
		err = MemError();
		if (err != noErr) {
			DEBUGTRAP("\p getHandleFromPtr RecoverHandle")
			h = NULL;
		}
	}

	return (h);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getSysHandleFromPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function returns the handle corresponding to an input
 * pointer.
 *
 * AUTHOR
 * Bonnie Hill
 *
 * DATE CREATED
 * November 27, 1991
 *
--------------------------------------------------------------------*/
KpHandle_t getSysHandleFromPtr (KpGenericPtr_t p)
{
	return getHandleFromPtr (p);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * freeBufferPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function frees a memory buffer given its handle and
 * returns nothing.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
--------------------------------------------------------------------*/
void freeBufferPtr (KpGenericPtr_t p)
{
Handle  handle;
	
	if (NULL != p) {
		handle = getHandleFromPtr (p);
		if (handle != NULL) {
			freeBuffer (handle);
		}
	}
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * freeSysBufferPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function frees a memory buffer given its handle and
 * returns nothing.
 *
 * AUTHOR
 * Bonnie Hill
 *
 * DATE CREATED
 * March 4, 1992
 *
--------------------------------------------------------------------*/
void freeSysBufferPtr (KpGenericPtr_t p)
{
	freeBufferPtr (p);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getBufferSize   (Macintosh Version)
 *
 * DESCRIPTION
 * This function returns the size of a buffer given its handle.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * July 16, 1991
 *
--------------------------------------------------------------------*/
KpInt32_t getBufferSizePrv (KpHandle_t h)
{
OSErr		err;
KpInt32_t	size;

	if (h == NULL) {
		return 0;
	}
	else {
		size = GetHandleSize (h);

		err = MemError();
		if (err != noErr) {
			DEBUGTRAP("\p getBufferSize GetHandleSize")
			size = 0;
		}
	}
	
	return size;
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getSysBufferSize   (Macintosh Version)
 *
 * DESCRIPTION
 * This function returns the size of a buffer given its handle.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * July 16, 1991
 *
--------------------------------------------------------------------*/
KpInt32_t getSysBufferSize (KpHandle_t h)
{
	return getBufferSize(h);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * lockBuffer  (Macintosh Version)
 *
 * DESCRIPTION
 * This function locks a memory buffer in place given its handle and
 * returns a pointer to the newly locked block.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
--------------------------------------------------------------------*/
KpGenericPtr_t lockBufferPrv( KpHandle_t h )
{
KpGenericPtr_t	p = NULL;
OSErr			err;
#if defined(KP_DEBUG)
Size			grow, maxBytes;

	maxBytes = MaxMem(&grow);	/* push memory around */
#endif
	
	if (h != NULL) {
#if defined(KP_DEBUG)
		MoveHHi(h);			/* move handle to flush out dangling pointers */
		err = MemError();
		if ((err != noErr) && (err != memLockedErr)) {
			DEBUGTRAP("\p lockBuffer MoveHHi")
		}
		else {		/* Now actually do the locking */
#endif
			HLock(h);

			err = MemError();
			if (err == noErr) {
#if !(TARGET_API_MAC_CARBON)
				p = (KpGenericPtr_t)StripAddress(*h);
#else
				p = *h;
#endif
			}
			else {
				DEBUGTRAP("\p lockBuffer HLock")
			}
#if defined(KP_DEBUG)
		}
#endif
	}

	return   p;
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * lockSysBuffer   (Macintosh Version)
 *
 * DESCRIPTION
 * This function locks a memory buffer in place given its handle and
 * returns a pointer to the newly locked block.  This version
 * merely calls lockBuffer.
 *
 * AUTHOR
 * Stan Pulchtopek
 *
 * DATE CREATED
 * May 23, 1994
 *
--------------------------------------------------------------------*/
KpGenericPtr_t lockSysBuffer (KpHandle_t h)
{
	return ( lockBuffer(h) );
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * unlockBuffer   (Macintosh Version)
 *
 * DESCRIPTION
 * This function unlocks a memory buffer given its handle and
 * returns a boolean to indicate whether if worked.
 *
 * AUTHOR
 * Peter Tracy
 *
 * DATE CREATED
 * May 24, 1991
 *
--------------------------------------------------------------------*/
KpInt32_t unlockBufferPrv( KpHandle_t h )
{
OSErr	err;
#if defined(KP_DEBUG)
Size	grow, maxBytes;

	maxBytes = MaxMem(&grow);	/* push memory around */
#endif

	if (h != NULL) {

		HUnlock(h);

		err = MemError();
		if (err != noErr) {
			DEBUGTRAP("\p unlockBuffer HUnlock")
			return KPFALSE;
		}
	}
	
	return KPTRUE;
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * unlockSysBuffer   (Macintosh Version)
 *
 * DESCRIPTION
 * This function unlocks a memory buffer given its handle and
 * returns a boolean to indicate whether if worked.  Merely calls
 * unlockBuffer.
 *
 * AUTHOR
 * Stan Pulchtopek
 *
 * DATE CREATED
 * May 23, 1994
 *
--------------------------------------------------------------------*/
KpInt32_t unlockSysBuffer (KpHandle_t h)
{
	return ( unlockBuffer(h) );
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * unlockBufferPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function unlocks a memory buffer given its pointer and
 * returns a handle to the memory block.
 *
 * AUTHOR
 * George Pawle
 *
 * DATE CREATED
 * 26 June 1991
 *
--------------------------------------------------------------------*/
KpHandle_t unlockBufferPtr( KpGenericPtr_t p )
{
Handle  h = NULL;

	if (p != NULL) {
	
		h = getHandleFromPtr((Ptr)p);
		if (h != NULL) {
			if (!unlockBuffer (h)) {
				DEBUGTRAP("\p unlockBufferPtr unlockBuffer")
				h = NULL;
			}
		}
	}

	return (h);
}
/*--------------------------------------------------------------------
 * FUNCTION NAME
 * unlockSysBufferPtr   (Macintosh Version)
 *
 * DESCRIPTION
 * This function unlocks a memory buffer given its address and
 * returns its handle.  This version calls unlockBufferPtr.
 *
 * AUTHOR
 * Stan Pulchtopek
 *
 * DATE CREATED
 * May 23, 1994
 *
--------------------------------------------------------------------*/
KpHandle_t  unlockSysBufferPtr (KpGenericPtr_t p)
{
	return ( unlockBufferPtr(p) );
}

#elif defined(KPUNIX)

/* this section now in unixmem.c */

#else

	*** OS NOT SUPPORTED ***

#endif
