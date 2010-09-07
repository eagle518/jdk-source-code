/*
 * @(#)spsys.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains system specific functions.

				Created by lsh, September 14, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993 - 1999 by Eastman Kodak Company, 
			all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spsys.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spsys.c $
		$Revision: 2 $
		$Date: 7/02/99 3:21p $
		$Author: Doro $

	SCCS Revision:
		@(#)spsys.c	1.11 11/24/98

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993 - 1999               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate a block of memory.  The allocated block can be bigger
 *	than 64K.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 14, 1993
 *------------------------------------------------------------------*/
void FAR *SpMalloc (KpInt32_t Size)
{
	return allocBufferPtr (Size);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free a block of memory allocated with SpMalloc.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 14, 1993
 *------------------------------------------------------------------*/
void SpFree (void FAR *Ptr)
{
	if (NULL != Ptr)
		freeBufferPtr (Ptr);
}



