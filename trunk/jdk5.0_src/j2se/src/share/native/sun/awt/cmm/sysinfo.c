/*
 * @(#)sysinfo.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the system info functions.

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1995 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile$
		$Logfile$
		$Revision$
		$Date$
		$Author$

	SCCS Revision:
		@(#)sysinfo.c	1.5 04/08/96
	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1995                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "kcms_sys.h"

#if defined(KPMAC)
#include <GestaltEqu.h>
#endif
#if defined (KPMSMAC)
#include <gestalte.h>
#endif
#if defined (KPMAC) || defined (KPMSMAC)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the type and version number of the OS.
 *
 * AUTHOR
 * 	sek
 *
 * DATE CREATED
 *	February 15, 1995
 *------------------------------------------------------------------*/

KpInt32_t KpGetSystemInfo (KpOsType_p KpOsType, KpInt32_p version)
{
	OSErr   myErr;

	/* get the version information */
	myErr = Gestalt ('sysv', version);
	if (myErr != noErr) {
        return (KCMS_FAIL);
    }

	switch ((*version & 0xff00) >> 8)
	{

		case 6:
				*KpOsType = KPOSTYPE_SYSTEM6;
				break;

		case 7:
				*KpOsType = KPOSTYPE_SYSTEM7;
				break;

		case 8:
				*KpOsType = KPOSTYPE_SYSTEM8;
				break;

		default:
				*KpOsType = KPOSTYPE_SYSUNKNOWN;
	}

	return (KCMS_SUCCESS);
}

#elif defined(KPWIN)

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the type and version number of the OS.
 *
 * AUTHOR
 * 	sek
 *
 * DATE CREATED
 *	February 15, 1995
 *------------------------------------------------------------------*/

KpInt32_t KpGetSystemInfo (KpOsType_p KpOsType, KpInt32_p version)
{
	DWORD versionInfo;
	OSVERSIONINFO OSInfo = {0};

	/* get the version information about windows */
	versionInfo = GetVersion ();

	/* get the windows version numbers */
	*version = (KpInt32_t)LOWORD (versionInfo);


	/* Figure out the OS */
	OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSInfo);

	if (OSInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	  {
		/* windows 95 */
		*KpOsType = KPOSTYPE_WIN95;
	  }
	else
	  {
	   if (OSInfo.dwPlatformId == VER_PLATFORM_WIN32s)
	     {
		  /* windows 32s */
		  *KpOsType = KPOSTYPE_WIN32S;
	     }
	   else
	     {
	      if (OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	        {
		     /* windows NT */
		     *KpOsType = KPOSTYPE_WINNT;
	        }
	      else
	        {
		     /* beat's me */
		     *KpOsType = KPOSTYPE_SYSUNKNOWN;
	        }
	     }
	  }


	return (KCMS_SUCCESS);
}

#else
#error	#### Unsupported OS ####
#endif
