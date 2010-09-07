/*
 * @(#)spcvrt.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the low level functions
				for converting between internal and external
				byte order.

				Created by lsh, November 6, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1997 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spcvrt.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spcvrt.c $
		$Revision: 2 $
		$Date: 7/02/99 3:21p $
		$Author: Doro $

	SCCS Revision:
		@(#)spcvrt.c	1.21	05/27/97

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-1997                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include <stdio.h>

#if defined (KPMAC)
#include <string.h>
#endif


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put N bytes to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 4, 1993
 *------------------------------------------------------------------*/
void SpPutBytes (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	Size,
				void		KPHUGE *Data)
{
	char	KPHUGE *BytePtr;

	if (0 == Size)
		return;

	BytePtr = *Ptr;

	KpMemCpy (BytePtr, Data, Size);

	*Ptr = BytePtr + Size;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put a 2 byte number to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 2, 1993
 *------------------------------------------------------------------*/
void SpPutUInt16 (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	Value)
{
	char	KPHUGE *BytePtr;

	BytePtr = *Ptr;
	*BytePtr++ = (char) (Value >> 8);
	*BytePtr++ = (char) (Value & 0xFF);
	*Ptr = BytePtr;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put N 2 byte numbers to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 6, 1993
 *------------------------------------------------------------------*/
void SpPutUInt16s (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	KPHUGE *Values,
				KpUInt32_t	Count)
{
	while (Count--)
		SpPutUInt16 (Ptr, *Values++);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put a 4 byte number to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 2, 1993
 *------------------------------------------------------------------*/
void SpPutUInt32 (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	Value)
{
	char	KPHUGE *BytePtr;

	BytePtr = *Ptr;
	*BytePtr++ = (char) (Value >> 24);
	*BytePtr++ = (char) (Value >> 16);
	*BytePtr++ = (char) (Value >> 8);
	*BytePtr++ = (char) (Value & 0xFF);
	*Ptr = BytePtr;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put a fixed point number to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 3, 1993
 *------------------------------------------------------------------*/
void SpPutF15d16 (
				char		KPHUGE * FAR *Ptr,
				KpF15d16_t	KPHUGE *Values,
				KpUInt32_t	Count)
{
	while (Count--)
		SpPutUInt32 (Ptr, (KpUInt32_t) *Values++);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put a fixed XYZ to a buffer in external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 3, 1993
 *------------------------------------------------------------------*/
void SpPutF15d16XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF15d16XYZ_t	FAR *Value)
{
	SpPutF15d16 (Ptr, &Value->X, 1);
	SpPutF15d16 (Ptr, &Value->Y, 1);
	SpPutF15d16 (Ptr, &Value->Z, 1);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get N bytes from a buffer to external format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 4, 1993
 *------------------------------------------------------------------*/
void SpGetBytes (
				char		KPHUGE * FAR *Ptr,
				void		KPHUGE *Data,
				KpUInt32_t	Size)
{
	char	KPHUGE *BytePtr;

	if (0 == Size)
		return;

	BytePtr = *Ptr;
	KpMemCpy (Data, BytePtr, Size);
	*Ptr = BytePtr + Size;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert a 2 byte number to internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 18, 1993
 *------------------------------------------------------------------*/
KpUInt16_t SpGetUInt16 (
				char	KPHUGE * FAR *Ptr)
{
	unsigned char	KPHUGE *BytePtr;
	KpUInt16_t		Value;

	BytePtr = (unsigned char KPHUGE *) *Ptr;
	Value = *BytePtr++;
	Value <<= 8;
	Value |= *BytePtr++;
	*Ptr = (char KPHUGE *) BytePtr;

	return Value;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get N 2 byte numbers in internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 6, 1993
 *------------------------------------------------------------------*/
void SpGetUInt16s (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	KPHUGE *Value,
				KpUInt32_t	Count)
{
	while (Count--)
		*Value++ = SpGetUInt16 (Ptr);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert a 4 byte number to internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 18, 1993
 *------------------------------------------------------------------*/
KpUInt32_t SpGetUInt32 (
				char	KPHUGE * FAR *Ptr)
{
	KpUInt32_t		Value;
	unsigned char	KPHUGE *BytePtr;

	BytePtr = (unsigned char KPHUGE *) *Ptr;
	Value = *BytePtr++;
	Value <<= 8;
	Value |= *BytePtr++;
	Value <<= 8;
	Value |= *BytePtr++;
	Value <<= 8;
	Value |= *BytePtr++;
	*Ptr = (char KPHUGE *) BytePtr;

	return Value;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert a fixed long to internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 18, 1993
 *------------------------------------------------------------------*/
void SpGetF15d16 (
				char		KPHUGE * FAR *Ptr,
				KpF15d16_t	KPHUGE *Values,
				KpUInt32_t	Count)
{
	while (Count--)
		*Values++ = SpGetUInt32 (Ptr);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert a fixed long XYZ to internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 18, 1993
 *------------------------------------------------------------------*/
void SpGetF15d16XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF15d16XYZ_t	FAR *XYZ)
{
	SpGetF15d16 (Ptr, &XYZ->X, 1);
	SpGetF15d16 (Ptr, &XYZ->Y, 1);
	SpGetF15d16 (Ptr, &XYZ->Z, 1);
}



