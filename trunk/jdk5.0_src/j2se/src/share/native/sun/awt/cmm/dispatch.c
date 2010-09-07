/*
 * @(#)dispatch.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)dispatch.c	1.00 00/03/30

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/


#include "kcms_sys.h"

#if !defined (KPWIN)
static KpMemoryData_t KpMemoryData = {sizeof (KpMemoryData_t),
				allocBufferHandlePrv,
				lockBufferPrv,
				unlockBufferPrv,
				getHandleFromPtrPrv,
				getBufferSizePrv,
				reallocBufferPtrPrv,
				freeBufferPrv};
#else
static KpMemoryData_t KpMemoryData;
#endif

/* Use default memory routines */
void
KpSysInit ()
{
	KpMemoryData.KpAllocBufferHandle = allocBufferHandlePrv;
	KpMemoryData.KpLockBuffer = lockBufferPrv;
	KpMemoryData.KpUnlockBuffer = unlockBufferPrv;
	KpMemoryData.KpGetHandleFromPtr = getHandleFromPtrPrv;
	KpMemoryData.KpGetBufferSize = getBufferSizePrv;
	KpMemoryData.KpReAllocBufferPtr = reallocBufferPtrPrv;
	KpMemoryData.KpFreeBuffer = freeBufferPrv;
}

/* Use memory routines passed in from the Application */
void
KpUseAppMem (KpMemoryData_t AppMemoryData)
{
	KpMemoryData = AppMemoryData;
}
/*---------------------------------------------------------------------
 * FUNCTION NAME
 * allocBufferHandle
 *
 * DESCRIPTION
 * This function allocates a buffer and returns a handle to it.  The
 * DOS implementation fakes all this out, of course.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
---------------------------------------------------------------------*/
KpHandle_t allocBufferHandle (KpInt32_t numBytes)
{
KpHandle_t	hBuffer;               /* handle to allocated buffer */
	
	hBuffer = (*KpMemoryData.KpAllocBufferHandle) (numBytes);

	return (hBuffer);
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * reallocBufferPtr
 *
 * DESCRIPTION
 * This function reallocates a memory buffer.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/

KpGenericPtr_t FAR reallocBufferPtr (KpGenericPtr_t fp, KpInt32_t numBytes)
{
KpGenericPtr_t	p = (KpGenericPtr_t) NULL;	/* initialize it in case of error */

	p = (*KpMemoryData.KpReAllocBufferPtr) (fp, numBytes);

	return p;
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * freeBuffer
 *
 * DESCRIPTION
 * This function frees a memory buffer given its handle and
 * returns nothing.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/
void freeBuffer (KpHandle_t handle)
{
	(*KpMemoryData.KpFreeBuffer) (handle);
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getHandlefromPtr
 *
 * DESCRIPTION
 * This function returns the handle corresponding to an input
 * pointer.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/
KpHandle_t getHandleFromPtr (KpGenericPtr_t p)
{
KpHandle_t	h = NULL;

	h = (*KpMemoryData.KpGetHandleFromPtr) (p);

	return (h);
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * getBufferSize
 *
 * DESCRIPTION
 * This function returns the size of a buffer given its handle.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/
KpInt32_t getBufferSize (KpHandle_t h)
{
KpInt32_t	size;
	
	size = (*KpMemoryData.KpGetBufferSize) (h);

	return size;
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * lockBuffer
 *
 * DESCRIPTION
 * This function locks a memory buffer in place given its handle and
 * returns a pointer to the newly locked block.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/
KpGenericPtr_t lockBuffer( KpHandle_t h )
{
KpGenericPtr_t	p = NULL;

	p = (*KpMemoryData.KpLockBuffer) (h);

	return   p;
}

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * unlockBuffer
 *
 * DESCRIPTION
 * This function unlocks a memory buffer given its handle and
 * returns a boolean to indicate whether if worked.
 *
 * AUTHOR
 * Mark Micalizzi
 *
 * DATE CREATED
 * March 30, 2000
 *
--------------------------------------------------------------------*/
KpInt32_t unlockBuffer( KpHandle_t h )
{
KpInt32_t	result;

	result = (*KpMemoryData.KpUnlockBuffer) (h);
	
	return result;
}



